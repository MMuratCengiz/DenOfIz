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

    struct DZ_API NodeResourceUsageDesc
    {
        explicit NodeResourceUsageDesc( IBufferResource *resource );
        explicit NodeResourceUsageDesc( ITextureResource *resource );
        NodeResourceUsageDesc( ) = default;

        uint32_t              FrameIndex = 0;
        ResourceState         State      = ResourceState::Undefined;
        NodeResourceUsageType Type       = NodeResourceUsageType::Buffer;
        union
        {
            IBufferResource  *BufferResource;
            ITextureResource *TextureResource;
        };
        static NodeResourceUsageDesc BufferState( const uint32_t frameIndex, IBufferResource *bufferResource, const ResourceState state );
        static NodeResourceUsageDesc TextureState( const uint32_t frameIndex, ITextureResource *textureResource, const ResourceState state );
    };
    template class DZ_API InteropArray<NodeResourceUsageDesc>;

    // Interop friendly callback
    class DZ_API NodeExecutionCallback
    {
    public:
        virtual ~NodeExecutionCallback( ) {};
        virtual void Execute( uint32_t frameIndex, ICommandList *commandList ) {};
    };

    namespace RenderGraphInternal
    {
        // Odd placement due to dependency on NodeResourceUsageDesc
        struct NodeExecutionContext
        {
            ICommandList                      *CommandList;
            InteropArray<ISemaphore *>         WaitOnSemaphores;
            InteropArray<ISemaphore *>         NotifySemaphores;
            std::vector<NodeResourceUsageDesc> ResourceUsagesPerFrame;
            std::mutex                         SelfMutex;
            NodeExecutionCallback             *Execute;
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
    } // namespace RenderGraphInternal
    using namespace RenderGraphInternal;

    struct DZ_API NodeDesc
    {
        InteropString                       Name;
        InteropArray<InteropString>         Dependencies;
        InteropArray<NodeResourceUsageDesc> RequiredStates;
        NodeExecutionCallback              *Execute;
    };

    class DZ_API PresentExecutionCallback
    {
    public:
        virtual ~PresentExecutionCallback( ) {};
        virtual void Execute( uint32_t frameIndex, ICommandList *commandList, ITextureResource *texture ) {};
    };

    struct DZ_API PresentNodeDesc
    {
        InteropArray<InteropString>         Dependencies;
        InteropArray<NodeResourceUsageDesc> RequiredStates;
        ISwapChain                         *SwapChain;
        PresentExecutionCallback           *Execute;
    };

    struct DZ_API RenderGraphDesc
    {
        GraphicsApi    *GraphicsApi;
        ILogicalDevice *LogicalDevice;
        ISwapChain     *SwapChain;
        uint8_t         NumFrames       = 3;
        uint32_t        NumCommandLists = 16;
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
        DZ_API explicit RenderGraph( const RenderGraphDesc &desc );
        DZ_API void Reset( );
        DZ_API void AddNode( const NodeDesc &desc );
        DZ_API void SetPresentNode( const PresentNodeDesc &desc );
        DZ_API void BuildGraph( );
        DZ_API void BuildTaskflow( );
        DZ_API void Update( );
        DZ_API void WaitIdle( ) const;

    private:
        ISemaphore *GetOrCreateSemaphore( uint32_t &index );
        void        InitAllNodes( );
        void        ConfigureGraph( );
        void        ValidateDependencies( const std::unordered_set<std::string> &allNodes, const InteropArray<InteropString> &dependencies ) const;
        void        ValidateNodes( ) const;
        void        IssueBarriers( ICommandList *commandList, const std::vector<NodeResourceUsageDesc> &resourceUsages );
    };
} // namespace DenOfIz
