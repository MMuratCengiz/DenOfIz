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

    typedef enum
    {
        DENOFIZ_FLASH_CANCEL        = 0,
        DENOFIZ_FLASH_BRIEFLY       = 1,
        DENOFIZ_FLASH_UNTIL_FOCUSED = 2
    } DenOfIz_FlashOperation;

    typedef enum
    {
        DENOFIZ_PROGRESS_STATE_INVALID       = -1,
        DENOFIZ_PROGRESS_STATE_NONE          = 0,
        DENOFIZ_PROGRESS_STATE_INDETERMINATE = 1,
        DENOFIZ_PROGRESS_STATE_NORMAL        = 2,
        DENOFIZ_PROGRESS_STATE_PAUSED        = 3,
        DENOFIZ_PROGRESS_STATE_ERROR         = 4
    } DenOfIz_ProgressState;

    typedef struct DenOfIz_DisplayDpi
    {
        float DiagonalDpi;
        float VerticalDpi;
        float HorizontalDpi;
    } DenOfIz_DisplayDpi;

    typedef struct DenOfIz_Rect
    {
        int32_t X;
        int32_t Y;
        int32_t Width;
        int32_t Height;
    } DenOfIz_Rect;

    typedef struct DenOfIz_WindowBordersSize
    {
        int32_t Top;
        int32_t Left;
        int32_t Bottom;
        int32_t Right;
    } DenOfIz_WindowBordersSize;

    typedef struct DenOfIz_AspectRatio
    {
        float MinAspect;
        float MaxAspect;
    } DenOfIz_AspectRatio;

    typedef struct DenOfIz_DisplayMode
    {
        uint32_t DisplayID;
        uint32_t Format;
        int32_t  Width;
        int32_t  Height;
        float    PixelDensity;
        float    RefreshRate;
    } DenOfIz_DisplayMode;

    typedef struct DenOfIz_WindowIconDesc
    {
        int32_t              Width;
        int32_t              Height;
        DenOfIz_ByteArrayView Pixels;
    } DenOfIz_WindowIconDesc;

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

    DZ_API void DenOfIz_Window_Show( DenOfIz_Window window );
    DZ_API void DenOfIz_Window_Hide( DenOfIz_Window window );
    DZ_API void DenOfIz_Window_Minimize( DenOfIz_Window window );
    DZ_API void DenOfIz_Window_Maximize( DenOfIz_Window window );
    DZ_API void DenOfIz_Window_Raise( DenOfIz_Window window );
    DZ_API void DenOfIz_Window_Restore( DenOfIz_Window window );

    DZ_API DenOfIz_GraphicsWindowHandle DenOfIz_Window_GetGraphicsWindowHandle( DenOfIz_Window window );
    DZ_API uint32_t                     DenOfIz_Window_GetWindowID( DenOfIz_Window window );

    DZ_API DenOfIz_DisplaySize DenOfIz_Window_GetSize( DenOfIz_Window window );
    DZ_API DenOfIz_DisplaySize DenOfIz_Window_GetSizeInPixels( DenOfIz_Window window );
    DZ_API void                DenOfIz_Window_SetSize( DenOfIz_Window window, int32_t width, int32_t height );
    DZ_API float               DenOfIz_Window_GetPixelDensity( DenOfIz_Window window );
    DZ_API float               DenOfIz_Window_GetDisplayScale( DenOfIz_Window window );

    DZ_API DenOfIz_StringView DenOfIz_Window_GetTitle( DenOfIz_Window window );
    DZ_API void               DenOfIz_Window_SetTitle( DenOfIz_Window window, DenOfIz_StringView title );

    DZ_API bool DenOfIz_Window_GetFullscreen( DenOfIz_Window window );
    DZ_API void DenOfIz_Window_SetFullscreen( DenOfIz_Window window, bool fullscreen );
    DZ_API void DenOfIz_Window_SetFullscreenMode( DenOfIz_Window window, const DenOfIz_DisplayMode *mode );
    DZ_API DenOfIz_DisplayMode DenOfIz_Window_GetFullscreenMode( DenOfIz_Window window );

    DZ_API void    DenOfIz_Window_SetPosition( DenOfIz_Window window, int32_t x, int32_t y );
    DZ_API int32_t DenOfIz_Window_GetPositionX( DenOfIz_Window window );
    DZ_API int32_t DenOfIz_Window_GetPositionY( DenOfIz_Window window );

    DZ_API void               DenOfIz_Window_SetResizable( DenOfIz_Window window, bool resizable );
    DZ_API void               DenOfIz_Window_SetBordered( DenOfIz_Window window, bool bordered );
    DZ_API void               DenOfIz_Window_SetMinimumSize( DenOfIz_Window window, int32_t minWidth, int32_t minHeight );
    DZ_API DenOfIz_DisplaySize DenOfIz_Window_GetMinimumSize( DenOfIz_Window window );
    DZ_API void               DenOfIz_Window_SetMaximumSize( DenOfIz_Window window, int32_t maxWidth, int32_t maxHeight );
    DZ_API DenOfIz_DisplaySize DenOfIz_Window_GetMaximumSize( DenOfIz_Window window );
    DZ_API void               DenOfIz_Window_SetAspectRatio( DenOfIz_Window window, float minAspect, float maxAspect );
    DZ_API DenOfIz_AspectRatio DenOfIz_Window_GetAspectRatio( DenOfIz_Window window );

    DZ_API bool     DenOfIz_Window_IsShown( DenOfIz_Window window );
    DZ_API bool     DenOfIz_Window_IsMinimized( DenOfIz_Window window );
    DZ_API bool     DenOfIz_Window_IsMaximized( DenOfIz_Window window );
    DZ_API uint32_t DenOfIz_Window_GetFlags( DenOfIz_Window window );
    DZ_API void     DenOfIz_Window_Sync( DenOfIz_Window window );

    DZ_API void  DenOfIz_Window_SetIcon( DenOfIz_Window window, const DenOfIz_WindowIconDesc *icon );
    DZ_API void  DenOfIz_Window_Flash( DenOfIz_Window window, DenOfIz_FlashOperation operation );
    DZ_API void  DenOfIz_Window_SetOpacity( DenOfIz_Window window, float opacity );
    DZ_API float DenOfIz_Window_GetOpacity( DenOfIz_Window window );

    DZ_API void DenOfIz_Window_SetAlwaysOnTop( DenOfIz_Window window, bool onTop );
    DZ_API void DenOfIz_Window_SetFocusable( DenOfIz_Window window, bool focusable );
    DZ_API void DenOfIz_Window_SetModal( DenOfIz_Window window, bool modal );

    DZ_API DenOfIz_Window DenOfIz_Window_GetParent( DenOfIz_Window window );
    DZ_API void           DenOfIz_Window_SetParent( DenOfIz_Window window, DenOfIz_Window parent );

    DZ_API void DenOfIz_Window_SetMouseGrab( DenOfIz_Window window, bool grabbed );
    DZ_API bool DenOfIz_Window_GetMouseGrab( DenOfIz_Window window );
    DZ_API void DenOfIz_Window_SetKeyboardGrab( DenOfIz_Window window, bool grabbed );
    DZ_API bool DenOfIz_Window_GetKeyboardGrab( DenOfIz_Window window );
    DZ_API void DenOfIz_Window_SetMouseRect( DenOfIz_Window window, const DenOfIz_Rect *rect );
    DZ_API DenOfIz_Rect DenOfIz_Window_GetMouseRect( DenOfIz_Window window );
    DZ_API void DenOfIz_Window_WarpMouse( DenOfIz_Window window, float x, float y );

    DZ_API DenOfIz_WindowBordersSize DenOfIz_Window_GetBordersSize( DenOfIz_Window window );
    DZ_API DenOfIz_Rect              DenOfIz_Window_GetSafeArea( DenOfIz_Window window );

    DZ_API void                 DenOfIz_Window_SetProgressState( DenOfIz_Window window, DenOfIz_ProgressState state );
    DZ_API DenOfIz_ProgressState DenOfIz_Window_GetProgressState( DenOfIz_Window window );
    DZ_API void                 DenOfIz_Window_SetProgressValue( DenOfIz_Window window, float value );
    DZ_API float                DenOfIz_Window_GetProgressValue( DenOfIz_Window window );

    DZ_API void     DenOfIz_Window_ShowSystemMenu( DenOfIz_Window window, int32_t x, int32_t y );
    DZ_API uint32_t DenOfIz_Window_GetDisplayID( DenOfIz_Window window );
    DZ_API uint32_t DenOfIz_Window_GetPixelFormat( DenOfIz_Window window );

#ifdef __cplusplus
}
#endif
