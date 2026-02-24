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

#include "DenOfIzGraphics/Utilities/Common.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DenOfIz_FSDesc
    {
        DenOfIz_StringView Name;
        DenOfIz_StringView AssetPath;
    } DenOfIz_FSDesc;

    DZ_API void               DenOfIz_FSConfig_InitDefaults( );
    DZ_API void               DenOfIz_FSConfig_Init( const DenOfIz_FSDesc *config );
    DZ_API DenOfIz_StringView DenOfIz_FSConfig_AssetPath( );
    DZ_API DenOfIz_StringView DenOfIz_FSConfig_BundleResourcePath( );

#ifdef __cplusplus
}
#endif
