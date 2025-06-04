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

#include "DenOfIzGraphics/Backends/Interface/IBufferResource.h"
#include "DenOfIzGraphics/Backends/Interface/ILogicalDevice.h"
#include "DenOfIzGraphics/Backends/Interface/ITextureResource.h"
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"
#include "DenOfIzExamples/Interop.h"

namespace DenOfIz
{
    struct MaterialDesc
    {
        ILogicalDevice    *Device;
        BatchResourceCopy *BatchCopy;
        InteropString      AlbedoTexture;
        InteropString      NormalTexture;
        InteropString      HeightTexture;
        InteropString      MetallicTexture;
        InteropString      RoughnessTexture;
        InteropString      AoTexture;
    };

    /// <summary> Material data class that holds the texture data for the material. </summary>
    class MaterialData
    {
        std::unique_ptr<ISampler>         m_sampler;
        std::unique_ptr<ITextureResource> m_albedoTexture;
        std::unique_ptr<ITextureResource> m_normalTexture;
        std::unique_ptr<ITextureResource> m_heightTexture;
        std::unique_ptr<ITextureResource> m_metallicTexture;
        std::unique_ptr<ITextureResource> m_roughnessTexture;
        std::unique_ptr<ITextureResource> m_aoTexture;

    public:
        MaterialData( const MaterialDesc &desc );
        ~MaterialData( ) = default;

        [[nodiscard]] ISampler         *Sampler( ) const;
        [[nodiscard]] ITextureResource *AlbedoTexture( ) const;
        [[nodiscard]] ITextureResource *NormalTexture( ) const;
        [[nodiscard]] ITextureResource *HeightTexture( ) const;
        [[nodiscard]] ITextureResource *MetallicTexture( ) const;
        [[nodiscard]] ITextureResource *RoughnessTexture( ) const;
        [[nodiscard]] ITextureResource *AoTexture( ) const;
    };
} // namespace DenOfIz
