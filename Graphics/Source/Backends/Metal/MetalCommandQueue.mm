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

#import "DenOfIzGraphicsInternal/Backends/Metal/MetalCommandQueue.h"
#import "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

MetalCommandQueue::MetalCommandQueue(MetalContext* context, const CommandQueueDesc& desc)
    : m_context(context), m_desc(desc)
{
    @autoreleasepool
    {
        m_queue = [m_context->Device newCommandQueue];

        if (!m_queue)
        {
            spdlog::critical("Failed to create Metal command queue");
        }
    }
}

MetalCommandQueue::~MetalCommandQueue()
{
}

void MetalCommandQueue::WaitIdle()
{

}

void MetalCommandQueue::ExecuteCommandLists(const ExecuteCommandListsDesc& executeCommandListsDesc)
{
    @autoreleasepool
    {
        bool waitRequired = false;
        for (uint32_t i = 0; i < executeCommandListsDesc.WaitSemaphores.NumElements; ++i)
        {
            MetalSemaphore* semaphore = static_cast<MetalSemaphore*>(executeCommandListsDesc.WaitSemaphores.Elements[i]);
            if (semaphore->IsSignaled())
            {
                waitRequired = true;
                break;
            }
        }

        if (waitRequired)
        {
            id<MTLCommandBuffer> waitCommandBuffer = [m_queue commandBufferWithUnretainedReferences];

            for (uint32_t i = 0; i < executeCommandListsDesc.WaitSemaphores.NumElements; ++i)
            {
                MetalSemaphore* semaphore = static_cast<MetalSemaphore*>(executeCommandListsDesc.WaitSemaphores.Elements[i]);
                if (semaphore->IsSignaled())
                {
                    semaphore->WaitFor(waitCommandBuffer);
                    semaphore->ResetSignaled();
                }
            }

            [waitCommandBuffer commit];
            waitCommandBuffer = nil;
        }

        for (uint32_t i = 0; i < executeCommandListsDesc.CommandLists.NumElements; i++)
        {
            MetalCommandList* cmdList = static_cast<MetalCommandList*>(executeCommandListsDesc.CommandLists.Elements[i]);

            if (executeCommandListsDesc.Signal)
            {
                MetalFence* metalFence = static_cast<MetalFence*>(executeCommandListsDesc.Signal);
                metalFence->NotifyOnCommandBufferCompletion(cmdList->GetCommandBuffer());
            }

            for (uint32_t j = 0; j < executeCommandListsDesc.SignalSemaphores.NumElements; ++j)
            {
                MetalSemaphore* metalSemaphore = static_cast<MetalSemaphore*>(executeCommandListsDesc.SignalSemaphores.Elements[j]);
                metalSemaphore->NotifyOnCommandBufferCompletion(cmdList->GetCommandBuffer());
            }

            [cmdList->GetCommandBuffer() commit];
        }
    }
}