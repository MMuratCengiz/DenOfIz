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
    m_context       = context;
    m_rootSignature = static_cast<MetalRootSignature *>( desc.RootSignature );
}

void MetalResourceBindGroup::Update( const UpdateDesc &desc )
{
    m_updateDesc = desc;

    if ( !desc.Buffers.empty( ) )
    {
        m_bufferTable = std::make_unique<MetalDescriptorTableBinding>( 0, DescriptorTable( m_context, desc.Buffers.size( ) ) );
    }
    if ( !desc.Textures.empty( ) )
    {
        m_textureTable = std::make_unique<MetalDescriptorTableBinding>( 0, DescriptorTable( m_context, desc.Textures.size( ) ) );
    }
    if ( !desc.Samplers.empty( ) )
    {
        m_samplerTable = std::make_unique<MetalDescriptorTableBinding>( 0, DescriptorTable( m_context, desc.Samplers.size( ) ) );
    }

    IResourceBindGroup::Update( desc );
}

void MetalResourceBindGroup::BindBuffer( const ResourceBindingSlot &slot, IBufferResource *resource )
{
    MetalBufferResource    *metalBuffer = static_cast<MetalBufferResource *>( resource );
    const MetalBindingDesc &binding     = m_rootSignature->FindMetalBinding( slot );
    m_bufferTable->Table.EncodeBuffer( metalBuffer->Instance( ), binding.Parent.Reflection.DescriptorTableIndex );
    UpdateDescriptorTable( binding, m_bufferTable.get( ) );
}

void MetalResourceBindGroup::BindTexture( const ResourceBindingSlot &slot, ITextureResource *resource )
{
    MetalTextureResource   *metalTexture = static_cast<MetalTextureResource *>( resource );
    const MetalBindingDesc &binding      = m_rootSignature->FindMetalBinding( slot );
    m_textureTable->Table.EncodeTexture( metalTexture->Instance( ), binding.Parent.Reflection.DescriptorTableIndex );
    UpdateDescriptorTable( binding, m_textureTable.get( ) );
}

void MetalResourceBindGroup::BindSampler( const ResourceBindingSlot &slot, ISampler *sampler )
{
    MetalSampler           *metalSampler = static_cast<MetalSampler *>( sampler );
    const MetalBindingDesc &binding      = m_rootSignature->FindMetalBinding( slot );
    m_samplerTable->Table.EncodeSampler( metalSampler->Instance( ), binding.Parent.Reflection.DescriptorTableIndex );
    UpdateDescriptorTable( binding, m_samplerTable.get( ) );
}

const MetalDescriptorTableBinding *MetalResourceBindGroup::BufferTable( ) const
{
    return m_bufferTable.get( );
}

const MetalDescriptorTableBinding *MetalResourceBindGroup::TextureTable( ) const
{
    return m_textureTable.get( );
}

const MetalDescriptorTableBinding *MetalResourceBindGroup::SamplerTable( ) const
{
    return m_samplerTable.get( );
}

MetalRootSignature *MetalResourceBindGroup::RootSignature( ) const
{
    return m_rootSignature;
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
void MetalResourceBindGroup::UpdateDescriptorTable( const MetalBindingDesc &binding, MetalDescriptorTableBinding *table )
{
    table->TLABOffset = binding.Parent.Reflection.DescriptorOffset;
    table->NumEntries++;
}
