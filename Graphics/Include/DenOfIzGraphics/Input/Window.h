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
#include "DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Input/Display.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/Common_Macro.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DenOfIz_WindowFlags
    {
        bool None;
        bool Fullscreen;
        bool OpenGL;
        bool Hidden;
        bool Borderless;
        bool Resizable;
        bool Minimized;
        bool Maximized;
        bool MouseGrabbed;
        bool InputFocus;
        bool MouseFocus;
        bool MouseCapture;
        bool AlwaysOnTop;
        bool Utility;
        bool Tooltip;
        bool PopupMenu;
        bool Vulkan;
    } DenOfIz_WindowFlags;

    typedef enum
    {
        DENOFIZ_WINDOW_POSITION_UNDEFINED = 0x1FFF0000,
        DENOFIZ_WINDOW_POSITION_CENTERED  = 0x2FFF0000
    } DenOfIz_WindowPosition;

    typedef struct DenOfIz_DisplayDpi
    {
        float DiagonalDpi;
        float VerticalDpi;
        float HorizontalDpi;
    } DenOfIz_DisplayDpi;

    typedef struct DenOfIz_WindowDesc
    {
        DenOfIz_StringView     Title;
        int32_t                X;
        int32_t                Y;
        int32_t                Width;
        int32_t                Height;
        DenOfIz_WindowFlags    Flags;
        DenOfIz_WindowPosition Position;
    } DenOfIz_WindowDesc;

    DENOFIZ_DEFINE_HANDLE( DenOfIz_Window )

    typedef struct DenOfIz_PopupWindowDesc
    {
        DenOfIz_Window          Parent;
        int32_t                 OffsetX;
        int32_t                 OffsetY;
        int32_t                 Width;
        int32_t                 Height;
        DenOfIz_WindowFlags     Flags;
    } DenOfIz_PopupWindowDesc;

    DZ_API DenOfIz_Window DenOfIz_Window_Create( const DenOfIz_WindowDesc *desc );
    DZ_API DenOfIz_Window DenOfIz_Window_CreatePopup( const DenOfIz_PopupWindowDesc *desc );
    DZ_API void           DenOfIz_Window_Destroy( DenOfIz_Window window );
    DZ_API void           DenOfIz_Window_Show( DenOfIz_Window window );
    DZ_API void           DenOfIz_Window_Hide( DenOfIz_Window window );
    DZ_API void           DenOfIz_Window_Minimize( DenOfIz_Window window );
    DZ_API void           DenOfIz_Window_Maximize( DenOfIz_Window window );
    DZ_API void           DenOfIz_Window_Raise( DenOfIz_Window window );
    DZ_API void           DenOfIz_Window_Restore( DenOfIz_Window window );

    DZ_API DenOfIz_GraphicsWindowHandle DenOfIz_Window_GetGraphicsWindowHandle( DenOfIz_Window window );
    DZ_API uint32_t                     DenOfIz_Window_GetWindowID( DenOfIz_Window window );

    DZ_API DenOfIz_DisplaySize DenOfIz_Window_GetSize( DenOfIz_Window window );
    DZ_API DenOfIz_DisplaySize DenOfIz_Window_GetSizeInPixels( DenOfIz_Window window );
    DZ_API void                DenOfIz_Window_SetSize( DenOfIz_Window window, int32_t width, int32_t height );
    DZ_API float               DenOfIz_Window_GetPixelDensity( DenOfIz_Window window );
    DZ_API float               DenOfIz_Window_GetDisplayScale( DenOfIz_Window window );

    DZ_API DenOfIz_StringView DenOfIz_Window_GetTitle( DenOfIz_Window window );
    DZ_API void               DenOfIz_Window_SetTitle( DenOfIz_Window window, DenOfIz_StringView title );
    DZ_API bool               DenOfIz_Window_GetFullscreen( DenOfIz_Window window );
    DZ_API void               DenOfIz_Window_SetFullscreen( DenOfIz_Window window, bool fullscreen );
    DZ_API void               DenOfIz_Window_SetPosition( DenOfIz_Window window, int32_t x, int32_t y );
    DZ_API int32_t            DenOfIz_Window_GetPositionX( DenOfIz_Window window );
    DZ_API int32_t            DenOfIz_Window_GetPositionY( DenOfIz_Window window );
    DZ_API void               DenOfIz_Window_SetResizable( DenOfIz_Window window, bool resizable );
    DZ_API void               DenOfIz_Window_SetBordered( DenOfIz_Window window, bool bordered );
    DZ_API void               DenOfIz_Window_SetMinimumSize( DenOfIz_Window window, int32_t minWidth, int32_t minHeight );
    DZ_API void               DenOfIz_Window_SetMaximumSize( DenOfIz_Window window, int32_t maxWidth, int32_t maxHeight );
    DZ_API bool               DenOfIz_Window_IsShown( DenOfIz_Window window );
    DZ_API bool               DenOfIz_Window_IsMinimized( DenOfIz_Window window );
    DZ_API bool               DenOfIz_Window_IsMaximized( DenOfIz_Window window );
    DZ_API uint32_t           DenOfIz_Window_GetFlags( DenOfIz_Window window );
    DZ_API void               DenOfIz_Window_Sync( DenOfIz_Window window );

#ifdef __cplusplus
}
#endif
