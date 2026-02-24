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

#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAsset.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryReader.h"
#include "DenOfIzGraphics/Backends/Interface/Buffer.h"
#include "DenOfIzGraphics/Backends/Interface/CommandList.h"
#include "DenOfIzGraphics/Backends/Interface/Texture.h"
#include "DenOfIzGraphics/Handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DenOfIz_TextureAssetReaderDesc
    {
        DenOfIz_BinaryReader Reader;
    } DenOfIz_TextureAssetReaderDesc;

    typedef struct DenOfIz_LoadIntoGpuTextureDesc
    {
        DenOfIz_CommandList CommandList;
        DenOfIz_Buffer      StagingBuffer;
        DenOfIz_Texture     Texture;
    } DenOfIz_LoadIntoGpuTextureDesc;

    DENOFIZ_DEFINE_HANDLE( DenOfIz_TextureAssetReader )

    DZ_API DenOfIz_TextureAssetReader DenOfIz_TextureAssetReader_Create( const DenOfIz_TextureAssetReaderDesc *desc );
    DZ_API void                       DenOfIz_TextureAssetReader_Destroy( DenOfIz_TextureAssetReader reader );

    DZ_API DenOfIz_TextureAsset DenOfIz_TextureAssetReader_Read( DenOfIz_TextureAssetReader reader );
    DZ_API void                 DenOfIz_TextureAssetReader_LoadIntoGpuTexture( DenOfIz_TextureAssetReader reader, const DenOfIz_LoadIntoGpuTextureDesc *desc );
    DZ_API DenOfIz_ByteArray    DenOfIz_TextureAssetReader_ReadRaw( DenOfIz_TextureAssetReader reader, uint32_t mipLevel, uint32_t arrayLayer );
    DZ_API uint64_t             DenOfIz_TextureAssetReader_AlignedTotalNumBytes( DenOfIz_TextureAssetReader reader, const DenOfIz_DeviceConstants *constants );

#ifdef __cplusplus
}
#endif
