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

MetalTextureResource::MetalTextureResource( MetalContext *context, const TextureDesc &desc ) : ITextureResource( desc ), m_context( context )
{
    m_context = context;
    m_desc    = desc;
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

    if ( m_desc.Descriptor.IsSet( ResourceDescriptor::RWTexture ) )
    {
        textureDesc.usage |= MTLTextureUsageShaderWrite;
    }

    if ( m_desc.InitialState.Any( { ResourceState::RenderTarget, ResourceState::DepthWrite } ) )
    {
        textureDesc.usage |= MTLTextureUsageRenderTarget;
    }

    // Create the texture resource
    m_texture      = [m_context->Device newTextureWithDescriptor:textureDesc];
    m_textureUsage = textureDesc.usage;

    if ( !m_texture )
    {
        LOG( ERROR ) << "Failed to create Metal texture resource: " << m_desc.DebugName;
    }
}

MetalTextureResource::MetalTextureResource( MetalContext *context, const TextureDesc &desc, id<MTLTexture> texture ) :
    ITextureResource( m_desc ), m_context( context ), m_texture( texture )
{
    m_context          = context;
    m_desc             = desc;
    m_texture          = texture;
    isExternalResource = true;
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
    bool isArray       = true; //m_desc.ArraySize > 1;
    bool isTexture     = m_desc.Descriptor == ResourceDescriptor::Texture;
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
    else if ( isTexture && isArray )
    {
        if ( hasDepth )
        {
            LOG( ERROR ) << "Array textures cannot have depth.";
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
}

float MetalTextureResource::MinLODClamp( )
{
    return 0;
}

MetalTextureResource::~MetalTextureResource( )
{
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
    samplerDesc.label                  = [NSString stringWithUTF8String:desc.DebugName.c_str( )];

    m_sampler = [m_context->Device newSamplerStateWithDescriptor:samplerDesc];
    if ( !m_sampler )
    {
        LOG( ERROR ) << "Failed to create Metal sampler state: " << desc.DebugName;
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
