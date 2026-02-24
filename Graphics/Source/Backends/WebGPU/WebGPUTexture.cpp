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

#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUTexture.h"
#include "DenOfIzGraphicsInternal/Backends/WebGPU/WebGPUEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/StringUtilities.h"

using namespace DenOfIz;

WebGPUTexture::WebGPUTexture( WebGPUContext *context, const DenOfIz_TextureDesc &desc ) :
    m_context( context ), m_desc( desc ), m_debugName( StringViewToStdString( desc.DebugName ) )
{
    if ( !m_debugName.empty( ) )
    {
        m_desc.DebugName = DenOfIz_StringView( m_debugName.c_str( ), static_cast<uint32_t>( m_debugName.size( ) ) );
    }
    else
    {
        m_desc.DebugName = { };
    }

    uint32_t wgpuUsage = WGPUTextureUsage_CopyDst;

    if ( desc.Usage & DENOFIZ_TEXTURE_USAGE_COPY_SRC_BIT )
    {
        wgpuUsage |= WGPUTextureUsage_CopySrc;
    }
    if ( desc.Usage & DENOFIZ_TEXTURE_USAGE_COPY_DST_BIT )
    {
        wgpuUsage |= WGPUTextureUsage_CopyDst;
    }
    if ( desc.Usage & DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT )
    {
        wgpuUsage |= WGPUTextureUsage_TextureBinding;
    }
    if ( desc.Usage & DENOFIZ_TEXTURE_USAGE_STORAGE_BINDING_BIT )
    {
        wgpuUsage |= WGPUTextureUsage_StorageBinding;
    }
    if ( desc.Usage & DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT )
    {
        wgpuUsage |= WGPUTextureUsage_RenderAttachment;
    }
    if ( desc.HeapType == DENOFIZ_HEAP_TYPE_GPU_CPU )
    {
        wgpuUsage |= WGPUTextureUsage_CopySrc;
    }

    WGPUTextureDescriptor textureDesc   = { };
    textureDesc.size.width              = desc.Width;
    textureDesc.size.height             = desc.Height;
    textureDesc.size.depthOrArrayLayers = desc.Depth > 1 ? desc.Depth : desc.ArraySize;
    textureDesc.mipLevelCount           = desc.MipLevels;
    textureDesc.sampleCount             = DenOfIz_WebGPUEnumConverter_ConvertSampleCount( desc.MSAASampleCount );
    textureDesc.dimension               = DenOfIz_WebGPUEnumConverter_ConvertTextureDimension( desc.Width, desc.Height, desc.Depth );
    textureDesc.format                  = DenOfIz_WebGPUEnumConverter_ConvertFormat( desc.Format );
    textureDesc.usage                   = static_cast<WGPUTextureUsage>( wgpuUsage );
#if DZ_WEBGPU_USE_DAWN_API
    textureDesc.label                   = { m_debugName.empty( ) ? nullptr : m_debugName.c_str( ), m_debugName.size( ) };
#else
    textureDesc.label                   = m_debugName.empty( ) ? nullptr : m_debugName.c_str( );
#endif

    m_texture = wgpuDeviceCreateTexture( m_context->Device, &textureDesc );
    if ( !m_texture )
    {
        spdlog::critical( "WebGPU: Failed to create texture" );
        return;
    }

    WGPUTextureViewDescriptor viewDesc = { };
    viewDesc.format                    = textureDesc.format;
    viewDesc.dimension                 = DenOfIz_WebGPUEnumConverter_ConvertTextureViewDimension( desc.ArraySize, desc.Depth );
    viewDesc.baseMipLevel              = 0;
    viewDesc.mipLevelCount             = desc.MipLevels;
    viewDesc.baseArrayLayer            = 0;
    viewDesc.arrayLayerCount           = desc.ArraySize;
    viewDesc.aspect                    = DenOfIz_WebGPUEnumConverter_GetTextureAspectFromFormat( desc.Format );

    m_textureView = wgpuTextureCreateView( m_texture, &viewDesc );
    if ( !m_textureView )
    {
        spdlog::critical( "WebGPU: Failed to create texture view" );
    }
}

WebGPUTexture::WebGPUTexture( WebGPUContext *context, const DenOfIz_TextureDesc &desc, const WGPUTexture externalTexture ) :
    m_context( context ), m_desc( desc ), m_debugName( StringViewToStdString( desc.DebugName ) ), m_texture( externalTexture ), m_ownsTexture( false )
{
    if ( !m_debugName.empty( ) )
    {
        m_desc.DebugName = DenOfIz_StringView( m_debugName.c_str( ), static_cast<uint32_t>( m_debugName.size( ) ) );
    }
    else
    {
        m_desc.DebugName = { };
    }

    if ( !m_texture )
    {
        spdlog::critical( "WebGPU: External texture is null" );
        return;
    }

    WGPUTextureViewDescriptor viewDesc = { };
    viewDesc.format                    = DenOfIz_WebGPUEnumConverter_ConvertFormat( desc.Format );
    viewDesc.dimension                 = DenOfIz_WebGPUEnumConverter_ConvertTextureViewDimension( desc.ArraySize, desc.Depth );
    viewDesc.baseMipLevel              = 0;
    viewDesc.mipLevelCount             = desc.MipLevels;
    viewDesc.baseArrayLayer            = 0;
    viewDesc.arrayLayerCount           = desc.ArraySize;
    viewDesc.aspect                    = DenOfIz_WebGPUEnumConverter_GetTextureAspectFromFormat( desc.Format );

    m_textureView = wgpuTextureCreateView( m_texture, &viewDesc );
    if ( !m_textureView )
    {
        spdlog::critical( "WebGPU: Failed to create texture view for external texture" );
    }
}

WebGPUTexture::~WebGPUTexture( )
{
#if DZ_WEBGPU_USE_DAWN_API
    if ( m_textureView )
    {
        wgpuTextureViewRelease( m_textureView );
    }
#else
    if ( m_textureView && m_ownsTexture )
    {
        wgpuTextureViewRelease( m_textureView );
    }
#endif
    if ( m_texture && m_ownsTexture )
    {
        wgpuTextureRelease( m_texture );
    }
}

DenOfIz_Format WebGPUTexture::GetFormat( ) const
{
    return m_desc.Format;
}

WGPUTexture WebGPUTexture::GetTexture( ) const
{
    return m_texture;
}

WGPUTextureView WebGPUTexture::GetTextureView( ) const
{
    return m_textureView;
}

void WebGPUTexture::SetCurrentState( const uint32_t state )
{
    m_currentState = state;
}

void WebGPUTexture::UpdateExternalTexture( const WGPUTexture texture )
{
    if ( m_ownsTexture )
    {
        spdlog::error( "WebGPU: Cannot update external texture on owned texture resource" );
        return;
    }

    if ( m_textureView )
    {
        wgpuTextureViewRelease( m_textureView );
        m_textureView = nullptr;
    }

    m_texture = texture;

    if ( m_texture )
    {
        WGPUTextureViewDescriptor viewDesc = { };
        viewDesc.format                    = DenOfIz_WebGPUEnumConverter_ConvertFormat( m_desc.Format );
        viewDesc.dimension                 = DenOfIz_WebGPUEnumConverter_ConvertTextureViewDimension( m_desc.ArraySize, m_desc.Depth );
        viewDesc.baseMipLevel              = 0;
        viewDesc.mipLevelCount             = m_desc.MipLevels;
        viewDesc.baseArrayLayer            = 0;
        viewDesc.arrayLayerCount           = m_desc.ArraySize;
        viewDesc.aspect                    = DenOfIz_WebGPUEnumConverter_GetTextureAspectFromFormat( m_desc.Format );

        m_textureView = wgpuTextureCreateView( m_texture, &viewDesc );
        if ( !m_textureView )
        {
            spdlog::error( "WebGPU: Failed to create texture view for updated external texture" );
        }
    }
}

#if !DZ_WEBGPU_USE_DAWN_API
WebGPUTexture::WebGPUTexture( WebGPUContext *context, const DenOfIz_TextureDesc &desc, const WGPUTextureView externalTextureView ) :
    m_context( context ), m_desc( desc ), m_debugName( StringViewToStdString( desc.DebugName ) ), m_texture( nullptr ), m_textureView( externalTextureView ), m_ownsTexture( false )
{
    if ( !m_debugName.empty( ) )
    {
        m_desc.DebugName = DenOfIz_StringView( m_debugName.c_str( ), static_cast<uint32_t>( m_debugName.size( ) ) );
    }
    else
    {
        m_desc.DebugName = { };
    }

    if ( !m_textureView )
    {
        spdlog::critical( "WebGPU: External texture view is null" );
    }
}

void WebGPUTexture::UpdateExternalTextureView( const WGPUTextureView textureView )
{
    if ( m_ownsTexture )
    {
        spdlog::error( "WebGPU: Cannot update external texture view on owned texture resource" );
        return;
    }

    m_textureView = textureView;
}
#endif

WebGPUSampler::WebGPUSampler( WebGPUContext *context, const DenOfIz_SamplerDesc &desc ) :
    m_context( context ), m_desc( desc ), m_debugName( StringViewToStdString( desc.DebugName ) )
{
    WGPUSamplerDescriptor samplerDesc{ };
#if DZ_WEBGPU_USE_DAWN_API
    samplerDesc.label         = { m_debugName.empty( ) ? nullptr : m_debugName.c_str( ), m_debugName.size( ) };
#else
    samplerDesc.label         = m_debugName.empty( ) ? nullptr : m_debugName.c_str( );
#endif
    samplerDesc.addressModeU  = DenOfIz_WebGPUEnumConverter_ConvertSamplerAddressMode( desc.AddressModeU );
    samplerDesc.addressModeV  = DenOfIz_WebGPUEnumConverter_ConvertSamplerAddressMode( desc.AddressModeV );
    samplerDesc.addressModeW  = DenOfIz_WebGPUEnumConverter_ConvertSamplerAddressMode( desc.AddressModeW );
    samplerDesc.magFilter     = DenOfIz_WebGPUEnumConverter_ConvertFilter( desc.MagFilter );
    samplerDesc.minFilter     = DenOfIz_WebGPUEnumConverter_ConvertFilter( desc.MinFilter );
    samplerDesc.mipmapFilter  = DenOfIz_WebGPUEnumConverter_ConvertMipmapMode( desc.MipmapMode );
    samplerDesc.lodMinClamp   = desc.MinLod;
    samplerDesc.lodMaxClamp   = desc.MaxLod;
    const bool enableCompare  = desc.CompareOp != DENOFIZ_COMPARE_OP_NEVER && desc.CompareOp != DENOFIZ_COMPARE_OP_ALWAYS;
    samplerDesc.compare       = enableCompare ? DenOfIz_WebGPUEnumConverter_ConvertCompareOp( desc.CompareOp ) : WGPUCompareFunction_Undefined;
    samplerDesc.maxAnisotropy = desc.MaxAnisotropy > 0 ? static_cast<uint16_t>( desc.MaxAnisotropy ) : 1;

    m_sampler = wgpuDeviceCreateSampler( m_context->Device, &samplerDesc );
    if ( !m_sampler )
    {
        spdlog::error( "WebGPU: Failed to create sampler" );
    }
}

WebGPUSampler::~WebGPUSampler( )
{
    if ( m_sampler )
    {
        wgpuSamplerRelease( m_sampler );
    }
}

WGPUSampler WebGPUSampler::GetSampler( ) const
{
    return m_sampler;
}
