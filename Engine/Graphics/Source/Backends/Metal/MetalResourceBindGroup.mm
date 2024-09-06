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

#include <DenOfIzGraphics/Backends/Metal/MetalResourceBindGroup.h>

using namespace DenOfIz;

MetalResourceBindGroup::MetalResourceBindGroup( MetalContext *context, ResourceBindGroupDesc desc ) : IResourceBindGroup( desc ), m_context( context )
{
    m_context             = context;
    m_rootSignature       = static_cast<MetalRootSignature *>( desc.RootSignature );
    m_descriptorTable     = m_rootSignature->DescriptorTable( desc.RegisterSpace );
    m_argumentDescriptors = [[NSMutableArray alloc] init];
}

void MetalResourceBindGroup::Update( const UpdateDesc &desc )
{
    m_updateDesc = desc;
    IResourceBindGroup::Update( desc );
    //    std::memcpy( m_argumentBuffer.contents, m_descriptorTable.data( ), m_descriptorTable.size( ) * sizeof( uint64_t ) );

    m_argumentEncoder = [m_context->Device newArgumentEncoderWithArguments:m_argumentDescriptors];
    m_argumentBuffer  = [m_context->Device newBufferWithLength:m_argumentEncoder.encodedLength options:MTLResourceStorageModeShared];

    [m_argumentEncoder setArgumentBuffer:m_argumentBuffer offset:0];

    for ( const auto &buffer : desc.Buffers )
    {
        MetalBufferResource    *metalBuffer = static_cast<MetalBufferResource *>( buffer.Resource );
        const MetalBindingDesc &binding     = m_rootSignature->FindMetalBinding( buffer.Slot );
        [m_argumentEncoder setBuffer:metalBuffer->Instance( ) offset:0 atIndex:binding.Parent.Reflection.LocationHint];
    }

    for ( const auto &texture : desc.Textures )
    {
        MetalTextureResource   *metalTexture = static_cast<MetalTextureResource *>( texture.Resource );
        const MetalBindingDesc &binding      = m_rootSignature->FindMetalBinding( texture.Slot );
        [m_argumentEncoder setTexture:metalTexture->Instance( ) atIndex:binding.Parent.Reflection.LocationHint];
    }

    for ( const auto &sampler : desc.Samplers )
    {
        MetalSampler           *metalSampler = static_cast<MetalSampler *>( sampler.Resource );
        const MetalBindingDesc &binding      = m_rootSignature->FindMetalBinding( sampler.Slot );
        [m_argumentEncoder setSamplerState:metalSampler->Instance( ) atIndex:binding.Parent.Reflection.LocationHint];
    }
}

void MetalResourceBindGroup::BindTexture( const ResourceBindingSlot &slot, ITextureResource *resource )
{
    //    const MetalBindingDesc &binding      = m_rootSignature->FindMetalBinding( slot );
    MetalTextureResource *metalTexture = static_cast<MetalTextureResource *>( resource );

    //    bool                    readonlyHeap   = ( metalTexture->Usage( ) & MTLTextureUsageRenderTarget ) == 0 && ( metalTexture->Usage( ) & MTLTextureUsageShaderWrite ) == 0;
    //    id<MTLBuffer>           texturePointer = CreateEntryBuffer( readonlyHeap );
    //    IRDescriptorTableEntry *entry          = (IRDescriptorTableEntry *)texturePointer.contents;
    //    IRDescriptorTableSetTexture( entry, metalTexture->Instance( ), 0, 0 );
    //    SetGpuAddress( binding.Parent.LocationHint, texturePointer.gpuAddress );

    PushUpdateDesc( slot, metalTexture->Usage( ), metalTexture, m_textures );

    MTLArgumentDescriptor *descriptor = CreateArgumentDescriptor( slot );
}

void MetalResourceBindGroup::BindBuffer( const ResourceBindingSlot &slot, IBufferResource *resource )
{
    //    const MetalBindingDesc &binding     = m_rootSignature->FindMetalBinding( slot );
    MetalBufferResource *metalBuffer = static_cast<MetalBufferResource *>( resource );

    //    id<MTLBuffer>           bufferPointer = CreateEntryBuffer( false );
    //    IRDescriptorTableEntry *entry         = (IRDescriptorTableEntry *)bufferPointer.contents;
    //    IRDescriptorTableSetBuffer( entry, metalBuffer->Instance( ).gpuAddress, 0 );
    //    SetGpuAddress( binding.Parent.LocationHint, bufferPointer.gpuAddress );

    PushUpdateDesc( slot, metalBuffer->Usage( ), metalBuffer, m_buffers );

    MTLArgumentDescriptor *descriptor = CreateArgumentDescriptor( slot );

    //    descriptor.dataType               = metalBuffer->Type( );
}

void MetalResourceBindGroup::BindSampler( const ResourceBindingSlot &slot, ISampler *sampler )
{
    //    const MetalBindingDesc &binding      = m_rootSignature->FindMetalBinding( slot );
    MetalSampler *metalSampler = static_cast<MetalSampler *>( sampler );

    //    id<MTLBuffer>           samplerPointer = CreateEntryBuffer( true );
    //    IRDescriptorTableEntry *entry          = (IRDescriptorTableEntry *)samplerPointer.contents;
    //
    //    IRDescriptorTableSetSampler( entry, metalSampler->Instance( ), 0 );
    //    SetGpuAddress( binding.Parent.LocationHint, samplerPointer.gpuAddress );

    PushUpdateDesc( slot, 0, metalSampler, m_samplers );

    MTLArgumentDescriptor *descriptor = CreateArgumentDescriptor( slot );
}

MTLArgumentDescriptor *MetalResourceBindGroup::CreateArgumentDescriptor( const DenOfIz::ResourceBindingSlot &slot )
{
    const MetalBindingDesc &binding = m_rootSignature->FindMetalBinding( slot );

    MTLArgumentDescriptor *descriptor = [[MTLArgumentDescriptor alloc] init];
    descriptor.access                 = MetalEnumConverter::ConvertDescriptorToBindingAccess( binding.Parent.Descriptor );
    descriptor.arrayLength            = binding.Parent.ArraySize == 1 ? 0 : binding.Parent.ArraySize; // 0 means it's not an array for Metal
    descriptor.index                  = binding.Parent.Reflection.LocationHint;

    switch ( binding.Parent.Reflection.Type )
    {
    case ReflectionBindingType::Pointer:
        descriptor.dataType = MTLDataTypePointer;
        break;
    case ReflectionBindingType::Struct:
        descriptor.dataType = MTLDataTypePointer; // todo validate
        break;
    case ReflectionBindingType::SamplerDesc:
        descriptor.dataType = MTLDataTypeSampler;
        break;
    case ReflectionBindingType::Texture:
        descriptor.dataType = MTLDataTypeTexture;
        break;
    }

    [m_argumentDescriptors addObject:descriptor];
    return descriptor;
}

id<MTLBuffer> MetalResourceBindGroup::CreateEntryBuffer( bool readonlyHeap )
{
    readonlyHeap = false; // TODO remove after testing
    if ( readonlyHeap )
    {
        m_bindHeap = true;
        return [m_context->ReadOnlyHeap newBufferWithLength:sizeof( IRDescriptorTableEntry ) options:MTLResourceStorageModeShared];
    }

    m_bindBuffer = true;
    return [m_context->Device newBufferWithLength:sizeof( IRDescriptorTableEntry ) options:MTLResourceStorageModeShared];
}

void MetalResourceBindGroup::SetGpuAddress( uint32_t binding, uint64_t address )
{
    if ( m_descriptorTable.size( ) <= binding )
    {
        LOG( ERROR ) << "Unable to find binding[" << binding << "].";
    }

    m_descriptorTable[ binding ] = address;
}

bool MetalResourceBindGroup::BindBuffer( ) const
{
    return m_bindBuffer;
}

const id<MTLBuffer> &MetalResourceBindGroup::ArgumentBuffer( ) const
{
    return m_argumentBuffer;
}

bool MetalResourceBindGroup::BindHeap( ) const
{
    return m_bindHeap;
}

const id<MTLHeap> &MetalResourceBindGroup::Heap( ) const
{
    return m_context->ReadOnlyHeap;
    //    return m_heap;
}

const std::vector<MetalUpdateDescItem<MetalBufferResource>> &MetalResourceBindGroup::Buffers( ) const
{
    return m_buffers;
}

const std::vector<MetalUpdateDescItem<MetalTextureResource>> &MetalResourceBindGroup::Textures( ) const
{
    return m_textures;
}

const std::vector<MetalUpdateDescItem<MetalSampler>> &MetalResourceBindGroup::Samplers( ) const
{
    return m_samplers;
}
