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

#include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>
#include <DenOfIzGraphics/Backends/Interface/ITextureResource.h>

namespace DenOfIz
{
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
         MaterialData( ) = default;
        ~MaterialData( ) = default;

        void AttachSampler( std::unique_ptr<ISampler> sampler );
        void AttachAlbedoData( std::unique_ptr<ITextureResource> texture );
        void AttachNormalData( std::unique_ptr<ITextureResource> texture );
        void AttachHeightData( std::unique_ptr<ITextureResource> texture );
        void AttachMetallicData( std::unique_ptr<ITextureResource> texture );
        void AttachRoughnessData( std::unique_ptr<ITextureResource> texture );
        void AttachAoData( std::unique_ptr<ITextureResource> texture );

        [[nodiscard]] ISampler         *Sampler( ) const;
        [[nodiscard]] ITextureResource *AlbedoTexture( ) const;
        [[nodiscard]] ITextureResource *NormalTexture( ) const;
        [[nodiscard]] ITextureResource *HeightTexture( ) const;
        [[nodiscard]] ITextureResource *MetallicTexture( ) const;
        [[nodiscard]] ITextureResource *RoughnessTexture( ) const;
        [[nodiscard]] ITextureResource *AoTexture( ) const;
    };
} // namespace DenOfIz
