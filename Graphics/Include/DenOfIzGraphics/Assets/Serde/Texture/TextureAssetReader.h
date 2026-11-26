/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
