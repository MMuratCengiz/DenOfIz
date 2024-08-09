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
        friend class MetalSwapChain;
        TextureDesc    m_desc;
        MetalContext  *m_context{ };
        id<MTLTexture> m_texture{ };
        bool           isExternalResource = false; // Used for swap chain render targets, might need a better way
    private:
        void UpdateTexture( id<MTLTexture> texture );

    public:
        MetalTextureResource( MetalContext *context, const TextureDesc &desc, std::string name );
        MetalTextureResource( MetalContext *context, const TextureDesc &desc, id<MTLTexture> texture, std::string name );
        const id<MTLTexture> &Instance( ) const
        {
            return m_texture;
        }
        ~MetalTextureResource( ) override;
    };

    class MetalSampler final : public ISampler
    {
    private:
        MetalContext       *m_context;
        SamplerDesc         m_desc;
        id<MTLSamplerState> m_sampler{ };

    public:
        MetalSampler( MetalContext *context, const SamplerDesc &desc, std::string name );
        ~MetalSampler( ) override;
    };
} // namespace DenOfIz
