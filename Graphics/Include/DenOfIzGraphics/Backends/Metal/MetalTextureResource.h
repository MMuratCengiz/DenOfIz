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

#include <DenOfIzGraphics/Backends/Interface/ITextureResource.h>
#include "MetalContext.h"
#include "MetalEnumConverter.h"

namespace DenOfIz
{

    class MetalTextureResource final : public ITextureResource
    {
        Format        m_format       = Format::Undefined;
        uint32_t      m_width        = 1;
        uint32_t      m_height       = 1;
        uint32_t      m_depth        = 1;
        ResourceUsage m_initialState = ResourceUsage::Undefined;

        friend class MetalSwapChain;
        TextureDesc     m_desc;
        MetalContext   *m_context{ };
        id<MTLTexture>  m_texture{ };
        MTLTextureType  m_textureType;
        MTLTextureUsage m_textureUsage;

    private:
        void UpdateTexture( const TextureDesc &desc, id<MTLTexture> texture );

    public:
        MetalTextureResource( MetalContext *context, const TextureDesc &desc );
        MetalTextureResource( MetalContext *context, const TextureDesc &desc, id<MTLTexture> texture );

        const id<MTLTexture>  &Instance( ) const;
        const MTLTextureType  &Type( ) const;
        const MTLTextureUsage &Usage( ) const;

        [[nodiscard]] BitSet<ResourceUsage> InitialState( ) const override;
        [[nodiscard]] uint32_t              GetWidth( ) const;
        [[nodiscard]] uint32_t              GetHeight( ) const;
        [[nodiscard]] uint32_t              GetDepth( ) const;
        [[nodiscard]] Format                GetFormat( ) const override;

        ~MetalTextureResource( ) override;

        float MinLODClamp( );

    private:
        void SetTextureType( );
    };

    class MetalSampler final : public ISampler
    {
    private:
        MetalContext       *m_context;
        SamplerDesc         m_desc;
        id<MTLSamplerState> m_sampler{ };
        std::string         m_name;

    public:
        MetalSampler( MetalContext *context, const SamplerDesc &desc );
        const id<MTLSamplerState>       &Instance( ) const;
        const float                      LODBias( ) const;
        [[nodiscard]] const std::string &Name( ) const;
        ~MetalSampler( ) override;
    };
} // namespace DenOfIz
