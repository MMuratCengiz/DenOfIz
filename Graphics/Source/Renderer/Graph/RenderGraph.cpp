/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <DenOfIzGraphics/Renderer/Graph/RenderGraph.h>

using namespace DenOfIz;
using namespace RenderGraphInternal;

NodeResourceUsageDesc::NodeResourceUsageDesc( IBufferResource *resource ) : BufferResource( resource )
{
}

NodeResourceUsageDesc::NodeResourceUsageDesc( ITextureResource *resource ) : Type( NodeResourceUsageType::Texture ), TextureResource( resource )
{
}

NodeResourceUsageDesc NodeResourceUsageDesc::BufferState( const uint32_t frameIndex, IBufferResource *bufferResource, const ResourceState state )
{
    NodeResourceUsageDesc desc( bufferResource );
    desc.FrameIndex = frameIndex;
    desc.State      = state;
    return desc;
}
NodeResourceUsageDesc NodeResourceUsageDesc::TextureState( const uint32_t frameIndex, ITextureResource *textureResource, const ResourceState state )
{
    NodeResourceUsageDesc desc( textureResource );
    desc.FrameIndex = frameIndex;
    desc.State      = state;
    return desc;
}

RenderGraph::RenderGraph( const RenderGraphDesc &desc ) : m_presentNode( { } ), m_desc( desc )
{
    m_frameFences.resize( m_desc.NumFrames );
    for ( uint8_t i = 0; i < m_desc.NumFrames; ++i )
    {
        PresentContext presentContext{ };
        presentContext.ImageReadySemaphore    = std::unique_ptr<ISemaphore>( m_desc.LogicalDevice->CreateSemaphore( ) );
        presentContext.ImageRenderedSemaphore = std::unique_ptr<ISemaphore>( m_desc.LogicalDevice->CreateSemaphore( ) );
        m_presentContexts.push_back( std::move( presentContext ) );

        m_frameFences[ i ] = std::unique_ptr<IFence>( desc.LogicalDevice->CreateFence( ) );
    }

    m_queueCommandListPools.emplace( QueueType::Graphics, 3 );
    m_queueCommandListPools.emplace( QueueType::RayTracing, 3 );
    m_queueCommandListPools.emplace( QueueType::Compute, 3 );
    m_queueCommandListPools.emplace( QueueType::Copy, 3 );

    CommandListPoolDesc poolDesc{ };
    for ( auto &pool : m_queueCommandListPools )
    {
        switch ( pool.first )
        {
        case QueueType::Graphics:
            poolDesc.QueueType       = QueueType::Graphics;
            poolDesc.NumCommandLists = m_desc.NumGraphicsCommandLists;
            break;
        case QueueType::RayTracing:
            poolDesc.QueueType       = QueueType::RayTracing;
            poolDesc.NumCommandLists = m_desc.NumRayTracingCommandLists;
            break;
        case QueueType::Compute:
            poolDesc.QueueType       = QueueType::Compute;
            poolDesc.NumCommandLists = m_desc.NumComputeCommandLists;
            break;
        case QueueType::Copy:
            poolDesc.QueueType       = QueueType::Copy;
            poolDesc.NumCommandLists = m_desc.NumCopyCommandLists;
            break;
        }
        for ( int i = 0; i < m_desc.NumFrames; ++i )
        {
            pool.second[ i ] = std::unique_ptr<ICommandListPool>( m_desc.LogicalDevice->CreateCommandListPool( poolDesc ) );
            if ( pool.first == QueueType::Graphics )
            {
                m_presentContexts[ i ].PresentCommandList = pool.second[ i ]->GetCommandLists( ).GetElement( 0 );
            }
        }
    }

    Reset( );
}

void RenderGraph::Reset( )
{
    m_resourceLocking.TextureStates.clear( );
    m_resourceLocking.BufferStates.clear( );
    m_nodes.clear( );
    for ( uint8_t i = 0; i < m_desc.NumFrames; i++ )
    {
        m_presentContexts[ i ].PresentDependencySemaphores.clear( );
    }
    m_frameTaskflows.clear( );
    m_nodeDescriptions.clear( );
    m_hasPresentNode = false;

    m_remainingCommandLists[ QueueType::Graphics ]   = m_desc.NumGraphicsCommandLists;
    m_remainingCommandLists[ QueueType::RayTracing ] = m_desc.NumRayTracingCommandLists;
    m_remainingCommandLists[ QueueType::Compute ]    = m_desc.NumComputeCommandLists;
    m_remainingCommandLists[ QueueType::Copy ]       = m_desc.NumCopyCommandLists;

    m_commandListIndexAtQueue[ QueueType::Graphics ]   = 0;
    m_commandListIndexAtQueue[ QueueType::RayTracing ] = 0;
    m_commandListIndexAtQueue[ QueueType::Compute ]    = 0;
    m_commandListIndexAtQueue[ QueueType::Copy ]       = 0;
}

void RenderGraph::AddNode( const NodeDesc &desc )
{
    if ( desc.Name.IsEmpty( ) )
    {
        LOG( FATAL ) << "Node must have a name.";
    }

    m_nodeDescriptions.push_back( desc );

    for ( int stateIndex = 0; stateIndex < desc.RequiredStates.NumElements( ); stateIndex++ )
    {
        auto &resourceState = desc.RequiredStates.GetElement( stateIndex );
        if ( resourceState.Type == NodeResourceUsageType::Texture )
        {
            if ( resourceState.TextureResource == nullptr )
            {
                LOG( FATAL ) << "Texture resource must be valid.";
            }

            m_resourceLocking.TextureStates.emplace( resourceState.TextureResource, resourceState.TextureResource->InitialState( ) );
        }
        else
        {
            if ( resourceState.BufferResource == nullptr )
            {
                LOG( FATAL ) << "Buffer resource must be valid.";
            }
            m_resourceLocking.BufferStates.emplace( resourceState.BufferResource, resourceState.BufferResource->InitialState( ) );
        }
    }
}

void RenderGraph::SetPresentNode( const PresentNodeDesc &desc )
{
    if ( desc.SwapChain == nullptr )
    {
        LOG( FATAL ) << "Present node must have a valid swap chain.";
    }

    m_hasPresentNode = true;
    m_presentNode    = desc;
}

void RenderGraph::BuildGraph( )
{
    InitAllNodes( );
    ValidateNodes( );
    ConfigureGraph( );
    BuildTaskflow( );
}

void RenderGraph::InitAllNodes( )
{
    for ( auto &node : m_nodeDescriptions )
    {
        auto graphNode              = std::make_unique<GraphNode>( );
        graphNode->CommandListIndex = m_nodes.size( );

        uint32_t &remaining        = m_remainingCommandLists[ node.QueueType ];
        uint32_t &commandListIndex = m_commandListIndexAtQueue[ node.QueueType ];

        if ( --remaining < 0 )
        {
            LOG( FATAL ) << "Not enough command lists for the queue type " << (uint32_t)node.QueueType << ", add more via `RenderGraphDesc`.";
        }

        CommandListPoolList &poolList = m_queueCommandListPools[ node.QueueType ];
        // First command list is reserved for present node
        uint32_t presentPadding = node.QueueType == QueueType::Graphics ? 1 : 0;

        for ( uint8_t i = 0; i < m_desc.NumFrames; i++ )
        {
            auto newContext         = std::make_unique<NodeExecutionContext>( );
            newContext->CommandList = poolList[ i ]->GetCommandLists( ).GetElement( commandListIndex + presentPadding );
            newContext->Execute     = node.Execute;
            for ( int stateIndex = 0; stateIndex < node.RequiredStates.NumElements( ); stateIndex++ )
            {
                auto &resourceState = node.RequiredStates.GetElement( stateIndex );
                if ( resourceState.FrameIndex == i )
                {
                    newContext->ResourceUsagesPerFrame.push_back( resourceState );
                }
            }
            graphNode->Contexts.push_back( std::move( newContext ) );
        }
        m_nodes.push_back( std::move( graphNode ) );
        ++commandListIndex;
    }
}

void RenderGraph::ConfigureGraph( )
{
    uint32_t                                  freeSemaphoreIndex = 0;
    std::unordered_map<std::string, uint32_t> processedNodes;
    while ( processedNodes.size( ) < m_nodeDescriptions.size( ) )
    {
        for ( uint32_t nodeIndex = 0; nodeIndex < m_nodeDescriptions.size( ); nodeIndex++ )
        {
            auto &node                 = m_nodeDescriptions[ nodeIndex ];
            bool  isDependencyResolved = true;

            for ( int i = 0; i < node.Dependencies.NumElements( ); ++i )
            {
                auto &dependency = node.Dependencies.GetElement( i );
                if ( !processedNodes.contains( dependency.Get( ) ) )
                {
                    isDependencyResolved = false;
                    break;
                }
            }

            if ( !isDependencyResolved )
            {
                continue;
            }

            for ( int i = 0; i < node.Dependencies.NumElements( ); ++i )
            {
                auto       &dependency = node.Dependencies.GetElement( i );
                ISemaphore *semaphore  = GetOrCreateSemaphore( freeSemaphoreIndex );
                for ( uint8_t frameIndex = 0; frameIndex < m_desc.NumFrames; frameIndex++ )
                {
                    m_nodes[ nodeIndex ]->Contexts[ frameIndex ]->WaitOnSemaphores.SetElement( 0, semaphore );
                    auto &notifySemaphores = m_nodes[ processedNodes[ dependency.Get( ) ] ]->Contexts[ frameIndex ]->NotifySemaphores;
                    notifySemaphores.AddElement( semaphore );
                }
            }
            processedNodes[ node.Name.Get( ) ] = nodeIndex;
        }
    }

    DZ_RETURN_IF( !m_hasPresentNode );
    for ( int i = 0; i < m_presentNode.Dependencies.NumElements( ); ++i )
    {
        auto       &dependency = m_presentNode.Dependencies.GetElement( i );
        ISemaphore *semaphore  = GetOrCreateSemaphore( freeSemaphoreIndex );
        for ( uint8_t frameIndex = 0; frameIndex < m_desc.NumFrames; frameIndex++ )
        {
            m_presentContexts[ frameIndex ].PresentDependencySemaphores.push_back( semaphore );
            auto &semaphores = m_nodes[ processedNodes[ dependency.Get( ) ] ]->Contexts[ frameIndex ]->NotifySemaphores;
            semaphores.AddElement( semaphore );
        }
    }

    for ( uint8_t i = 0; i < m_desc.NumFrames; i++ )
    {
        for ( int stateIndex = 0; stateIndex < m_presentNode.RequiredStates.NumElements( ); stateIndex++ )
        {
            auto &resourceState = m_presentNode.RequiredStates.GetElement( stateIndex );
            if ( resourceState.FrameIndex == i )
            {
                m_presentContexts[ i ].ResourceUsagesPerFrame.push_back( resourceState );
            }
        }
    }
}

void RenderGraph::BuildTaskflow( )
{
    m_frameTaskflows.clear( );
    std::unordered_map<std::string, tf::Task> tasks;

    for ( uint32_t frame = 0; frame < m_desc.NumFrames; frame++ )
    {
        tf::Taskflow &taskflow  = m_frameTaskflows.emplace_back( );
        uint32_t      nodeIndex = 0;

        for ( auto &node : m_nodes )
        {
            NodeDesc   &nodeDesc = m_nodeDescriptions[ nodeIndex ];
            std::string nodeName = nodeDesc.Name.Get( );

            tasks[ nodeName ] = taskflow
                                    .emplace(
                                        [ this, frame, &node, nodeIndex ]
                                        {
                                            const std::unique_ptr<NodeExecutionContext> &context = node->Contexts[ frame ];
                                            std::lock_guard                              selfLock( context->SelfMutex );
                                            ICommandList                               *&commandList = context->CommandList;

                                            commandList->Begin( );
                                            IssueBarriers( commandList, context->ResourceUsagesPerFrame );
                                            context->Execute->Execute( frame, commandList );

                                            ExecuteDesc executeDesc{ };
                                            if ( nodeIndex == m_nodes.size( ) - 1 && !m_hasPresentNode )
                                            {
                                                executeDesc.Notify = m_frameFences[ frame ].get( );
                                            }
                                            executeDesc.WaitOnSemaphores = context->WaitOnSemaphores;
                                            executeDesc.NotifySemaphores = context->NotifySemaphores;
                                            commandList->Execute( executeDesc );
                                        } )
                                    .name( nodeName );

            for ( int i = 0; i < nodeDesc.Dependencies.NumElements( ); ++i )
            {
                auto &dependency = nodeDesc.Dependencies.GetElement( i );
                tasks[ nodeName ].succeed( tasks[ dependency.Get( ) ] );
            }
            nodeIndex++;
        }

        if ( !m_hasPresentNode )
        {
            continue;
        }
        tf::Task presentTask = taskflow.emplace(
            [ this, frame ]
            {
                PresentContext &presentContext     = m_presentContexts[ frame ];
                const uint32_t  image              = m_presentNode.SwapChain->AcquireNextImage( presentContext.ImageReadySemaphore.get( ) );
                ICommandList  *&presentCommandList = presentContext.PresentCommandList;
                presentCommandList->Begin( );
                IssueBarriers( presentCommandList, presentContext.ResourceUsagesPerFrame );
                ITextureResource *swapChainRenderTarget = m_presentNode.SwapChain->GetRenderTarget( image );

                presentCommandList->PipelineBarrier( PipelineBarrierDesc::UndefinedToRenderTarget( swapChainRenderTarget ) );
                m_presentNode.Execute->Execute( frame, presentCommandList, swapChainRenderTarget );
                presentCommandList->PipelineBarrier( PipelineBarrierDesc::RenderTargetToPresent( swapChainRenderTarget ) );

                ExecuteDesc presentExecuteDesc{ };
                presentExecuteDesc.Notify = m_frameFences[ frame ].get( );
                presentExecuteDesc.WaitOnSemaphores.AddElement( presentContext.ImageReadySemaphore.get( ) );
                for ( auto &dependency : presentContext.PresentDependencySemaphores )
                {
                    presentExecuteDesc.WaitOnSemaphores.AddElement( dependency );
                }
                presentExecuteDesc.NotifySemaphores.AddElement( presentContext.ImageRenderedSemaphore.get( ) );
                presentCommandList->Execute( presentExecuteDesc );

                InteropArray<ISemaphore *> presentSemaphores;
                presentSemaphores.AddElement( presentContext.ImageRenderedSemaphore.get( ) );
                presentCommandList->Present( m_presentNode.SwapChain, image, presentSemaphores );
            } );

        for ( int i = 0; i < m_presentNode.Dependencies.NumElements( ); ++i )
        {
            auto &dependency = m_presentNode.Dependencies.GetElement( i );
            presentTask.succeed( tasks[ dependency.Get( ) ] );
        }
    }
}

void RenderGraph::Update( )
{
    m_frameFences[ m_frameIndex ]->Wait( );
    m_executor.run( m_frameTaskflows[ m_frameIndex ] ).wait( );
    m_frameIndex = ( m_frameIndex + 1 ) % m_desc.NumFrames;
}

void RenderGraph::WaitIdle( ) const
{
    m_frameFences[ m_frameIndex ]->Wait( );
}

ISemaphore *RenderGraph::GetOrCreateSemaphore( uint32_t &index )
{
    if ( index >= m_nodeSemaphores.size( ) )
    {
        m_nodeSemaphores.emplace_back( m_desc.LogicalDevice->CreateSemaphore( ) );
    }
    return m_nodeSemaphores[ index++ ].get( );
}

void RenderGraph::ValidateNodes( ) const
{
    std::unordered_set<std::string> allNodes;
    for ( auto &node : m_nodeDescriptions )
    {
        allNodes.insert( node.Name.Get( ) );
    }
    for ( auto &node : m_nodeDescriptions )
    {
        ValidateDependencies( allNodes, node.Dependencies );
    }
    if ( m_hasPresentNode )
    {
        ValidateDependencies( allNodes, m_presentNode.Dependencies );
    }
}

void RenderGraph::ValidateDependencies( const std::unordered_set<std::string> &allNodes, const InteropArray<InteropString> &dependencies ) const
{
    for ( int i = 0; i < dependencies.NumElements( ); ++i )
    {
        auto &dependency = dependencies.GetElement( i );
        if ( !allNodes.contains( dependency.Get( ) ) )
        {
            LOG( ERROR ) << "Node has a dependency " << dependency.Get( ) << " that does not exist.";
        }
    }
}

void RenderGraph::IssueBarriers( ICommandList *commandList, const std::vector<NodeResourceUsageDesc> &resourceUsages )
{
    PipelineBarrierDesc       barrierDesc{ };
    std::vector<std::mutex *> m_unlocks;

    for ( auto &resourceState : resourceUsages )
    {
        if ( resourceState.Type == NodeResourceUsageType::Texture )
        {
            auto  texture     = resourceState.TextureResource;
            auto &lockedState = m_resourceLocking.TextureStates[ texture ];
            if ( lockedState.State != resourceState.State )
            {
                lockedState.Mutex.lock( );
                m_unlocks.push_back( &lockedState.Mutex );
                barrierDesc.TextureBarrier( TextureBarrierDesc{ texture, lockedState.State, resourceState.State } );
                lockedState.State = resourceState.State;
            }
        }
        else
        {
            auto  buffer      = resourceState.BufferResource;
            auto &lockedState = m_resourceLocking.BufferStates[ buffer ];
            if ( lockedState.State != resourceState.State )
            {
                lockedState.Mutex.lock( );
                m_unlocks.push_back( &lockedState.Mutex );
                barrierDesc.BufferBarrier( BufferBarrierDesc{ buffer, lockedState.State, resourceState.State } );
                lockedState.State = resourceState.State;
            }
        }
    }

    if ( barrierDesc.GetTextureBarriers( ).NumElements( ) != 0 || barrierDesc.GetBufferBarriers( ).NumElements( ) != 0 )
    {
        commandList->PipelineBarrier( barrierDesc );
        for ( const auto &mutex : m_unlocks )
        {
            mutex->unlock( );
        }
    }
}
