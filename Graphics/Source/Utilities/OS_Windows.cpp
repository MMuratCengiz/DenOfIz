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

#include "DenOfIzGraphics/Utilities/OS.h"

#define WIN32_LEAN_AND_MEAN
#include "Windows.h"

extern "C"
{

    float DenOfIz_OS_GetDisplayScale( int display )
    {
        const HDC hdc  = GetDC( NULL );
        const int dpiX = GetDeviceCaps( hdc, LOGPIXELSX );
        ReleaseDC( NULL, hdc );
        return (float)dpiX / 96.0f;
    }

    float DenOfIz_OS_GetWindowSizeScaleFactor( int display )
    {
        return 1.0f;
    }
}
