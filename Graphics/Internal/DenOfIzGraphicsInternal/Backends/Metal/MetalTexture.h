/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
        MTLTextureType      m_textureType = MTLTextureType2D;
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
        [[nodiscard]] uint32_t       GetMipLevels( ) const override;
        [[nodiscard]] const DenOfIz_TextureDesc &GetDesc( ) const override
        {
            return m_desc;
        }

        ~MetalTexture( ) override;

        float MinLODClamp( );
        id<MTLTexture> MipView( uint32_t mipLevel );

    private:
        std::vector<id<MTLTexture>> m_mipViews;
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
