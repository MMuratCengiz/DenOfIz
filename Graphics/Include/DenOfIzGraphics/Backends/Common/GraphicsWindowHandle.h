/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <stdbool.h>
#include <stdint.h>
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Macro.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /** Refers to an SDL3 window, see CreateFromSDLWindowId. Destroying the handle leaves the window alone. */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_GraphicsWindowHandle )

    typedef enum
    {
        DENOFIZ_FULLSCREEN_MODE_WINDOWED   = 0,
        DENOFIZ_FULLSCREEN_MODE_BORDERLESS = 1,
        DENOFIZ_FULLSCREEN_MODE_EXCLUSIVE  = 2
    } DenOfIz_FullScreenMode;

    /** Window size in pixels, zero when there is no window. */
    typedef struct
    {
        uint32_t Width;
        uint32_t Height;
    } DenOfIz_GraphicsWindowSurface;

    DZ_API DenOfIz_GraphicsWindowHandle DenOfIz_GraphicsWindowHandle_Create( );
    DZ_API void                         DenOfIz_GraphicsWindowHandle_CreateFromSDLWindowId( DenOfIz_GraphicsWindowHandle handle, uint32_t windowId );
    DZ_API void                         DenOfIz_GraphicsWindowHandle_GetSDLWindowId( DenOfIz_GraphicsWindowHandle handle, uint32_t *outWindowId );
    DZ_API void                         DenOfIz_GraphicsWindowHandle_GetSurface( DenOfIz_GraphicsWindowHandle handle, DenOfIz_GraphicsWindowSurface *outSurface );
    DZ_API void                         DenOfIz_GraphicsWindowHandle_IsValid( DenOfIz_GraphicsWindowHandle handle, bool *outIsValid );
    DZ_API void                         DenOfIz_GraphicsWindowHandle_SetFullScreenMode( DenOfIz_GraphicsWindowHandle handle, DenOfIz_FullScreenMode mode );
    DZ_API void                         DenOfIz_GraphicsWindowHandle_GetFullScreenMode( DenOfIz_GraphicsWindowHandle handle, DenOfIz_FullScreenMode *outMode );
    DZ_API void                         DenOfIz_GraphicsWindowHandle_Destroy( DenOfIz_GraphicsWindowHandle handle );

#ifdef __cplusplus
}
#endif
