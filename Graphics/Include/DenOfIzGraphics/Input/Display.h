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

#include <stddef.h>
#include <stdint.h>
#include "DenOfIzGraphics/Utilities/Common_Macro.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        int32_t Width;
        int32_t Height;
    } DenOfIz_DisplaySize;

    typedef struct
    {
        uint32_t            ID;
        DenOfIz_DisplaySize Size;
        float               DpiScale;
        bool                IsPrimary;
    } DenOfIz_Display;

    typedef struct DenOfIz_DisplayArray
    {
        DenOfIz_Display *Elements;
        size_t           NumElements;
    } DenOfIz_DisplayArray;

    DZ_API DenOfIz_DisplayArray DenOfIz_Display_GetDisplays( );
    DZ_API DenOfIz_Display      DenOfIz_Display_GetPrimaryDisplay( );
    DZ_API void                 DenOfIz_Display_RefreshDisplays( );

#ifdef __cplusplus
}
#endif
