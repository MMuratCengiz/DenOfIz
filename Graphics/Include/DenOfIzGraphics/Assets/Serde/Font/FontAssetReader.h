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

#include "DenOfIzGraphics/Assets/Serde/Font/FontAsset.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryReader.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_FontAssetReader )

    typedef struct DenOfIz_FontAssetReaderDesc
    {
        DenOfIz_BinaryReader Reader;
    } DenOfIz_FontAssetReaderDesc;

    typedef struct DenOfIz_LoadAtlasIntoGpuTextureDesc
    {
        DenOfIz_LogicalDevice Device;
        DenOfIz_CommandList   CommandList;
        DenOfIz_Buffer        StagingBuffer;
        DenOfIz_Texture       Texture;
    } DenOfIz_LoadAtlasIntoGpuTextureDesc;

    DZ_API DenOfIz_FontAssetReader DenOfIz_FontAssetReader_Create( const DenOfIz_FontAssetReaderDesc *desc );
    DZ_API void                    DenOfIz_FontAssetReader_Destroy( DenOfIz_FontAssetReader reader );
    DZ_API DenOfIz_FontAsset       DenOfIz_FontAssetReader_Read( DenOfIz_FontAssetReader reader );
    DZ_API void                    DenOfIz_FontAssetReader_LoadAtlasIntoGpuTexture( DenOfIz_FontAsset fontAsset, const DenOfIz_LoadAtlasIntoGpuTextureDesc *desc );

#ifdef __cplusplus
}
#endif
