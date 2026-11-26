/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <string>

#include "../Interop.h"
#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Backends/Interface/Texture.h"
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"

namespace DenOfIz
{
    struct MaterialDesc
    {
        DenOfIz_LogicalDevice     Device;
        DenOfIz_BatchResourceCopy BatchCopy;
        std::string               AlbedoTexture;
        std::string               NormalTexture;
        std::string               HeightTexture;
        std::string               MetallicTexture;
        std::string               RoughnessTexture;
        std::string               AoTexture;
    };

    class MaterialData
    {
        DenOfIz_Sampler m_sampler          = DENOFIZ_NULL_HANDLE;
        DenOfIz_Texture m_albedoTexture    = DENOFIZ_NULL_HANDLE;
        DenOfIz_Texture m_normalTexture    = DENOFIZ_NULL_HANDLE;
        DenOfIz_Texture m_heightTexture    = DENOFIZ_NULL_HANDLE;
        DenOfIz_Texture m_metallicTexture  = DENOFIZ_NULL_HANDLE;
        DenOfIz_Texture m_roughnessTexture = DENOFIZ_NULL_HANDLE;
        DenOfIz_Texture m_aoTexture        = DENOFIZ_NULL_HANDLE;

    public:
        DZ_EXAMPLES_API MaterialData( const MaterialDesc &desc );
        DZ_EXAMPLES_API ~MaterialData( );

        [[nodiscard]] DZ_EXAMPLES_API DenOfIz_Sampler Sampler( ) const;
        [[nodiscard]] DZ_EXAMPLES_API DenOfIz_Texture AlbedoTexture( ) const;
        [[nodiscard]] DZ_EXAMPLES_API DenOfIz_Texture NormalTexture( ) const;
        [[nodiscard]] DZ_EXAMPLES_API DenOfIz_Texture HeightTexture( ) const;
        [[nodiscard]] DZ_EXAMPLES_API DenOfIz_Texture MetallicTexture( ) const;
        [[nodiscard]] DZ_EXAMPLES_API DenOfIz_Texture RoughnessTexture( ) const;
        [[nodiscard]] DZ_EXAMPLES_API DenOfIz_Texture AoTexture( ) const;
    };
} // namespace DenOfIz
