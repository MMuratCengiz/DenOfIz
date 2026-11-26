/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
