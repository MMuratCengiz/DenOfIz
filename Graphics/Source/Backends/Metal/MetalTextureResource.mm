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

#include "DenOfIzGraphicsInternal/Backends/Metal/MetalTextureResource.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

MetalTextureResource::MetalTextureResource( MetalContext *context, const TextureDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_width        = desc.Width;
    m_height       = desc.Height;
    m_depth        = desc.Depth;
    m_format       = desc.Format;
    m_initialState = desc.InitialUsage;

    ValidateTextureDesc( m_desc );
    SetTextureType( );

    MTLTextureDescriptor *textureDesc = [[MTLTextureDescriptor alloc] init];

    textureDesc.width            = std::max( 1u, m_desc.Width );
    textureDesc.height           = std::max( 1u, m_desc.Height );
    textureDesc.depth            = std::max( 1u, m_desc.Depth );
    textureDesc.arrayLength      = std::max( 1u, m_desc.ArraySize );
    textureDesc.mipmapLevelCount = m_desc.MipLevels;
    textureDesc.sampleCount      = MSAASampleCountToNumSamples( m_desc.MSAASampleCount );
    textureDesc.pixelFormat      = MetalEnumConverter::ConvertFormat( m_desc.Format );
    textureDesc.textureType      = m_textureType;

    // TODO validate:
    switch ( m_desc.HeapType )
    {
    case HeapType::GPU:
        textureDesc.resourceOptions = MTLResourceStorageModePrivate;
        textureDesc.storageMode     = MTLStorageModePrivate;
        break;
    case HeapType::CPU:
        textureDesc.resourceOptions = MTLResourceStorageModeShared;
        textureDesc.storageMode     = MTLStorageModeShared;
        break;
    case HeapType::GPU_CPU:
    case HeapType::CPU_GPU:
        textureDesc.resourceOptions = MTLResourceStorageModeManaged;
        textureDesc.storageMode     = MTLStorageModeManaged;
        break;
    }

    if ( m_desc.Descriptor & ResourceDescriptor::RWTexture )
    {
        textureDesc.usage |= MTLTextureUsageShaderWrite;
    }

    if ( m_desc.Descriptor & ResourceDescriptor::DepthStencil )
    {
        textureDesc.usage |= MTLTextureUsageShaderRead;
    }
    if ( m_desc.Descriptor & ResourceDescriptor::RenderTarget || m_desc.Descriptor & ResourceDescriptor::DepthStencil )
    {
        textureDesc.usage |= MTLTextureUsageRenderTarget;
    }

    // Create the texture resource
    m_texture       = [m_context->Device newTextureWithDescriptor:textureDesc];
    m_textureUsage  = textureDesc.usage;
    m_texture.label = [NSString stringWithUTF8String:desc.DebugName.Get( )];

    if ( !m_texture )
    {
        spdlog::error("Failed to create Metal texture resource: {}", m_desc.DebugName.Get( ));
    }
}

MetalTextureResource::MetalTextureResource( MetalContext *context, const TextureDesc &desc, id<MTLTexture> texture ) : m_context( context ), m_texture( texture )
{
    m_context = context;
    m_desc    = desc;
    m_texture = texture;
}

void MetalTextureResource::UpdateTexture( const TextureDesc &desc, id<MTLTexture> texture )
{
    m_texture = texture;
    m_desc    = desc;
    SetTextureType( );
}

void MetalTextureResource::SetTextureType( )
{
    // When using Metal Shader Converter + HLSL, the conversion is always an array.
    bool isArray   = true; // m_desc.ArraySize > 1;
    bool isTexture = m_desc.Descriptor & ( ResourceDescriptor::Texture | ResourceDescriptor::RWTexture | ResourceDescriptor::RenderTarget | ResourceDescriptor::DepthStencil );
    bool isTextureCube = m_desc.Descriptor == ResourceDescriptor::TextureCube;
    bool hasDepth      = m_desc.Depth > 1;
    bool hasHeight     = m_desc.Height > 1;

    if ( isTexture && !isArray )
    {
        if ( hasDepth )
        {
            m_textureType = MTLTextureType3D;
        }
        else if ( hasHeight )
        {
            m_textureType = MTLTextureType2D;
        }
        else
        {
            m_textureType = MTLTextureType1D;
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
        else if ( hasHeight )
        {
            m_textureType = MTLTextureType2DArray;
        }
        else
        {
            m_textureType = MTLTextureType1DArray;
        }
    }
}

float MetalTextureResource::MinLODClamp( )
{
    return 0;
}

MetalTextureResource::~MetalTextureResource( )
{
}
const id<MTLTexture> &MetalTextureResource::Instance( ) const
{
    return m_texture;
}
const MTLTextureType &MetalTextureResource::Type( ) const
{
    return m_textureType;
}
const MTLTextureUsage &MetalTextureResource::Usage( ) const
{
    return m_textureUsage;
}

uint32_t MetalTextureResource::InitialState( ) const
{
    return m_initialState;
}

uint32_t MetalTextureResource::GetWidth( ) const
{
    return m_width;
}

uint32_t MetalTextureResource::GetHeight( ) const
{
    return m_height;
}

uint32_t MetalTextureResource::GetDepth( ) const
{
    return m_depth;
}

Format MetalTextureResource::GetFormat( ) const
{
    return m_format;
}

MetalSampler::MetalSampler( MetalContext *context, const SamplerDesc &desc ) : m_context( context ), m_desc( desc )
{
    MTLSamplerDescriptor *samplerDesc = [[MTLSamplerDescriptor alloc] init];

    samplerDesc.supportArgumentBuffers = YES;
    samplerDesc.normalizedCoordinates  = YES;
    samplerDesc.minFilter              = MetalEnumConverter::ConvertFilter( desc.MinFilter );
    samplerDesc.magFilter              = MetalEnumConverter::ConvertFilter( desc.MagFilter );
    samplerDesc.mipFilter              = MetalEnumConverter::ConvertMipMapFilter( desc.MipmapMode );
    samplerDesc.sAddressMode           = MetalEnumConverter::ConvertSamplerAddressMode( desc.AddressModeU );
    samplerDesc.tAddressMode           = MetalEnumConverter::ConvertSamplerAddressMode( desc.AddressModeV );
    samplerDesc.rAddressMode           = MetalEnumConverter::ConvertSamplerAddressMode( desc.AddressModeW );
    samplerDesc.lodMinClamp            = desc.MinLod;
    samplerDesc.lodMaxClamp            = desc.MaxLod;
    samplerDesc.compareFunction        = MetalEnumConverter::ConvertCompareFunction( desc.CompareOp );
    samplerDesc.maxAnisotropy          = std::max( 1.0f, desc.MaxAnisotropy );
    samplerDesc.label                  = [NSString stringWithUTF8String:desc.DebugName.Get( )];
    m_sampler                          = [m_context->Device newSamplerStateWithDescriptor:samplerDesc];
    if ( !m_sampler )
    {
        spdlog::error("Failed to create Metal sampler state: {}", desc.DebugName.Get( ));
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
    return m_name;
}
