/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#import "DenOfIzGraphicsInternal/Backends/Metal/MetalCommandQueue.h"
#import "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

MetalCommandQueue::MetalCommandQueue(MetalContext* context, const DenOfIz_CommandQueueDesc& desc)
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
    @autoreleasepool
    {
        m_context->WaitIdle();
    }
}

void MetalCommandQueue::ExecuteCommandLists(const DenOfIz_ExecuteCommandListsDesc& executeCommandListsDesc)
{
    @autoreleasepool
    {
        bool waitRequired = false;
        for (size_t i = 0; i < executeCommandListsDesc.WaitSemaphores.NumElements; ++i)
        {
            MetalSemaphore* semaphore = static_cast<MetalSemaphore*>(DENOFIZ_FROM_HANDLE(ISemaphore, executeCommandListsDesc.WaitSemaphores.Elements[i]));
            if (semaphore->IsSignaled())
            {
                waitRequired = true;
                break;
            }
        }

        if (waitRequired)
        {
            id<MTLCommandBuffer> waitCommandBuffer = [m_queue commandBufferWithUnretainedReferences];

            for (size_t i = 0; i < executeCommandListsDesc.WaitSemaphores.NumElements; ++i)
            {
                MetalSemaphore* semaphore = static_cast<MetalSemaphore*>(DENOFIZ_FROM_HANDLE(ISemaphore, executeCommandListsDesc.WaitSemaphores.Elements[i]));
                if (semaphore->IsSignaled())
                {
                    semaphore->WaitFor(waitCommandBuffer);
                    semaphore->ResetSignaled();
                }
            }

            [waitCommandBuffer commit];
            m_context->TrackCommandBuffer(waitCommandBuffer);
            waitCommandBuffer = nil;
        }

        const size_t numCommandLists = executeCommandListsDesc.CommandLists.NumElements;
        for (size_t i = 0; i < numCommandLists; i++)
        {
            MetalCommandList* cmdList = static_cast<MetalCommandList*>(DENOFIZ_FROM_HANDLE(ICommandList, executeCommandListsDesc.CommandLists.Elements[i]));

            if (i == numCommandLists - 1 && DENOFIZ_HANDLE_IS_VALID(executeCommandListsDesc.Signal))
            {
                MetalFence* metalFence = static_cast<MetalFence*>(DENOFIZ_FROM_HANDLE(IFence, executeCommandListsDesc.Signal));
                metalFence->NotifyOnCommandBufferCompletion(cmdList->GetCommandBuffer());
            }

            for (size_t j = 0; j < executeCommandListsDesc.SignalSemaphores.NumElements; ++j)
            {
                MetalSemaphore* metalSemaphore = static_cast<MetalSemaphore*>(DENOFIZ_FROM_HANDLE(ISemaphore, executeCommandListsDesc.SignalSemaphores.Elements[j]));
                metalSemaphore->NotifyOnCommandBufferCompletion(cmdList->GetCommandBuffer());
            }

            [cmdList->GetCommandBuffer() commit];
            m_context->TrackCommandBuffer(cmdList->GetCommandBuffer());
        }
    }
}