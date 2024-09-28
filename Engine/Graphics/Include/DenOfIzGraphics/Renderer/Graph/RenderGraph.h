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
    namespace RenderGraphInternal
    {
        enum class NodeResourceUsageType
        {
            Buffer,
            Texture
        };

        struct FrameExecutionContext
        {
            uint32_t FrameIndex;
        };

        struct NodeExecutionContext
        {
            ICommandList                                                  *CommandList;
            std::vector<ISemaphore *>                                      WaitOnSemaphores;
            std::vector<ISemaphore *>                                      NotifySemaphores;
            PipelineBarrierDesc                                            BarrierDesc;
            std::function<void( FrameExecutionContext *, ICommandList * )> Execute;
        };

        struct GraphNode
        {
            uint32_t                                           Index;
            std::vector<std::unique_ptr<NodeExecutionContext>> Contexts;
        };
    } // namespace RenderGraphInternal
    using namespace RenderGraphInternal;

    struct NodeResourceUsageDesc
    {
    private:
        NodeResourceUsageDesc( ) = default;

    public:
        ResourceState         State = ResourceState::Undefined;
        NodeResourceUsageType Type  = NodeResourceUsageType::Buffer;
        union
        {
            IBufferResource  *BufferResource;
            ITextureResource *TextureResource;
        };

        static NodeResourceUsageDesc BufferUsage( IBufferResource *bufferResource, const ResourceState usage )
        {
            NodeResourceUsageDesc desc{ };
            desc.State          = usage;
            desc.Type           = NodeResourceUsageType::Buffer;
            desc.BufferResource = bufferResource;
            return desc;
        }

        static NodeResourceUsageDesc TextureUsage( ITextureResource *textureResource, const ResourceState usage )
        {
            NodeResourceUsageDesc desc{ };
            desc.State           = usage;
            desc.Type            = NodeResourceUsageType::Texture;
            desc.TextureResource = textureResource;
            return desc;
        }
    };

    struct NodeDesc
    {
        std::string                                                    Name;
        std::vector<std::string>                                       Dependencies;
        std::vector<NodeResourceUsageDesc>                             ResourceStates;
        std::function<void( FrameExecutionContext *, ICommandList * )> Execute;
    };

    struct PresentNodeDesc
    {
        std::vector<std::string>                                                           Dependencies;
        std::vector<NodeResourceUsageDesc>                                                 ResourceUsages;
        ISwapChain                                                                        *SwapChain;
        std::function<void( FrameExecutionContext *, ICommandList *, ITextureResource * )> Execute;
    };

    struct RenderGraphDesc
    {
        GraphicsApi    *GraphicsApi;
        ILogicalDevice *LogicalDevice;
        ISwapChain     *SwapChain;
        uint8_t         NumFrames;
        uint32_t        NumCommandLists = 16;
    };

    class RenderGraph
    {
        uint32_t                                              m_frameIndex     = 0;
        bool                                                  m_hasPresentNode = false;
        std::vector<NodeDesc>                                 m_nodeDescriptions;
        std::vector<std::unique_ptr<GraphNode>>               m_nodes;
        PresentNodeDesc                                       m_presentNode;
        std::unordered_map<ITextureResource *, ResourceState> m_textureStates;
        std::unordered_map<IBufferResource *, ResourceState>  m_bufferStates;
        RenderGraphDesc                                       m_desc;
        std::vector<std::unique_ptr<ICommandListPool>>        m_commandListPools; // Each entry is for a single frame
        std::vector<std::unique_ptr<ICommandList>>            m_commandLists;
        std::vector<std::unique_ptr<ISemaphore>>              m_nodeSemaphores;
        std::vector<std::unique_ptr<IFence>>                  m_frameFences;
        std::vector<std::vector<ISemaphore *>>                m_presentDependencySemaphores; // Each entry is for a single frame
        std::vector<std::unique_ptr<ISemaphore>>              m_imageReadySemaphores;
        std::vector<std::unique_ptr<ISemaphore>>              m_imageRenderedSemaphores;

    public:
        explicit RenderGraph( const RenderGraphDesc &desc );
        void Reset( );
        void AddNode( const NodeDesc &desc );
        void AddPresentNode( const PresentNodeDesc &desc );
        void BuildGraph( );
        void Update( );
        void WaitIdle( );

    private:
        ISemaphore *GetOrCreateSemaphore( uint32_t &index );
        void        InitAllNodes( );
        void        ConfigureGraph( );
        void        ValidateDependencies( const std::unordered_set<std::string> &allNodes, const std::vector<std::string> &dependencies );
        void        ValidateNodes( );
        void        IssueBarriers( ICommandList *commandList, std::vector<NodeResourceUsageDesc> &resourceUsages );
    };
} // namespace DenOfIz
