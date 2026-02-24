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

#include <stdbool.h>
#include <stdint.h>
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Macro.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Pointer to a native window intended to be used as bring your own window manager structure, although currently it is tied directly to SDL3
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_GraphicsWindowHandle )

    typedef enum
    {
        DENOFIZ_FULLSCREEN_MODE_WINDOWED   = 0,
        DENOFIZ_FULLSCREEN_MODE_BORDERLESS = 1,
        DENOFIZ_FULLSCREEN_MODE_EXCLUSIVE  = 2
    } DenOfIz_FullScreenMode;

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
