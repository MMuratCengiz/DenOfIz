/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
