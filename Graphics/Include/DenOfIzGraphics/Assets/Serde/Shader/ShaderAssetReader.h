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

#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAsset.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryReader.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_ShaderAssetReader )

    typedef struct DenOfIz_ShaderAssetReaderDesc
    {
        DenOfIz_BinaryReader Reader;
    } DenOfIz_ShaderAssetReaderDesc;

    DZ_API DenOfIz_ShaderAssetReader DenOfIz_ShaderAssetReader_Create( const DenOfIz_ShaderAssetReaderDesc *desc );
    DZ_API void                      DenOfIz_ShaderAssetReader_Destroy( DenOfIz_ShaderAssetReader reader );
    DZ_API DenOfIz_ShaderAsset       DenOfIz_ShaderAssetReader_Read( DenOfIz_ShaderAssetReader reader );

#ifdef __cplusplus
}
#endif
