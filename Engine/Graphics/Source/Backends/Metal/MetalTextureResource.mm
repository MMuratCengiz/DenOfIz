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

#include <DenOfIzGraphics/Backends/Metal/MetalTextureResource.h>

using namespace DenOfIz;

MetalTextureResource::MetalTextureResource( MetalContext *context, const TextureDesc &desc, std::string name ) : ITextureResource( desc ), m_context( context )
{
    m_context = context;
    m_desc    = desc;
    ValidateTextureDesc( m_desc );

    MTLTextureDescriptor *textureDesc = [[MTLTextureDescriptor alloc] init];

    textureDesc.width            = std::max( 1u, m_desc.Width );
    textureDesc.height           = std::max( 1u, m_desc.Height );
    textureDesc.depth            = std::max( 1u, m_desc.Depth );
    textureDesc.arrayLength      = std::max( 1u, m_desc.ArraySize );
    textureDesc.mipmapLevelCount = m_desc.MipLevels;
    textureDesc.sampleCount      = MSAASampleCountToNumSamples( m_desc.MSAASampleCount );
    textureDesc.pixelFormat      = MetalEnumConverter::ConvertFormat( m_desc.Format );

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

    if ( m_desc.Descriptor.IsSet( ResourceDescriptor::RWTexture ) )
    {
        textureDesc.usage |= MTLTextureUsageShaderWrite;
    }

    if ( m_desc.InitialState.Any( { ResourceState::RenderTarget, ResourceState::DepthWrite } ) )
    {
        textureDesc.usage |= MTLTextureUsageRenderTarget;
    }

    // Create the texture resource
    m_texture = [m_context->Device newTextureWithDescriptor:textureDesc];

    if ( !m_texture )
    {
        LOG( ERROR ) << "Failed to create Metal texture resource: " << m_name;
    }
}

MetalTextureResource::MetalTextureResource( MetalContext *context, const TextureDesc &desc, id<MTLTexture> texture, std::string name ) :
    ITextureResource( m_desc ), m_context( context ), m_texture( texture )
{
    m_context          = context;
    m_desc             = desc;
    m_texture          = texture;
    m_name             = name;
    isExternalResource = true;
}

void MetalTextureResource::UpdateTexture( const TextureDesc &desc, id<MTLTexture> texture )
{
    m_texture = texture;
    m_desc    = desc;
}

MetalTextureResource::~MetalTextureResource( )
{
}

MetalSampler::MetalSampler( MetalContext *context, const SamplerDesc &desc, std::string name ) : m_context( context ), m_desc( desc )
{
    m_name                            = name;
    MTLSamplerDescriptor *samplerDesc = [[MTLSamplerDescriptor alloc] init];

    samplerDesc.minFilter       = MetalEnumConverter::ConvertFilter( desc.MinFilter );
    samplerDesc.magFilter       = MetalEnumConverter::ConvertFilter( desc.MagFilter );
    samplerDesc.mipFilter       = MetalEnumConverter::ConvertMipMapFilter( desc.MipmapMode );
    samplerDesc.sAddressMode    = MetalEnumConverter::ConvertSamplerAddressMode( desc.AddressModeU );
    samplerDesc.tAddressMode    = MetalEnumConverter::ConvertSamplerAddressMode( desc.AddressModeV );
    samplerDesc.rAddressMode    = MetalEnumConverter::ConvertSamplerAddressMode( desc.AddressModeW );
    samplerDesc.lodMinClamp     = desc.MinLod;
    samplerDesc.lodMaxClamp     = desc.MaxLod;
    samplerDesc.compareFunction = MetalEnumConverter::ConvertCompareFunction( desc.CompareOp );
    samplerDesc.maxAnisotropy   = desc.MaxAnisotropy;

    m_sampler = [m_context->Device newSamplerStateWithDescriptor:samplerDesc];
    if ( !m_sampler )
    {
        LOG( ERROR ) << "Failed to create Metal sampler state: " << name;
    }
}

MetalSampler::~MetalSampler( )
{
}

const id<MTLSamplerState> &MetalSampler::Instance( ) const
{
    return m_sampler;
}
