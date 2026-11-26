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
     * CPU wait for GPU work. Signalled when the submission that passes it as DenOfIz_ExecuteCommandListsDesc::Signal finishes, Wait
     * before submitting it again.
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_Fence )

    /** Blocks until the last submission that signals the fence finished. Returns immediately for a fence that was never submitted. */
    DZ_API void DenOfIz_Fence_Wait( DenOfIz_Fence fence );
    /** Not needed between submissions, ExecuteCommandLists resets the fence it signals. Do not Wait between Reset and the next submission. */
    DZ_API void DenOfIz_Fence_Reset( DenOfIz_Fence fence );
    /** Destroyed immediately, Wait first when it is pending. */
    DZ_API void DenOfIz_Fence_Destroy( DenOfIz_Fence fence );

#ifdef __cplusplus
}
#endif
