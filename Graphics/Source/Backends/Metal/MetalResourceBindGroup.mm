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
    m_boundBuffersWithOffsets.clear( );
    m_boundTextures.clear( );
    m_boundSamplers.clear( );
    m_rootParameterBindings.clear( );
    m_indirectResources.clear( );
    m_buffers.clear( );
    m_textures.clear( );
    m_samplers.clear( );
    return this;
}

IResourceBindGroup *MetalResourceBindGroup::Cbv( const uint32_t binding, IBufferResource *resource )
{
    m_boundBuffers.emplace_back( GetSlot( binding, ResourceBindingType::ConstantBuffer ), resource );
    return this;
}

IResourceBindGroup *MetalResourceBindGroup::Cbv( const BindBufferDesc& desc )
{
    m_boundBuffersWithOffsets.emplace_back( MetalBufferBindingWithOffset{
        GetSlot( desc.Binding, ResourceBindingType::ConstantBuffer ),
        desc.Resource,
        desc.ResourceOffset
    } );
    return this;
}

IResourceBindGroup *MetalResourceBindGroup::Srv( const uint32_t binding, IBufferResource *resource )
{
    m_boundBuffers.emplace_back( GetSlot( binding, ResourceBindingType::ShaderResource ), resource );
    return this;
}

IResourceBindGroup *MetalResourceBindGroup::Srv( const BindBufferDesc& desc )
{
    // Use our custom binding with offset to handle buffer offsets in Metal
    m_boundBuffersWithOffsets.emplace_back( MetalBufferBindingWithOffset{
        GetSlot( desc.Binding, ResourceBindingType::ShaderResource ),
        desc.Resource,
        desc.ResourceOffset
    } );
    return this;
}

IResourceBindGroup *MetalResourceBindGroup::Srv( const uint32_t binding, ITextureResource *resource )
{
    m_boundTextures.emplace_back( GetSlot( binding, ResourceBindingType::ShaderResource ), resource );
    return this;
}

IResourceBindGroup *MetalResourceBindGroup::Srv( const uint32_t binding, ITopLevelAS *accelerationStructure )
{
    m_boundAccelerationStructures.emplace_back( GetSlot( binding, ResourceBindingType::ShaderResource ), accelerationStructure );
    return this;
}

IResourceBindGroup *MetalResourceBindGroup::Uav( const uint32_t binding, IBufferResource *resource )
{
    m_boundBuffers.emplace_back( GetSlot( binding, ResourceBindingType::UnorderedAccess ), resource );
    return this;
}

IResourceBindGroup *MetalResourceBindGroup::Uav( const BindBufferDesc& desc )
{
    // Use our custom binding with offset to handle buffer offsets in Metal
    m_boundBuffersWithOffsets.emplace_back( MetalBufferBindingWithOffset{
        GetSlot(desc.Binding, ResourceBindingType::UnorderedAccess),
        desc.Resource,
        desc.ResourceOffset
    } );
    return this;
}

IResourceBindGroup *MetalResourceBindGroup::Uav( const uint32_t binding, ITextureResource *resource )
{
    m_boundTextures.emplace_back( GetSlot( binding, ResourceBindingType::UnorderedAccess ), resource );
    return this;
}

IResourceBindGroup *MetalResourceBindGroup::Sampler( const uint32_t binding, ISampler *sampler )
{
    m_boundSamplers.emplace_back( GetSlot( binding, ResourceBindingType::Sampler ), sampler );
    return this;
}

void MetalResourceBindGroup::EndUpdate( )
{
    size_t cbvSrvUavTableSize = m_boundAccelerationStructures.size( ) + m_boundBuffers.size( ) + m_boundBuffersWithOffsets.size( ) + m_boundTextures.size( );
    if ( m_desc.RegisterSpace == DZConfiguration::Instance( ).RootLevelBufferRegisterSpace )
    {
        // Buffers will be bound separately
        cbvSrvUavTableSize = m_boundTextures.size( );
    }

    if ( cbvSrvUavTableSize > 0 )
    {
        m_cbvSrvUavTable = std::make_unique<MetalDescriptorTableBinding>( 0, DescriptorTable( m_context, cbvSrvUavTableSize ) );
        m_cbvSrvUavTable->Table.SetDebugName( "CbvSrvUav Table[Space: " + std::to_string( m_desc.RegisterSpace ) + "]" );
        m_cbvSrvUavTable->TLABOffset = m_rootSignature->CbvSrvUavTableOffset( m_desc.RegisterSpace );
    }
    if ( !m_boundSamplers.empty( ) )
    {
        m_samplerTable = std::make_unique<MetalDescriptorTableBinding>( 0, DescriptorTable( m_context, m_boundSamplers.size( ) ) );
        m_samplerTable->Table.SetDebugName( "Sampler Table[Space: " + std::to_string( m_desc.RegisterSpace ) + "]" );
        m_samplerTable->TLABOffset = m_rootSignature->SamplerTableOffset( m_desc.RegisterSpace );
    }

    for ( auto item : m_boundBuffers )
    {
        BindBuffer( item.first, item.second );
    }
    
    for ( auto item : m_boundBuffersWithOffsets )
    {
        BindBufferWithOffset( item.Slot, item.Resource, item.Offset );
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
    MetalTopLevelAS *metalAS = static_cast<MetalTopLevelAS *>( accelerationStructure );
    for ( const auto &item : metalAS->IndirectResources( ) )
    {
        m_indirectResources.emplace_back( item );
    }

    if ( slot.RegisterSpace == DZConfiguration::Instance( ).RootLevelBufferRegisterSpace )
    {
        m_rootParameterBindings.emplace_back( m_rootSignature->TLABOffset( slot ), metalAS->HeaderBuffer( ) );
        return;
    }

    m_cbvSrvUavTable->Table.EncodeAccelerationStructure( metalAS->HeaderBuffer( ), m_rootSignature->CbvSrvUavResourceIndex( slot ) );
    m_cbvSrvUavTable->NumEntries++;
}

void MetalResourceBindGroup::BindBuffer( const ResourceBindingSlot &slot, IBufferResource *resource )
{
    MetalBufferResource    *metalBuffer = static_cast<MetalBufferResource *>( resource );
    if ( slot.RegisterSpace == DZConfiguration::Instance( ).RootLevelBufferRegisterSpace )
    {
        m_rootParameterBindings.emplace_back( m_rootSignature->TLABOffset( slot ), metalBuffer->Instance( ) );
        return;
    }

    m_cbvSrvUavTable->Table.EncodeBuffer( metalBuffer->Instance( ), m_rootSignature->CbvSrvUavResourceIndex( slot ) );
    m_cbvSrvUavTable->NumEntries++;
    m_buffers.emplace_back( metalBuffer, m_rootSignature->CbvSrvUavResourceShaderStages( slot ), metalBuffer->Usage( ) );
}

void MetalResourceBindGroup::BindBufferWithOffset( const ResourceBindingSlot &slot, IBufferResource *resource, uint32_t offset )
{
    MetalBufferResource    *metalBuffer = static_cast<MetalBufferResource *>( resource );
    if ( slot.RegisterSpace == DZConfiguration::Instance( ).RootLevelBufferRegisterSpace )
    {
        // For root parameters, we handle the offset in the argument buffer directly
        // Metal uses gpuAddress + offset for binding with offsets
        m_rootParameterBindings.emplace_back( m_rootSignature->TLABOffset( slot ), metalBuffer->Instance( ) );
        return;
    }

    m_cbvSrvUavTable->Table.EncodeBuffer( metalBuffer->Instance( ), m_rootSignature->CbvSrvUavResourceIndex( slot ), offset );
    m_cbvSrvUavTable->NumEntries++;
    m_buffers.emplace_back( metalBuffer, m_rootSignature->CbvSrvUavResourceShaderStages( slot ), metalBuffer->Usage( ) );
}

void MetalResourceBindGroup::BindTexture( const ResourceBindingSlot &slot, ITextureResource *resource )
{
    MetalTextureResource   *metalTexture = static_cast<MetalTextureResource *>( resource );

    m_cbvSrvUavTable->Table.EncodeTexture( metalTexture->Instance( ), metalTexture->MinLODClamp( ), m_rootSignature->CbvSrvUavResourceIndex( slot ) );
    m_cbvSrvUavTable->NumEntries++;
    m_textures.emplace_back( metalTexture, m_rootSignature->CbvSrvUavResourceShaderStages( slot ), metalTexture->Usage( ) );
}

void MetalResourceBindGroup::BindSampler( const ResourceBindingSlot &slot, ISampler *sampler )
{
    MetalSampler           *metalSampler = static_cast<MetalSampler *>( sampler );

    m_samplerTable->Table.EncodeSampler( metalSampler->Instance( ), metalSampler->LODBias( ), m_rootSignature->SamplerResourceIndex( slot ) );
    m_samplerTable->NumEntries++;
    m_samplers.emplace_back( metalSampler, m_rootSignature->SamplerResourceShaderStages( slot ), MTLResourceUsageRead );
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

ResourceBindingSlot MetalResourceBindGroup::GetSlot( uint32_t binding, const ResourceBindingType &type ) const
{
    return ResourceBindingSlot{ .Type = type, .Binding = binding, .RegisterSpace = m_desc.RegisterSpace };
}
