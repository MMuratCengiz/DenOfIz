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
#include "DenOfIzGraphicsInternal/Backends/Interface/ITexture.h"
#include "MetalContext.h"
#include "MetalEnumConverter.h"

namespace DenOfIz
{

    class MetalTexture final : public ITexture
    {
        DenOfIz_Format m_format = DENOFIZ_FORMAT_UNDEFINED;
        uint32_t       m_width  = 1;
        uint32_t       m_height = 1;
        uint32_t       m_depth  = 1;

        friend class MetalSwapChain;
        DenOfIz_TextureDesc m_desc;
        std::string         m_debugName;
        MetalContext       *m_context{ };
        id<MTLTexture>      m_texture{ };
        MTLTextureType      m_textureType;
        MTLTextureUsage     m_textureUsage;

    private:
        void UpdateTexture( const DenOfIz_TextureDesc &desc, id<MTLTexture> texture );

    public:
        MetalTexture( MetalContext *context, const DenOfIz_TextureDesc &desc );
        MetalTexture( MetalContext *context, const DenOfIz_TextureDesc &desc, id<MTLTexture> texture );

        const id<MTLTexture>  &Instance( ) const;
        const MTLTextureType  &Type( ) const;
        const MTLTextureUsage &Usage( ) const;

        [[nodiscard]] uint32_t       GetWidth( ) const;
        [[nodiscard]] uint32_t       GetHeight( ) const;
        [[nodiscard]] uint32_t       GetDepth( ) const;
        [[nodiscard]] DenOfIz_Format GetFormat( ) const override;

        ~MetalTexture( ) override;

        float MinLODClamp( );

    private:
        void SetTextureType( );
    };

    class MetalSampler final : public ISampler
    {
    private:
        MetalContext       *m_context;
        DenOfIz_SamplerDesc m_desc;
        id<MTLSamplerState> m_sampler{ };
        std::string         m_debugName;

    public:
        MetalSampler( MetalContext *context, const DenOfIz_SamplerDesc &desc );
        const id<MTLSamplerState>       &Instance( ) const;
        const float                      LODBias( ) const;
        [[nodiscard]] const std::string &Name( ) const;
        ~MetalSampler( ) override;
    };
} // namespace DenOfIz
