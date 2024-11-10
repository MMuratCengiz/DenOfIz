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
#include <DenOfIzGraphics/Backends/Metal/RayTracing/MetalTopLevelAS.h>

using namespace DenOfIz;

MetalResourceBindGroup::MetalResourceBindGroup( MetalContext *context, ResourceBindGroupDesc desc ) : m_desc( desc ), m_context( context )
{
    m_context       = context;
    m_rootSignature = static_cast<MetalRootSignature *>( desc.RootSignature );
    if ( desc.RegisterSpace == DZConfiguration::Instance( ).RootConstantRegisterSpace )
    {
        m_rootConstant.resize( m_rootSignature->NumRootConstantBytes( ) );
    }
}

void MetalResourceBindGroup::SetRootConstantsData( uint32_t binding, const InteropArray<Byte> &data )
{
    const auto &rootConstants = m_rootSignature->RootConstants( );
    if ( binding >= rootConstants.size( ) )
    {
        LOG( FATAL ) << "Root constant binding out of range";
    }
    const auto &rootConstantBinding = rootConstants[ binding ];
    if ( data.NumElements( ) != rootConstantBinding.NumBytes )
    {
        LOG( ERROR ) << "Root constant size mismatch. Expected: " << rootConstantBinding.NumBytes << ", Got: " << data.NumElements( );
        return;
    }
    SetRootConstants( binding, (void *)data.Data( ) );
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

IResourceBindGroup *MetalResourceBindGroup::BeginUpdate( )
{
    m_boundAccelerationStructures.clear( );
    m_boundBuffers.clear( );
    m_boundTextures.clear( );
    m_boundSamplers.clear( );
    m_rootParameterBindings.clear( );
    m_indirectResources.clear();
    m_buffers.clear( );
    m_textures.clear( );
    m_samplers.clear( );
    return this;
}

IResourceBindGroup *MetalResourceBindGroup::Cbv( const uint32_t binding, IBufferResource *resource )
{
    m_boundBuffers.emplace_back( GetSlot( binding, DescriptorBufferBindingType::ConstantBuffer ), resource );
    return this;
}

IResourceBindGroup *MetalResourceBindGroup::Srv( const uint32_t binding, IBufferResource *resource )
{
    m_boundBuffers.emplace_back( GetSlot( binding, DescriptorBufferBindingType::ShaderResource ), resource );
    return this;
}

IResourceBindGroup *MetalResourceBindGroup::Srv( const uint32_t binding, ITextureResource *resource )
{
    m_boundTextures.emplace_back( GetSlot( binding, DescriptorBufferBindingType::ShaderResource ), resource );
    return this;
}

IResourceBindGroup *MetalResourceBindGroup::Srv( const uint32_t binding, ITopLevelAS *accelerationStructure )
{
    m_boundAccelerationStructures.emplace_back( GetSlot( binding, DescriptorBufferBindingType::ShaderResource ), accelerationStructure );
    return this;
}

IResourceBindGroup *MetalResourceBindGroup::Uav( const uint32_t binding, IBufferResource *resource )
{
    m_boundBuffers.emplace_back( GetSlot( binding, DescriptorBufferBindingType::UnorderedAccess ), resource );
    return this;
}

IResourceBindGroup *MetalResourceBindGroup::Uav( const uint32_t binding, ITextureResource *resource )
{
    m_boundTextures.emplace_back( GetSlot( binding, DescriptorBufferBindingType::UnorderedAccess ), resource );
    return this;
}

IResourceBindGroup *MetalResourceBindGroup::Sampler( const uint32_t binding, ISampler *sampler )
{
    m_boundSamplers.emplace_back( GetSlot( binding, DescriptorBufferBindingType::Sampler ), sampler );
    return this;
}

void MetalResourceBindGroup::EndUpdate( )
{
    size_t cbvSrvUavTableSize = m_boundAccelerationStructures.size( ) + m_boundBuffers.size( ) + m_boundTextures.size( );
    if ( m_desc.RegisterSpace == DZConfiguration::Instance( ).RootLevelBufferRegisterSpace )
    {
        // Buffers will be bound separately
        cbvSrvUavTableSize = m_boundTextures.size( );
    }

    if ( cbvSrvUavTableSize > 0 )
    {
        m_cbvSrvUavTable = std::make_unique<MetalDescriptorTableBinding>( 0, DescriptorTable( m_context, cbvSrvUavTableSize ) );
        m_cbvSrvUavTable->Table.SetDebugName( "CbvSrvUav Table[Space: " + std::to_string( m_desc.RegisterSpace ) + "]" );
    }
    if ( !m_boundSamplers.empty( ) )
    {
        m_samplerTable = std::make_unique<MetalDescriptorTableBinding>( 0, DescriptorTable( m_context, m_boundSamplers.size( ) ) );
        m_samplerTable->Table.SetDebugName( "Sampler Table[Space: " + std::to_string( m_desc.RegisterSpace ) + "]" );
    }

    for ( auto item : m_boundBuffers )
    {
        BindBuffer( item.first, item.second );
    }
    for ( auto item : m_boundAccelerationStructures )
    {
        BindAccelerationStructure( item.first, item.second );
    }
    for ( auto item : m_boundTextures )
    {
        BindTexture( item.first, item.second );
    }
    for ( auto item : m_boundSamplers )
    {
        BindSampler( item.first, item.second );
    }
}

void MetalResourceBindGroup::BindAccelerationStructure( const ResourceBindingSlot &slot, ITopLevelAS *accelerationStructure )
{
    MetalTopLevelAS     *metalAS      = static_cast<MetalTopLevelAS *>( accelerationStructure );
    for ( const auto &item : metalAS->IndirectResources( ) )
    {
        m_indirectResources.emplace_back( item );
    }

    const MetalBindingDesc &metalBinding = m_rootSignature->FindMetalBinding( slot );
    if ( slot.RegisterSpace == DZConfiguration::Instance( ).RootLevelBufferRegisterSpace )
    {
        m_rootParameterBindings.emplace_back( metalBinding.Parent.Reflection.TLABOffset, metalAS->HeaderBuffer( ) );
        return;
    }

    m_cbvSrvUavTable->Table.EncodeAccelerationStructure( metalAS->HeaderBuffer( ), metalBinding.Parent.Reflection.DescriptorTableIndex );
    UpdateDescriptorTable( metalBinding, m_cbvSrvUavTable.get( ) );
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

const std::vector<id<MTLResource>> &MetalResourceBindGroup::IndirectResources( ) const
{
    return m_indirectResources;
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

ResourceBindingSlot MetalResourceBindGroup::GetSlot( uint32_t binding, const DescriptorBufferBindingType &type ) const
{
    return ResourceBindingSlot{ .Type = type, .Binding = binding, .RegisterSpace = m_desc.RegisterSpace };
}
