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
#include <webgpu/webgpu.h>
#include "DenOfIzGraphicsInternal/Backends/Interface/ITexture.h"
#include "WebGPUContext.h"

namespace DenOfIz
{
    class WebGPUTexture final : public ITexture
    {
        WebGPUContext      *m_context;
        DenOfIz_TextureDesc m_desc;
        std::string         m_debugName;
        WGPUTexture         m_texture      = nullptr;
        WGPUTextureView     m_textureView  = nullptr;
        uint32_t            m_currentState = DENOFIZ_RESOURCE_USAGE_COMMON_BIT;
        bool                m_ownsTexture  = true;
        std::vector<WGPUTextureView> m_mipViews;

    public:
        WebGPUTexture( WebGPUContext *context, const DenOfIz_TextureDesc &desc );
        WebGPUTexture( WebGPUContext *context, const DenOfIz_TextureDesc &desc, WGPUTexture externalTexture );
#if !DZ_WEBGPU_USE_DAWN_API
        WebGPUTexture( WebGPUContext *context, const DenOfIz_TextureDesc &desc, WGPUTextureView externalTextureView );
#endif
        ~WebGPUTexture( ) override;

        [[nodiscard]] DenOfIz_Format GetFormat( ) const override;
        [[nodiscard]] uint32_t       GetMipLevels( ) const override;

        [[nodiscard]] WGPUTexture                GetTexture( ) const;
        [[nodiscard]] WGPUTextureView            GetTextureView( ) const;
        [[nodiscard]] WGPUTextureView            GetMipTextureView( uint32_t mipLevel );
        [[nodiscard]] const DenOfIz_TextureDesc &GetDesc( ) const
        {
            return m_desc;
        }
        void SetCurrentState( uint32_t state );
        void UpdateExternalTexture( WGPUTexture texture );
#if !DZ_WEBGPU_USE_DAWN_API
        void UpdateExternalTextureView( WGPUTextureView textureView );
#endif
    };

    class WebGPUSampler final : public ISampler
    {
        WebGPUContext      *m_context;
        DenOfIz_SamplerDesc m_desc;
        std::string         m_debugName;
        WGPUSampler         m_sampler = nullptr;

    public:
        WebGPUSampler( WebGPUContext *context, const DenOfIz_SamplerDesc &desc );
        ~WebGPUSampler( ) override;

        [[nodiscard]] WGPUSampler GetSampler( ) const;
    };

} // namespace DenOfIz
