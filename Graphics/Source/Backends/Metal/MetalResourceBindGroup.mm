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

MetalResourceBindGroup::MetalResourceBindGroup( MetalContext *context, ResourceBindGroupDesc desc ) : m_desc( desc ), m_context( context ), m_updateDesc( desc.RegisterSpace )
{
    m_context       = context;
    m_rootSignature = static_cast<MetalRootSignature *>( desc.RootSignature );
    if ( desc.RegisterSpace == DZConfiguration::Instance( ).RootConstantRegisterSpace )
    {
        m_rootConstant.resize( m_rootSignature->NumRootConstantBytes( ) );
    }
}

void MetalResourceBindGroup::SetRootConstants( uint32_t binding, void *data )
{
    // Find binding offset and copy data to m_rootConstant
    const auto &rootConstants = m_rootSignature->RootConstants( );
    if ( binding >= rootConstants.size( ) )
    {
        LOG( FATAL ) << "Root constant binding out of range";
    }
    const auto &rootConstantBinding = rootConstants[ binding ];
    std::memcpy( &m_rootConstant[ rootConstantBinding.Offset ], data, rootConstantBinding.NumBytes );
}

void MetalResourceBindGroup::Update( const UpdateDesc &desc )
{
    m_updateDesc = desc;

    size_t cbvSrvUavTableSize = desc.Buffers.size( ) + desc.Textures.size( );
    if ( desc.RegisterSpace == DZConfiguration::Instance( ).RootLevelBufferRegisterSpace )
    {
        // Buffers will be bound separately
        cbvSrvUavTableSize = desc.Textures.size( );
    }

    if ( cbvSrvUavTableSize > 0 )
    {
        m_cbvSrvUavTable = std::make_unique<MetalDescriptorTableBinding>( 0, DescriptorTable( m_context, cbvSrvUavTableSize ) );
        m_cbvSrvUavTable->Table.SetDebugName( "CbvSrvUav Table[Space: " + std::to_string( m_desc.RegisterSpace ) + "]" );
    }
    if ( !desc.Samplers.empty( ) )
    {
        m_samplerTable = std::make_unique<MetalDescriptorTableBinding>( 0, DescriptorTable( m_context, desc.Samplers.size( ) ) );
        m_samplerTable->Table.SetDebugName( "Sampler Table[Space: " + std::to_string( m_desc.RegisterSpace ) + "]" );
    }

    for ( auto item : desc.Buffers )
    {
        BindBuffer( item.Slot, item.Resource );
    }
    for ( auto item : desc.Textures )
    {
        BindTexture( item.Slot, item.Resource );
    }
    for ( auto item : desc.Samplers )
    {
        BindSampler( item.Slot, item.Resource );
    }
}

void MetalResourceBindGroup::BindBuffer( const ResourceBindingSlot &slot, IBufferResource *resource )
{
    MetalBufferResource    *metalBuffer = static_cast<MetalBufferResource *>( resource );
    const MetalBindingDesc &binding     = m_rootSignature->FindMetalBinding( slot );
    if ( slot.RegisterSpace == DZConfiguration::Instance( ).RootLevelBufferRegisterSpace )
    {
        m_rootParameterBindings.emplace_back( binding.Parent.Reflection.TLABOffset, metalBuffer->Instance( ) );
        return;
    }
    m_cbvSrvUavTable->Table.EncodeBuffer( metalBuffer->Instance( ), binding.Parent.Reflection.DescriptorTableIndex );
    UpdateDescriptorTable( binding, m_cbvSrvUavTable.get( ) );
    m_buffers.emplace_back( metalBuffer, binding.Stages, metalBuffer->Usage( ) );
}

void MetalResourceBindGroup::BindTexture( const ResourceBindingSlot &slot, ITextureResource *resource )
{
    MetalTextureResource   *metalTexture = static_cast<MetalTextureResource *>( resource );
    const MetalBindingDesc &binding      = m_rootSignature->FindMetalBinding( slot );
    m_cbvSrvUavTable->Table.EncodeTexture( metalTexture->Instance( ), metalTexture->MinLODClamp( ), binding.Parent.Reflection.DescriptorTableIndex );
    UpdateDescriptorTable( binding, m_cbvSrvUavTable.get( ) );
    m_textures.emplace_back( metalTexture, binding.Stages, metalTexture->Usage( ) );
}

void MetalResourceBindGroup::BindSampler( const ResourceBindingSlot &slot, ISampler *sampler )
{
    MetalSampler           *metalSampler = static_cast<MetalSampler *>( sampler );
    const MetalBindingDesc &binding      = m_rootSignature->FindMetalBinding( slot );
    m_samplerTable->Table.EncodeSampler( metalSampler->Instance( ), metalSampler->LODBias( ), binding.Parent.Reflection.DescriptorTableIndex );
    UpdateDescriptorTable( binding, m_samplerTable.get( ) );
    m_samplers.emplace_back( metalSampler, binding.Stages, MTLResourceUsageRead );
}

const std::vector<Byte> &MetalResourceBindGroup::RootConstant( ) const
{
    return m_rootConstant;
}

const std::vector<MetalRootParameterBinding> &MetalResourceBindGroup::RootParameters( ) const
{
    return m_rootParameterBindings;
}

const MetalDescriptorTableBinding *MetalResourceBindGroup::CbvSrvUavTable( ) const
{
    return m_cbvSrvUavTable.get( );
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
    table->TLABOffset = binding.Parent.Reflection.TLABOffset;
    table->NumEntries++;
}
