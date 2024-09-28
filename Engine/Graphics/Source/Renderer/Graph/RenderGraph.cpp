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

RenderGraph::RenderGraph( const RenderGraphDesc &desc ) : m_presentNode( { } ), m_desc( desc )
{
    m_frameFences.resize( m_desc.NumFrames );
    m_imageReadySemaphores.resize( m_desc.NumFrames );
    m_imageRenderedSemaphores.resize( m_desc.NumFrames );

    for ( uint8_t i = 0; i < m_desc.NumFrames; ++i )
    {
        m_frameFences[ i ]             = desc.LogicalDevice->CreateFence( );
        m_imageReadySemaphores[ i ]    = desc.LogicalDevice->CreateSemaphore( );
        m_imageRenderedSemaphores[ i ] = desc.LogicalDevice->CreateSemaphore( );
    }

    CommandListPoolDesc poolDesc{ };
    poolDesc.NumCommandLists = m_desc.NumCommandLists;
    m_commandListPools.clear( );
    for ( int i = 0; i < m_desc.NumFrames; ++i )
    {
        m_commandListPools.push_back( m_desc.LogicalDevice->CreateCommandListPool( poolDesc ) );
    }
}

void RenderGraph::Reset( )
{
    m_nodes.clear( );
    m_presentDependencySemaphores.clear( );
    m_presentDependencySemaphores.resize( m_desc.NumFrames );
    m_nodeDescriptions.clear( );
    m_hasPresentNode = false;
}

void RenderGraph::AddNode( const NodeDesc &desc )
{
    m_nodeDescriptions.push_back( desc );
}

void RenderGraph::AddPresentNode( const PresentNodeDesc &desc )
{
    m_hasPresentNode = true;
    m_presentNode    = desc;
}

void RenderGraph::BuildGraph( )
{
    InitAllNodes( );
    ValidateNodes( );
    ConfigureGraph( );
}

void RenderGraph::InitAllNodes( )
{
    for ( auto &node : m_nodeDescriptions )
    {
        auto graphNode   = std::make_unique<GraphNode>( );
        graphNode->Index = m_nodes.size( );

        for ( uint8_t i = 0; i < m_desc.NumFrames; i++ )
        {
            auto newContext         = std::make_unique<NodeExecutionContext>( );
            newContext->CommandList = m_commandListPools[ i ]->GetCommandLists( )[ graphNode->Index + 1 ]; // +1 for the present node
            newContext->Execute     = node.Execute;
            graphNode->Contexts.push_back( std::move( newContext ) );
        }
        m_nodes.push_back( std::move( graphNode ) );
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
            for ( auto &dependency : node.Dependencies )
            {
                if ( !processedNodes.contains( dependency ) )
                {
                    isDependencyResolved = false;
                    break;
                }
            }

            if ( !isDependencyResolved )
            {
                continue;
            }

            for ( auto &dependency : node.Dependencies )
            {
                ISemaphore *semaphore = GetOrCreateSemaphore( freeSemaphoreIndex );
                for ( uint8_t i = 0; i < m_desc.NumFrames; i++ )
                {
                    m_nodes[ nodeIndex ]->Contexts[ i ]->WaitOnSemaphores.push_back( semaphore );
                    m_nodes[ processedNodes[ dependency ] ]->Contexts[ i ]->NotifySemaphores.push_back( semaphore );
                }
            }
            processedNodes[ node.Name ] = nodeIndex;
        }
    }

    DZ_RETURN_IF( !m_hasPresentNode );
    for ( auto &dependency : m_presentNode.Dependencies )
    {
        ISemaphore *semaphore = GetOrCreateSemaphore( freeSemaphoreIndex );
        for ( uint8_t i = 0; i < m_desc.NumFrames; i++ )
        {
            m_presentDependencySemaphores[ i ].push_back( semaphore );
            m_nodes[ processedNodes[ dependency ] ]->Contexts[ i ]->NotifySemaphores.push_back( semaphore );
        }
    }
}

void RenderGraph::Update( )
{
    m_frameFences[ m_frameIndex ]->Wait( );

    FrameExecutionContext frameExecutionContext{ };
    frameExecutionContext.FrameIndex = m_frameIndex;

    uint32_t nodeIndex = 0;
    for ( auto &node : m_nodes )
    {
        std::unique_ptr<NodeExecutionContext> &context     = node->Contexts[ m_frameIndex ];
        ICommandList                         *&commandList = context->CommandList;
        commandList->Begin( );
        IssueBarriers( commandList, m_nodeDescriptions[ nodeIndex ].ResourceStates );
        context->Execute( &frameExecutionContext, commandList );

        ExecuteDesc executeDesc{ };
        if ( nodeIndex == m_nodes.size( ) - 1 && !m_hasPresentNode )
        {
            executeDesc.Notify = m_frameFences[ m_frameIndex ].get( );
        }
        executeDesc.WaitOnSemaphores = context->WaitOnSemaphores;
        executeDesc.NotifySemaphores = context->NotifySemaphores;
        commandList->Execute( executeDesc );
        nodeIndex++;
    }

    DZ_RETURN_IF( !m_hasPresentNode );

    uint32_t image              = m_presentNode.SwapChain->AcquireNextImage( m_imageReadySemaphores[ m_frameIndex ].get( ) );
    auto     presentCommandList = m_commandListPools[ m_frameIndex ]->GetCommandLists( )[ 0 ];
    presentCommandList->Begin( );
    IssueBarriers( presentCommandList, m_presentNode.ResourceUsages );
    m_presentNode.Execute( &frameExecutionContext, presentCommandList, m_presentNode.SwapChain->GetRenderTarget( image ) );

    ExecuteDesc presentExecuteDesc{ };
    presentExecuteDesc.Notify           = m_frameFences[ m_frameIndex ].get( );
    presentExecuteDesc.WaitOnSemaphores = { m_imageReadySemaphores[ m_frameIndex ].get( ) };
    std::copy( m_presentDependencySemaphores[ m_frameIndex ].begin( ), m_presentDependencySemaphores[ m_frameIndex ].end( ),
               std::back_inserter( presentExecuteDesc.WaitOnSemaphores ) );
    presentExecuteDesc.NotifySemaphores = { m_imageRenderedSemaphores[ m_frameIndex ].get( ) };
    presentCommandList->Execute( presentExecuteDesc );
    presentCommandList->Present( m_presentNode.SwapChain, image, { m_imageRenderedSemaphores[ m_frameIndex ].get( ) } );

    m_frameIndex = ( m_frameIndex + 1 ) % m_desc.NumFrames;
}

void RenderGraph::WaitIdle( )
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

void RenderGraph::ValidateNodes( )
{
    std::unordered_set<std::string> allNodes;
    for ( auto &node : m_nodeDescriptions )
    {
        allNodes.insert( node.Name );
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

void RenderGraph::ValidateDependencies( const std::unordered_set<std::string> &allNodes, const std::vector<std::string> &dependencies )
{
    for ( auto &dependency : dependencies )
    {
        if ( !allNodes.contains( dependency ) )
        {
            LOG( ERROR ) << "Node has a dependency " << dependency << " that does not exist.";
        }
    }
}

void RenderGraph::IssueBarriers( ICommandList *commandList, std::vector<NodeResourceUsageDesc> &resourceUsages )
{
    PipelineBarrierDesc barrierDesc{ };
    for ( auto &resourceState : resourceUsages )
    {
        if ( resourceState.Type == NodeResourceUsageType::Texture )
        {
            auto texture = resourceState.TextureResource;
            if ( !m_textureStates.contains( texture ) )
            {
                m_textureStates[ texture ] = texture->InitialState( );
            }
            barrierDesc.TextureBarrier( TextureBarrierDesc{ texture, m_textureStates[ texture ], resourceState.State } );
            m_textureStates[ texture ] = resourceState.State;
        }
        else
        {
            auto buffer = resourceState.BufferResource;
            if ( !m_bufferStates.contains( buffer ) )
            {
                m_bufferStates[ buffer ] = buffer->InitialState( );
            }
            barrierDesc.BufferBarrier( BufferBarrierDesc{ buffer, m_bufferStates[ buffer ], resourceState.State } );
            m_bufferStates[ buffer ] = resourceState.State;
        }
    }
    commandList->PipelineBarrier( barrierDesc );
}
