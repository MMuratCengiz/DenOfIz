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

#include <memory>
#include "DenOfIzGraphics/Backends/Common/GraphicsWindowHandle.h"
#include "DenOfIzGraphics/Utilities/Engine.h"
#include "DenOfIzGraphics/Utilities/Interop.h"

namespace DenOfIz
{
    struct DZ_API WindowFlags
    {
        bool None              = false;
        bool Fullscreen        = false;
        bool OpenGL            = false;
        bool Shown             = false;
        bool Hidden            = false;
        bool Borderless        = false;
        bool Resizable         = false;
        bool Minimized         = false;
        bool Maximized         = false;
        bool InputGrabbed      = false;
        bool InputFocus        = false;
        bool MouseFocus        = false;
        bool FullscreenDesktop = false;
        bool Foreign           = false;
        bool HighDPI           = false;
        bool MouseCapture      = false;
        bool AlwaysOnTop       = false;
        bool SkipTaskbar       = false;
        bool Utility           = false;
        bool Tooltip           = false;
        bool PopupMenu         = false;
        bool Vulkan            = false;
    };

    enum class DZ_API WindowPosition
    {
        Undefined = 0x1FFF0000,
        Centered  = 0x2FFF0000
    };

    struct DZ_API WindowSize
    {
        int Width;
        int Height;
    };

    struct DZ_API WindowCoords
    {
        int X;
        int Y;
    };

    struct DZ_API WindowDesc
    {
        InteropString  Title;
        int            X;
        int            Y;
        int            Width;
        int            Height;
        WindowFlags    Flags;
        WindowPosition Position = WindowPosition::Undefined; // WindowPosition::Centered overwrites X, Y
    };

    class Window
    {
        struct Impl;
        std::unique_ptr<Impl> m_impl;

    public:
        DZ_API explicit Window( const WindowDesc &properties );
        DZ_API ~Window( );

        DZ_API void Destroy( ) const;
        DZ_API void Show( ) const;
        DZ_API void Hide( ) const;
        DZ_API void Minimize( ) const;
        DZ_API void Maximize( ) const;
        DZ_API void Raise( ) const;
        DZ_API void Restore( ) const;

        DZ_API [[nodiscard]] GraphicsWindowHandle *GetGraphicsWindowHandle( ) const;
        DZ_API [[nodiscard]] uint32_t              GetWindowID( ) const;
        DZ_API [[nodiscard]] WindowSize            GetSize( ) const;
        DZ_API void                                SetSize( int width, int height ) const;
        DZ_API [[nodiscard]] InteropString         GetTitle( ) const;
        DZ_API void                                SetTitle( const InteropString &title ) const;
        DZ_API [[nodiscard]] bool                  GetFullscreen( ) const;
        DZ_API void                                SetFullscreen( bool fullscreen ) const;
        DZ_API void                                SetPosition( int x, int y ) const;
        DZ_API [[nodiscard]] WindowCoords          GetPosition( ) const;
        DZ_API void                                SetResizable( bool resizable ) const;
        DZ_API void                                SetBordered( bool bordered ) const;
        DZ_API void                                SetMinimumSize( int minWidth, int minHeight ) const;
        DZ_API void                                SetMaximumSize( int maxWidth, int maxHeight ) const;
        DZ_API [[nodiscard]] bool                  IsShown( ) const;
    };

} // namespace DenOfIz
