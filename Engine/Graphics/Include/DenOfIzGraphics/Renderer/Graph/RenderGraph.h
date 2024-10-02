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
#pragma once

#include <DenOfIzGraphics/Backends/GraphicsApi.h>
#include <taskflow/taskflow.hpp>

/// <summary>
/// RenderGraph.h manages command list executions and resource transitions. Resources should not be transitioned outside the graph.
/// Every node is executed in parallel, fences and semaphores are used to synchronize the nodes automatically according to their dependencies.
/// </summary>
namespace DenOfIz
{
    enum class NodeResourceUsageType
    {
        Buffer,
        Texture
    };

    struct NodeResourceUsageDesc
    {
    private:
        explicit NodeResourceUsageDesc( IBufferResource *resource ) : BufferResource( resource )
        {
        }
        explicit NodeResourceUsageDesc( ITextureResource *resource ) : Type( NodeResourceUsageType::Texture ), TextureResource( resource )
        {
        }

    public:
        uint32_t              FrameIndex = 0;
        ResourceState         State      = ResourceState::Undefined;
        NodeResourceUsageType Type       = NodeResourceUsageType::Buffer;
        union
        {
            IBufferResource  *BufferResource;
            ITextureResource *TextureResource;
        };

        static NodeResourceUsageDesc BufferState( const uint32_t frameIndex, IBufferResource *bufferResource, const ResourceState state )
        {
            NodeResourceUsageDesc desc( bufferResource );
            desc.FrameIndex = frameIndex;
            desc.State      = state;
            return desc;
        }

        static NodeResourceUsageDesc TextureState( const uint32_t frameIndex, ITextureResource *textureResource, const ResourceState state )
        {
            NodeResourceUsageDesc desc( textureResource );
            desc.FrameIndex = frameIndex;
            desc.State      = state;
            return desc;
        }
    };

    namespace RenderGraphInternal
    {
        // Odd placement due to dependency on NodeResourceUsageDesc
        struct NodeExecutionContext
        {
            ICommandList                                   *CommandList;
            std::vector<ISemaphore *>                       WaitOnSemaphores;
            std::vector<ISemaphore *>                       NotifySemaphores;
            std::vector<NodeResourceUsageDesc>              ResourceUsagesPerFrame;
            std::mutex                                      SelfMutex;
            std::function<void( uint32_t, ICommandList * )> Execute;
        };

        struct GraphNode
        {
            uint32_t                                           Index;
            std::vector<std::unique_ptr<NodeExecutionContext>> Contexts;
        };

        struct PresentContext
        {
            std::vector<NodeResourceUsageDesc> ResourceUsagesPerFrame;
            std::vector<ISemaphore *>          PresentDependencySemaphores;
            ICommandList                      *PresentCommandList;
            std::unique_ptr<ISemaphore>        ImageReadySemaphore;
            std::unique_ptr<ISemaphore>        ImageRenderedSemaphore;
        };
    } // namespace RenderGraphInternal
    using namespace RenderGraphInternal;

    struct NodeDesc
    {
        std::string                                     Name;
        std::vector<std::string>                        Dependencies;
        std::vector<NodeResourceUsageDesc>              RequiredResourceStates;
        std::function<void( uint32_t, ICommandList * )> Execute;
    };

    struct PresentNodeDesc
    {
        std::vector<std::string>                                            Dependencies;
        std::vector<NodeResourceUsageDesc>                                  RequiredResourceStates;
        ISwapChain                                                         *SwapChain;
        std::function<void( uint32_t, ICommandList *, ITextureResource * )> Execute;
    };

    struct RenderGraphDesc
    {
        GraphicsApi    *GraphicsApi;
        ILogicalDevice *LogicalDevice;
        ISwapChain     *SwapChain;
        uint8_t         NumFrames       = 3;
        uint32_t        NumCommandLists = 16;
    };

    struct ResourceLockedState
    {
        ResourceState State = ResourceState::Undefined;
        std::mutex    Mutex;
    };

    struct ResourceLocking
    {
        std::unordered_map<ITextureResource *, ResourceLockedState> TextureStates;
        std::unordered_map<IBufferResource *, ResourceLockedState>  BufferStates;
    };

    class RenderGraph
    {
        uint32_t                                       m_frameIndex     = 0;
        bool                                           m_hasPresentNode = false;
        std::vector<NodeDesc>                          m_nodeDescriptions;
        std::vector<std::unique_ptr<GraphNode>>        m_nodes;
        PresentNodeDesc                                m_presentNode;
        RenderGraphDesc                                m_desc;
        std::vector<std::unique_ptr<ICommandListPool>> m_commandListPools; // Each entry is for a single frame
        std::vector<std::unique_ptr<ICommandList>>     m_commandLists;
        std::vector<std::unique_ptr<ISemaphore>>       m_nodeSemaphores;
        std::vector<std::unique_ptr<IFence>>           m_frameFences;
        std::vector<PresentContext>                    m_presentContexts;

        std::vector<tf::Taskflow> m_frameTaskflows;
        tf::Executor              m_executor;

        ResourceLocking m_resourceLocking;

    public:
        explicit RenderGraph( const RenderGraphDesc &desc );
        void     Reset( );
        void     AddNode( const NodeDesc &desc );
        void     SetPresentNode( const PresentNodeDesc &desc );
        void     BuildGraph( );
        void     BuildTaskflow( );
        void     Update( );
        void     WaitIdle( ) const;

    private:
        ISemaphore *GetOrCreateSemaphore( uint32_t &index );
        void        InitAllNodes( );
        void        ConfigureGraph( );
        void        ValidateDependencies( const std::unordered_set<std::string> &allNodes, const std::vector<std::string> &dependencies ) const;
        void        ValidateNodes( ) const;
        void        IssueBarriers( ICommandList *commandList, const std::vector<NodeResourceUsageDesc> &resourceUsages );
    };
} // namespace DenOfIz
