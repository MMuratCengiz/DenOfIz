/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "CommandList.h"
#include "CommandQueue.h"
#include "DenOfIzGraphics/Handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_CommandListPool )

    /** Allocates NumCommandLists command lists of CommandQueue's queue type. */
    typedef struct DenOfIz_CommandListPoolDesc
    {
        DenOfIz_CommandQueue CommandQueue;
        uint32_t             NumCommandLists;
    } DenOfIz_CommandListPoolDesc;

    /** The command lists are owned by the pool. Elements is a new array allocated with malloc, the caller frees it. */
    DZ_API void DenOfIz_CommandListPool_GetCommandLists( DenOfIz_CommandListPool pool, DenOfIz_CommandListArray *outCommandLists );
    /** Destroys the pool and its command lists immediately, none of them may be executing. */
    DZ_API void DenOfIz_CommandListPool_Destroy( DenOfIz_CommandListPool pool );

#ifdef __cplusplus
}
#endif
