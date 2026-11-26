/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphics/Handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Orders GPU work between submissions, passed in DenOfIz_ExecuteCommandListsDesc::SignalSemaphores and WaitSemaphores. A wait waits
     * for the most recent signal submitted before it.
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_Semaphore )

    typedef struct DenOfIz_SemaphoreArray
    {
        DenOfIz_Semaphore *Elements;
        size_t             NumElements;
    } DenOfIz_SemaphoreArray;

    /** Signals from the CPU on DX12 only. Vulkan advances the value waits expect without signalling it, Metal does nothing. */
    DZ_API void DenOfIz_Semaphore_Notify( DenOfIz_Semaphore semaphore );
    /**
     * DX12 and Vulkan: the GPU reached the most recent signal (DX12 also answers true before the first signal). Metal: a submission
     * signalled it and no submission has waited on it since, whether or not the GPU finished.
     */
    DZ_API void DenOfIz_Semaphore_IsCompleted( DenOfIz_Semaphore semaphore, bool *outCompleted );
    /** Destroyed immediately, no pending submission may wait on or signal it. */
    DZ_API void DenOfIz_Semaphore_Destroy( DenOfIz_Semaphore semaphore );

#ifdef __cplusplus
}
#endif
