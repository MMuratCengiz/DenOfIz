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
