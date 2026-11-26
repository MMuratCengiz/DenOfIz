/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAsset.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DenOfIz_TextureAssetWriterDesc
    {
        DenOfIz_BinaryWriter Writer;
    } DenOfIz_TextureAssetWriterDesc;

    DENOFIZ_DEFINE_HANDLE( DenOfIz_TextureAssetWriter )

    DZ_API DenOfIz_TextureAssetWriter DenOfIz_TextureAssetWriter_Create( const DenOfIz_TextureAssetWriterDesc *desc );
    DZ_API void                       DenOfIz_TextureAssetWriter_Destroy( DenOfIz_TextureAssetWriter writer );

    DZ_API void DenOfIz_TextureAssetWriter_Write( DenOfIz_TextureAssetWriter writer, DenOfIz_TextureAsset textureAsset );
    DZ_API void DenOfIz_TextureAssetWriter_AddPixelData( DenOfIz_TextureAssetWriter writer, const DenOfIz_ByteArrayView *bytes, uint32_t mipIndex, uint32_t arrayLayer );
    DZ_API void DenOfIz_TextureAssetWriter_End( DenOfIz_TextureAssetWriter writer );

#ifdef __cplusplus
}
#endif
