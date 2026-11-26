/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "CommandList.h"
#include "DenOfIzGraphics/Handle.h"
#include "Fence.h"
#include "Semaphore.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_CommandQueue )

    typedef struct DenOfIz_CommandQueueFlags
    {
        bool RequirePresentationSupport;
    } DenOfIz_CommandQueueFlags;

    /** Used by DX12 only, where LOW (the zero default) is D3D12 NORMAL, NORMAL is HIGH and HIGH is GLOBAL_REALTIME. */
    typedef enum DenOfIz_QueuePriority
    {
        DENOFIZ_QUEUE_PRIORITY_LOW,
        DENOFIZ_QUEUE_PRIORITY_NORMAL,
        DENOFIZ_QUEUE_PRIORITY_HIGH
    } DenOfIz_QueuePriority;

    typedef struct DenOfIz_CommandQueueDesc
    {
        DenOfIz_QueueType         QueueType;
        DenOfIz_QueuePriority     Priority;
        DenOfIz_CommandQueueFlags Flags;
    } DenOfIz_CommandQueueDesc;

    typedef struct DenOfIz_ExecuteCommandListsDesc
    {
        DenOfIz_Fence            Signal;       /**< Optional, signalled when every command list finished. */
        DenOfIz_CommandListArray CommandLists; /**< Ended command lists of a type the queue accepts, executed in order. */
        DenOfIz_SemaphoreArray   WaitSemaphores;
        DenOfIz_SemaphoreArray   SignalSemaphores;
    } DenOfIz_ExecuteCommandListsDesc;

    /** Blocks until the queue's submitted work finished. On Metal it waits for every queue of the device. */
    DZ_API void DenOfIz_CommandQueue_WaitIdle( DenOfIz_CommandQueue queue );
    DZ_API void DenOfIz_CommandQueue_ExecuteCommandLists( DenOfIz_CommandQueue queue, const DenOfIz_ExecuteCommandListsDesc *executeCommandListsDesc );
    /** Destroyed immediately. DX12 and Vulkan wait for the queue to go idle first. */
    DZ_API void DenOfIz_CommandQueue_Destroy( DenOfIz_CommandQueue queue );

#ifdef __cplusplus
}
#endif
