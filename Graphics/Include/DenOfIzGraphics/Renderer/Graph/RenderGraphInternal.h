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

#include "RenderGraphNodeDesc.h"

namespace DenOfIz::RenderGraphInternal
{
    struct ResourceBarrier
    {
        ITextureResource *Texture;
        IBufferResource  *Buffer;
        ResourceUsage     OldState;
        ResourceUsage     NewState;
    };

    // Odd placement due to dependency on NodeResourceUsageDesc
    struct NodeExecutionContext
    {
        ICommandQueue             *CommandQueue;
        ICommandList              *CommandList;
        InteropArray<ISemaphore *> WaitOnSemaphores;
        InteropArray<ISemaphore *> NotifySemaphores;
        std::mutex                 SelfMutex; // Ensure the same node is not executed concurrently in really fast graphs
        NodeExecutionCallback     *Execute;
    };

    struct GraphNode
    {
        uint32_t                                           CommandListIndex;
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
        QueueType     Queue = QueueType::Graphics;
        ResourceUsage State = ResourceUsage::Undefined;
        std::mutex    Mutex;
    };

    struct ResourceLocking
    {
        std::unordered_map<ITextureResource *, ResourceLockedState> TextureStates;
        std::unordered_map<IBufferResource *, ResourceLockedState>  BufferStates;
    };
} // namespace DenOfIz::RenderGraphInternal
