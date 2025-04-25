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

#include "RenderGraphInternal.h"

#include <DenOfIzGraphics/Renderer/Sync/ResourceTracking.h>
#include <taskflow/taskflow.hpp>

/// <summary>
/// RenderGraph.h manages command list executions and resource transitions. Resources should not be transitioned outside the graph.
/// Every node is executed in parallel, fences and semaphores are used to synchronize the nodes automatically according to their dependencies.
/// </summary>
namespace DenOfIz
{
    using namespace RenderGraphInternal;

    struct DZ_API RenderGraphDesc
    {
        GraphicsApi    *GraphicsApi;
        ILogicalDevice *LogicalDevice;
        ISwapChain     *SwapChain;
        uint8_t         NumFrames               = 3;
        uint32_t        NumGraphicsCommandLists = 8;
        uint32_t        NumComputeCommandLists  = 2;
        uint32_t        NumCopyCommandLists     = 1;
    };

    class RenderGraph
    {
        typedef std::vector<std::unique_ptr<ICommandListPool>> CommandListPoolList;
        uint32_t                                               m_frameIndex     = 0;
        bool                                                   m_hasPresentNode = false;
        std::vector<NodeDesc>                                  m_nodeDescriptions;
        std::vector<std::unique_ptr<GraphNode>>                m_nodes;
        PresentNodeDesc                                        m_presentNode;
        RenderGraphDesc                                        m_desc;
        // CommandList tracking, these ares maps and not arrays since they are not performance critical, only used in initialization, and it's easier to understand
        std::unordered_map<QueueType, CommandListPoolList> m_queueCommandListPools;
        std::unordered_map<QueueType, uint32_t>            m_remainingCommandLists;
        std::unordered_map<QueueType, uint32_t>            m_commandListIndexAtQueue;
        // --
        std::vector<std::unique_ptr<ISemaphore>> m_nodeSemaphores;
        std::vector<std::unique_ptr<IFence>>     m_frameFences;
        std::vector<PresentContext>              m_presentContexts;

        std::unique_ptr<ICommandQueue> m_graphicsCommandQueue;
        std::unique_ptr<ICommandQueue> m_computeCommandQueue;
        std::unique_ptr<ICommandQueue> m_copyCommandQueue;

        std::vector<tf::Taskflow> m_frameTaskflows;
        tf::Executor              m_executor;

        ResourceLocking  m_resourceLocking;
        ResourceTracking m_resourceTracking;

    public:
        DZ_API explicit RenderGraph( const RenderGraphDesc &desc );
        DZ_API void              Reset( );
        DZ_API void              AddNode( const NodeDesc &desc );
        DZ_API void              SetPresentNode( const PresentNodeDesc &desc );
        DZ_API void              BuildGraph( );
        DZ_API void              BuildTaskflow( );
        DZ_API void              Update( );
        DZ_API void              WaitIdle( ) const;
        DZ_API ResourceTracking &GetResourceTracking( );

    private:
        ISemaphore *GetOrCreateSemaphore( uint32_t &index );
        void        InitAllNodes( );
        void        ConfigureGraph( );
        void        ValidateDependencies( const std::unordered_set<std::string> &allNodes, const InteropArray<InteropString> &dependencies ) const;
        void        ValidateNodes( ) const;
    };
} // namespace DenOfIz
