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
    m_context         = context;
    m_rootSignature   = static_cast<MetalRootSignature *>( desc.RootSignature );
    m_descriptorTable = m_rootSignature->DescriptorTable( desc.RegisterSpace );

    unsigned long alignedSize = Utilities::Align( m_descriptorTable.size( ) * sizeof( uint64_t ), 8 );
    m_argumentBuffer          = [m_context->Device newBufferWithLength:alignedSize options:MTLResourceStorageModeShared];
}

void MetalResourceBindGroup::Update( const UpdateDesc &desc )
{
    m_updateDesc = desc;
    IResourceBindGroup::Update( desc );
    std::memcpy( m_argumentBuffer.contents, m_descriptorTable.data( ), m_descriptorTable.size( ) * sizeof( uint64_t ) );
}

void MetalResourceBindGroup::BindTexture( const ResourceBindingSlot &slot, ITextureResource *resource )
{
    MetalTextureResource *metalTexture = static_cast<MetalTextureResource *>( resource );

    bool                    readonlyHeap   = ( metalTexture->Usage( ) & MTLTextureUsageRenderTarget ) == 0 && ( metalTexture->Usage( ) & MTLTextureUsageShaderWrite ) == 0;
    id<MTLBuffer>           texturePointer = CreateEntryBuffer( readonlyHeap );
    IRDescriptorTableEntry *entry          = (IRDescriptorTableEntry *)texturePointer.contents;

    IRDescriptorTableSetTexture( entry, metalTexture->Instance( ), 0, 0 );
    SetGpuAddress( slot.Binding, texturePointer.gpuAddress );
}

void MetalResourceBindGroup::BindBuffer( const ResourceBindingSlot &slot, IBufferResource *resource )
{
    const MetalBindingDesc &binding     = m_rootSignature->FindMetalBinding( slot );
    MetalBufferResource    *metalBuffer = static_cast<MetalBufferResource *>( resource );

    id<MTLBuffer>           bufferPointer = CreateEntryBuffer( false );
    IRDescriptorTableEntry *entry         = (IRDescriptorTableEntry *)bufferPointer.contents;

    IRDescriptorTableSetBuffer( entry, metalBuffer->Instance( ).gpuAddress, 0 );
    SetGpuAddress( slot.Binding, bufferPointer.gpuAddress );
}

void MetalResourceBindGroup::BindSampler( const ResourceBindingSlot &slot, ISampler *sampler )
{
    MetalSampler *metalSampler = static_cast<MetalSampler *>( sampler );

    id<MTLBuffer>           samplerPointer = CreateEntryBuffer( true );
    IRDescriptorTableEntry *entry          = (IRDescriptorTableEntry *)samplerPointer.contents;

    IRDescriptorTableSetSampler( entry, metalSampler->Instance( ), 0 );
    SetGpuAddress( slot.Binding, samplerPointer.gpuAddress );
}

id<MTLBuffer> MetalResourceBindGroup::CreateEntryBuffer( bool readonlyHeap )
{
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
