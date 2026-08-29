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

#include "DenOfIzGraphicsInternal/Backends/Metal/MetalBindGroup.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalTopLevelAS.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

MetalBindGroup::MetalBindGroup( MetalContext *context, DenOfIz_BindGroupDesc desc ) : m_desc( desc ), m_context( context )
{
    m_context         = context;
    m_bindGroupLayout = static_cast<MetalBindGroupLayout *>( DENOFIZ_FROM_HANDLE( IBindGroupLayout, desc.Layout ) );
}

IBindGroup *MetalBindGroup::BeginUpdate( )
{
    m_boundAccelerationStructures.clear( );
    m_boundBuffers.clear( );
    m_boundBuffersWithOffsets.clear( );
    m_boundTextures.clear( );
    m_boundTextureArrayIndices.clear( );
    m_boundSamplers.clear( );
    m_rootParameterBindings.clear( );
    m_indirectResources.clear( );
    m_buffers.clear( );
    m_textures.clear( );
    m_samplers.clear( );
    return this;
}

IBindGroup *MetalBindGroup::Cbv( const uint32_t binding, IBuffer *resource )
{
    m_boundBuffers.emplace_back( GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER ), resource );
    return this;
}

IBindGroup *MetalBindGroup::Cbv( const uint32_t binding, IBuffer *resource, size_t resourceOffset )
{
    m_boundBuffersWithOffsets.emplace_back( MetalBufferBindingWithOffset{
        GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER ),
        resource,
        resourceOffset
    } );
    return this;
}

IBindGroup *MetalBindGroup::Srv( const uint32_t binding, IBuffer *resource )
{
    m_boundBuffers.emplace_back( GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE ), resource );
    return this;
}

IBindGroup *MetalBindGroup::Srv( const uint32_t binding, IBuffer *resource, size_t resourceOffset )
{
    m_boundBuffersWithOffsets.emplace_back( MetalBufferBindingWithOffset{
        GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE ),
        resource,
        resourceOffset
    } );
    return this;
}

IBindGroup *MetalBindGroup::Srv( const uint32_t binding, ITexture *resource, const uint32_t mipLevel )
{
    m_boundTextures.push_back( { GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE ), mipLevel, resource } );
    return this;
}

IBindGroup *MetalBindGroup::SrvArray( const uint32_t binding, const DenOfIz_TextureArray &resources )
{
    for ( uint32_t i = 0; i < resources.NumElements; ++i )
    {
        auto *metalResource  = dynamic_cast<MetalTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, resources.Elements[ i ] ) );
        SrvArrayIndex( binding, i, metalResource );
    }
    return this;
}

IBindGroup *MetalBindGroup::SrvArrayIndex( const uint32_t binding, uint32_t arrayIndex, ITexture *resource )
{
    const DenOfIz_ResourceBindingSlot slot = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE );
    
    m_boundTextureArrayIndices.emplace_back( MetalTextureArrayIndexBinding{
        slot, arrayIndex, resource
    } );
    
    return this;
}

IBindGroup *MetalBindGroup::Srv( const uint32_t binding, ITopLevelAS *accelerationStructure )
{
    m_boundAccelerationStructures.emplace_back( GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE ), accelerationStructure );
    return this;
}

IBindGroup *MetalBindGroup::Uav( const uint32_t binding, IBuffer *resource )
{
    m_boundBuffers.emplace_back( GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS ), resource );
    return this;
}

IBindGroup *MetalBindGroup::Uav( const uint32_t binding, IBuffer *resource, size_t resourceOffset )
{
    m_boundBuffersWithOffsets.emplace_back( MetalBufferBindingWithOffset{
        GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS ),
        resource,
        resourceOffset
    } );
    return this;
}

IBindGroup *MetalBindGroup::Uav( const uint32_t binding, ITexture *resource, const uint32_t mipLevel )
{
    m_boundTextures.push_back( { GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS ), mipLevel, resource } );
    return this;
}

IBindGroup *MetalBindGroup::Sampler( const uint32_t binding, ISampler *sampler )
{
    m_boundSamplers.emplace_back( GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER ), sampler );
    return this;
}

void MetalBindGroup::EndUpdate( )
{
    @autoreleasepool
    {
        uint32_t registerSpace     = m_bindGroupLayout->RegisterSpace( );
        uint32_t cbvSrvUavTableSize = m_bindGroupLayout->CbvSrvUavTableSize( );
        if ( cbvSrvUavTableSize == 0 )
        {
            cbvSrvUavTableSize = m_boundAccelerationStructures.size( ) + m_boundBuffers.size( ) + m_boundBuffersWithOffsets.size( ) + m_boundTextures.size( ) + m_boundTextureArrayIndices.size( );
            if ( registerSpace == DENOFIZ_ROOT_LEVEL_BUFFER_REGISTER_SPACE )
            {
                cbvSrvUavTableSize = m_boundTextures.size( ) + m_boundTextureArrayIndices.size( );
            }
        }

        if ( cbvSrvUavTableSize > 0 )
        {
            if ( !m_cbvSrvUavTable )
            {
                m_cbvSrvUavTable = std::make_unique<MetalDescriptorTableBinding>( 0, DescriptorTable( m_context, cbvSrvUavTableSize ) );
                m_cbvSrvUavTable->Table.SetDebugName( "CbvSrvUav Table[Space: " + std::to_string( registerSpace ) + "]" );
                m_cbvSrvUavTable->TLABOffset = m_bindGroupLayout->CbvSrvUavTableOffset( );
            }
            else
            {
                m_cbvSrvUavTable->Table.Reset( cbvSrvUavTableSize );
            }
            m_cbvSrvUavTable->NumEntries = 0;
        }
        if ( !m_boundSamplers.empty( ) )
        {
            if ( !m_samplerTable )
            {
                m_samplerTable = std::make_unique<MetalDescriptorTableBinding>( 0, DescriptorTable( m_context, m_boundSamplers.size( ) ) );
                m_samplerTable->Table.SetDebugName( "Sampler Table[Space: " + std::to_string( registerSpace ) + "]" );
                m_samplerTable->TLABOffset = m_bindGroupLayout->SamplerTableOffset( );
            }
            else
            {
                m_samplerTable->Table.Reset( m_boundSamplers.size( ) );
            }
            m_samplerTable->NumEntries = 0;
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
        for ( const auto &[slot, mipLevel, resource] : m_boundTextures )
        {
            BindTexture( slot, resource, mipLevel );
        }
        for ( auto item : m_boundTextureArrayIndices )
        {
            BindTextureArrayIndex( item.Slot, item.ArrayIndex, item.Resource );
        }
        for ( auto item : m_boundSamplers )
        {
            BindSampler( item.first, item.second );
        }

        if ( getenv( "DZ_DEBUG_DISPATCH" ) && m_cbvSrvUavTable )
        {
            static int dumpCount = 0;
            if ( dumpCount++ < 6 )
            {
                id<MTLBuffer>   tableBuffer = m_cbvSrvUavTable->Table.Buffer( );
                const auto     *entries     = static_cast<const IRDescriptorTableEntry *>( tableBuffer.contents );
                const uint32_t  numEntries  = static_cast<uint32_t>( tableBuffer.length / sizeof( IRDescriptorTableEntry ) );
                spdlog::warn( "BindGroup[space {}] table gpuAddr={:#x} entries={}", m_bindGroupLayout->RegisterSpace( ), tableBuffer.gpuAddress, numEntries );
                for ( uint32_t i = 0; i < numEntries; ++i )
                {
                    spdlog::warn( "  entry[{}] gpuVA={:#x} texViewID={:#x} metadata={:#x}", i, entries[ i ].gpuVA, entries[ i ].textureViewID, entries[ i ].metadata );
                }
                for ( const auto &tex : m_textures )
                {
                    spdlog::warn( "  bound texture resID={:#x} type={} label={}", tex.Resource->Instance( ).gpuResourceID._impl, (int)tex.Resource->Instance( ).textureType,
                                  [tex.Resource->Instance( ).label UTF8String] ? [tex.Resource->Instance( ).label UTF8String] : "?" );
                }
            }
        }
    }
}

void MetalBindGroup::BindAccelerationStructure( const DenOfIz_ResourceBindingSlot &slot, ITopLevelAS *accelerationStructure )
{
    MetalTopLevelAS *metalAS = static_cast<MetalTopLevelAS *>( accelerationStructure );
    for ( const auto &item : metalAS->IndirectResources( ) )
    {
        m_indirectResources.emplace_back( item );
    }

    if ( slot.RegisterSpace == DENOFIZ_ROOT_LEVEL_BUFFER_REGISTER_SPACE )
    {
        m_rootParameterBindings.emplace_back( m_bindGroupLayout->TLABOffset( slot ), metalAS->HeaderBuffer( ) );
        return;
    }

    m_cbvSrvUavTable->Table.EncodeAccelerationStructure( metalAS->HeaderBuffer( ), m_bindGroupLayout->CbvSrvUavResourceIndex( slot ) );
    m_cbvSrvUavTable->NumEntries++;
}

void MetalBindGroup::BindBuffer( const DenOfIz_ResourceBindingSlot &slot, IBuffer *resource )
{
    MetalBuffer    *metalBuffer = static_cast<MetalBuffer *>( resource );
    if ( slot.RegisterSpace == DENOFIZ_ROOT_LEVEL_BUFFER_REGISTER_SPACE )
    {
        m_rootParameterBindings.emplace_back( m_bindGroupLayout->TLABOffset( slot ), metalBuffer->Instance( ) );
        return;
    }

    m_cbvSrvUavTable->Table.EncodeBuffer( metalBuffer->Instance( ), m_bindGroupLayout->CbvSrvUavResourceIndex( slot ) );
    m_cbvSrvUavTable->NumEntries++;
    m_buffers.emplace_back( metalBuffer, m_bindGroupLayout->CbvSrvUavResourceShaderStages( slot ), metalBuffer->Usage( ) );
}

void MetalBindGroup::BindBufferWithOffset( const DenOfIz_ResourceBindingSlot &slot, IBuffer *resource, uint32_t offset )
{
    MetalBuffer    *metalBuffer = static_cast<MetalBuffer *>( resource );
    if ( slot.RegisterSpace == DENOFIZ_ROOT_LEVEL_BUFFER_REGISTER_SPACE )
    {
        m_rootParameterBindings.emplace_back( m_bindGroupLayout->TLABOffset( slot ), metalBuffer->Instance( ) );
        return;
    }

    m_cbvSrvUavTable->Table.EncodeBuffer( metalBuffer->Instance( ), m_bindGroupLayout->CbvSrvUavResourceIndex( slot ), offset );
    m_cbvSrvUavTable->NumEntries++;
    m_buffers.emplace_back( metalBuffer, m_bindGroupLayout->CbvSrvUavResourceShaderStages( slot ), metalBuffer->Usage( ) );
}

void MetalBindGroup::BindTexture( const DenOfIz_ResourceBindingSlot &slot, ITexture *resource, const uint32_t mipLevel )
{
    MetalTexture  *metalTexture = static_cast<MetalTexture *>( resource );
    id<MTLTexture> textureView  = mipLevel == DZ_ALL_MIP_LEVELS ? metalTexture->Instance( ) : metalTexture->MipView( mipLevel );

    m_cbvSrvUavTable->Table.EncodeTexture( textureView, metalTexture->MinLODClamp( ), m_bindGroupLayout->CbvSrvUavResourceIndex( slot ) );
    m_cbvSrvUavTable->NumEntries++;

    MTLResourceUsage resourceUsage = MTLResourceUsageRead;
    if ( slot.Type == DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS )
    {
        resourceUsage = MTLResourceUsageRead | MTLResourceUsageWrite;
    }
    m_textures.emplace_back( metalTexture, m_bindGroupLayout->CbvSrvUavResourceShaderStages( slot ), resourceUsage );
}

void MetalBindGroup::BindTextureArrayIndex( const DenOfIz_ResourceBindingSlot &slot, uint32_t arrayIndex, ITexture *resource )
{
    MetalTexture *metalTexture = static_cast<MetalTexture *>( resource );

    const uint32_t baseIndex   = m_bindGroupLayout->CbvSrvUavResourceIndex( slot );
    const uint32_t actualIndex = baseIndex + arrayIndex;

    m_cbvSrvUavTable->Table.EncodeTexture( metalTexture->Instance( ), metalTexture->MinLODClamp( ), actualIndex );

    MTLResourceUsage resourceUsage = MTLResourceUsageRead;
    if ( slot.Type == DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS )
    {
        resourceUsage = MTLResourceUsageRead | MTLResourceUsageWrite;
    }
    m_textures.emplace_back( metalTexture, m_bindGroupLayout->CbvSrvUavResourceShaderStages( slot ), resourceUsage );
}

void MetalBindGroup::BindSampler( const DenOfIz_ResourceBindingSlot &slot, ISampler *sampler )
{
    MetalSampler           *metalSampler = static_cast<MetalSampler *>( sampler );

    m_samplerTable->Table.EncodeSampler( metalSampler->Instance( ), metalSampler->LODBias( ), m_bindGroupLayout->SamplerResourceIndex( slot ) );
    m_samplerTable->NumEntries++;
    m_samplers.emplace_back( metalSampler, m_bindGroupLayout->SamplerResourceShaderStages( slot ), MTLResourceUsageRead );
}

const std::vector<MetalRootParameterBinding> &MetalBindGroup::RootParameters( ) const
{
    return m_rootParameterBindings;
}

const MetalDescriptorTableBinding *MetalBindGroup::CbvSrvUavTable( ) const
{
    return m_cbvSrvUavTable.get( );
}

const MetalDescriptorTableBinding *MetalBindGroup::SamplerTable( ) const
{
    return m_samplerTable.get( );
}

MetalBindGroupLayout *MetalBindGroup::BindGroupLayout( ) const
{
    return m_bindGroupLayout;
}

const std::vector<id<MTLResource>> &MetalBindGroup::IndirectResources( ) const
{
    return m_indirectResources;
}

const std::vector<MetalUpdateDescItem<MetalBuffer>> &MetalBindGroup::Buffers( ) const
{
    return m_buffers;
}

const std::vector<MetalUpdateDescItem<MetalTexture>> &MetalBindGroup::Textures( ) const
{
    return m_textures;
}

const std::vector<MetalUpdateDescItem<MetalSampler>> &MetalBindGroup::Samplers( ) const
{
    return m_samplers;
}

DenOfIz_ResourceBindingSlot MetalBindGroup::GetSlot( uint32_t binding, const DenOfIz_ResourceBindingType &type ) const
{
    return DenOfIz_ResourceBindingSlot{ .Type = type, .Binding = binding, .RegisterSpace = m_bindGroupLayout->RegisterSpace( ) };
}

uint32_t MetalBindGroup::RegisterSpace( ) const
{
    return m_bindGroupLayout->RegisterSpace( );
}
