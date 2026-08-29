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

#include "DenOfIzGraphicsInternal/Backends/Metal/MetalTexture.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/StringUtilities.h"

using namespace DenOfIz;

MetalTexture::MetalTexture( MetalContext *context, const DenOfIz_TextureDesc &desc ) :
    m_context( context ),
    m_desc( desc ),
    m_debugName( StringViewToStdString( desc.DebugName ) )
{
    if ( !m_debugName.empty( ) )
    {
        m_desc.DebugName = DenOfIz_StringView( m_debugName.c_str( ), static_cast<uint32_t>( m_debugName.size( ) ) );
    }
    else
    {
        m_desc.DebugName = { };
    }

    m_width  = desc.Width;
    m_height = desc.Height;
    m_depth  = desc.Depth;
    m_format = desc.Format;

    SetTextureType( );

    MTLTextureDescriptor *textureDesc = [[MTLTextureDescriptor alloc] init];

    textureDesc.width            = std::max( 1u, m_desc.Width );
    textureDesc.height           = std::max( 1u, m_desc.Height );
    textureDesc.depth            = std::max( 1u, m_desc.Depth );
    textureDesc.arrayLength      = std::max( 1u, m_desc.ArraySize );
    textureDesc.mipmapLevelCount = m_desc.MipLevels;
    textureDesc.sampleCount      = DenOfIz_MSAASampleCount_ToNumSamples( m_desc.MSAASampleCount );
    textureDesc.pixelFormat      = DenOfIz_MetalEnumConverter_ConvertFormat( m_desc.Format );
    textureDesc.textureType      = m_textureType;

    // TODO validate:
    switch ( m_desc.HeapType )
    {
    case DENOFIZ_HEAP_TYPE_GPU:
        textureDesc.resourceOptions = MTLResourceStorageModePrivate;
        textureDesc.storageMode     = MTLStorageModePrivate;
        break;
    case DENOFIZ_HEAP_TYPE_CPU:
        textureDesc.resourceOptions = MTLResourceStorageModeShared;
        textureDesc.storageMode     = MTLStorageModeShared;
        break;
    case DENOFIZ_HEAP_TYPE_GPU_CPU:
    case DENOFIZ_HEAP_TYPE_CPU_GPU:
        textureDesc.resourceOptions = MTLResourceStorageModeManaged;
        textureDesc.storageMode     = MTLStorageModeManaged;
        break;
    }

    if ( m_desc.Usage & DENOFIZ_TEXTURE_USAGE_STORAGE_BINDING_BIT )
    {
        textureDesc.usage |= MTLTextureUsageShaderWrite;
    }

    if ( m_desc.Usage & DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT || m_desc.Usage & DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT )
    {
        textureDesc.usage |= MTLTextureUsageShaderRead;
    }
    if ( m_desc.Usage & DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT )
    {
        textureDesc.usage |= MTLTextureUsageRenderTarget;
    }

    // Create the texture resource
    m_texture       = [m_context->Device newTextureWithDescriptor:textureDesc];
    m_textureUsage  = textureDesc.usage;
    @autoreleasepool
    {
        m_texture.label = [NSString stringWithUTF8String:m_debugName.empty( ) ? "" : m_debugName.c_str( )];
    }

    if ( !m_texture )
    {
        spdlog::error("Failed to create Metal texture resource: {}", m_debugName );
    }
}

MetalTexture::MetalTexture( MetalContext *context, const DenOfIz_TextureDesc &desc, id<MTLTexture> texture ) :
    m_context( context ),
    m_desc( desc ),
    m_debugName( StringViewToStdString( desc.DebugName ) ),
    m_texture( texture )
{
    if ( !m_debugName.empty( ) )
    {
        m_desc.DebugName = DenOfIz_StringView( m_debugName.c_str( ), static_cast<uint32_t>( m_debugName.size( ) ) );
    }
    else
    {
        m_desc.DebugName = { };
    }
    
    m_width       = desc.Width;
    m_height      = desc.Height;
    m_depth       = desc.Depth;
    m_format      = desc.Format;
    m_textureType = MTLTextureType2D;
    m_textureUsage = MTLTextureUsageRenderTarget;
}

void MetalTexture::UpdateTexture( const DenOfIz_TextureDesc &desc, id<MTLTexture> texture )
{
    m_texture = texture;
    m_desc    = desc;
    m_debugName = StringViewToStdString( desc.DebugName );
    if ( !m_debugName.empty( ) )
    {
        m_desc.DebugName = DenOfIz_StringView( m_debugName.c_str( ), static_cast<uint32_t>( m_debugName.size( ) ) );
    }
    else
    {
        m_desc.DebugName = { };
    }

    m_width  = desc.Width;
    m_height = desc.Height;
    m_depth  = desc.Depth;
    m_format = desc.Format;

    SetTextureType( );
}

void MetalTexture::SetTextureType( )
{
    bool isArray       = m_desc.ArraySize > 1;
    bool isTexture     = m_desc.Usage & ( DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT | DENOFIZ_TEXTURE_USAGE_STORAGE_BINDING_BIT | DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT );
    bool isTextureCube = m_desc.Usage & DENOFIZ_TEXTURE_USAGE_CUBE_BIT;
    bool hasDepth      = m_desc.Depth > 1;
    bool is1D          = m_desc.Height == 0;

    if ( isTexture && !isArray )
    {
        if ( hasDepth )
        {
            m_textureType = MTLTextureType3D;
        }
        else if ( is1D )
        {
            m_textureType = MTLTextureType1D;
        }
        else
        {
            m_textureType = MTLTextureType2D;
        }
    }
    else if ( isTextureCube )
    {
        if ( isArray )
        {
            m_textureType = MTLTextureTypeCubeArray;
        }
        else
        {
            m_textureType = MTLTextureTypeCube;
        }
    }
    else if ( isArray )
    {
        if ( hasDepth )
        {
            spdlog::error("Array textures cannot have depth.");
        }
        else if ( is1D )
        {
            m_textureType = MTLTextureType1DArray;
        }
        else
        {
            m_textureType = MTLTextureType2DArray;
        }
    }
}

float MetalTexture::MinLODClamp( )
{
    return 0;
}

id<MTLTexture> MetalTexture::MipView( uint32_t mipLevel )
{
    if ( m_mipViews.empty( ) )
    {
        m_mipViews.resize( m_desc.MipLevels, nil );
    }

    if ( mipLevel >= m_desc.MipLevels )
    {
        return m_texture;
    }

    if ( m_mipViews[ mipLevel ] == nil )
    {
        m_mipViews[ mipLevel ] = [m_texture newTextureViewWithPixelFormat:m_texture.pixelFormat
                                                             textureType:m_texture.textureType
                                                                  levels:NSMakeRange( mipLevel, 1 )
                                                                  slices:NSMakeRange( 0, m_texture.arrayLength )];
    }

    return m_mipViews[ mipLevel ];
}

MetalTexture::~MetalTexture( )
{
    for ( id<MTLTexture> mipView : m_mipViews )
    {
        mipView = nil;
    }
    m_mipViews.clear( );
}
const id<MTLTexture> &MetalTexture::Instance( ) const
{
    return m_texture;
}
const MTLTextureType &MetalTexture::Type( ) const
{
    return m_textureType;
}
const MTLTextureUsage &MetalTexture::Usage( ) const
{
    return m_textureUsage;
}

uint32_t MetalTexture::GetWidth( ) const
{
    return m_width;
}

uint32_t MetalTexture::GetHeight( ) const
{
    return m_height;
}

uint32_t MetalTexture::GetDepth( ) const
{
    return m_depth;
}

DenOfIz_Format MetalTexture::GetFormat( ) const
{
    return m_format;
}

uint32_t MetalTexture::GetMipLevels( ) const
{
    return m_desc.MipLevels;
}

MetalSampler::MetalSampler( MetalContext *context, const DenOfIz_SamplerDesc &desc ) :
    m_context( context ),
    m_desc( desc ),
    m_debugName( StringViewToStdString( desc.DebugName ) )
{
    MTLSamplerDescriptor *samplerDesc = [[MTLSamplerDescriptor alloc] init];

    samplerDesc.supportArgumentBuffers = YES;
    samplerDesc.normalizedCoordinates  = YES;
    samplerDesc.minFilter              = DenOfIz_MetalEnumConverter_ConvertFilter( desc.MinFilter );
    samplerDesc.magFilter              = DenOfIz_MetalEnumConverter_ConvertFilter( desc.MagFilter );
    samplerDesc.mipFilter              = DenOfIz_MetalEnumConverter_ConvertMipMapFilter( desc.MipmapMode );
    samplerDesc.sAddressMode           = DenOfIz_MetalEnumConverter_ConvertSamplerAddressMode( desc.AddressModeU );
    samplerDesc.tAddressMode           = DenOfIz_MetalEnumConverter_ConvertSamplerAddressMode( desc.AddressModeV );
    samplerDesc.rAddressMode           = DenOfIz_MetalEnumConverter_ConvertSamplerAddressMode( desc.AddressModeW );
    samplerDesc.lodMinClamp            = desc.MinLod;
    samplerDesc.lodMaxClamp            = desc.MaxLod;
    samplerDesc.compareFunction        = DenOfIz_MetalEnumConverter_ConvertCompareFunction( desc.CompareOp );
    samplerDesc.maxAnisotropy          = std::max( 1.0f, desc.MaxAnisotropy );
    @autoreleasepool
    {
        samplerDesc.label = [NSString stringWithUTF8String:m_debugName.empty( ) ? "" : m_debugName.c_str( )];
        m_sampler         = [m_context->Device newSamplerStateWithDescriptor:samplerDesc];
    }
    if ( !m_sampler )
    {
        spdlog::error( "Failed to create Metal sampler state: {}", m_debugName );
    }
}

MetalSampler::~MetalSampler( )
{
}

const id<MTLSamplerState> &MetalSampler::Instance( ) const
{
    return m_sampler;
}

const float MetalSampler::LODBias( ) const
{
    return m_desc.MipLodBias;
}

const std::string &MetalSampler::Name( ) const
{
    return m_debugName;
}
