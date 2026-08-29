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
    m_indirectResources.clear( );
    m_buffers.clear( );
    m_textures.clear( );
    m_samplers.clear( );
    return this;
}

IBindGroup *MetalBindGroup::Cbv( const uint32_t binding, IBuffer *resource )
{
    const DenOfIz_ResourceBindingSlot slot = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER );
    if ( ValidateSlot( slot ) )
    {
        m_boundBuffers.emplace_back( slot, resource );
    }
    return this;
}

IBindGroup *MetalBindGroup::Cbv( const uint32_t binding, IBuffer *resource, size_t resourceOffset )
{
    const DenOfIz_ResourceBindingSlot slot = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER );
    if ( ValidateSlot( slot ) )
    {
        m_boundBuffersWithOffsets.emplace_back( MetalBufferBindingWithOffset{ slot, resource, resourceOffset } );
    }
    return this;
}

IBindGroup *MetalBindGroup::Srv( const uint32_t binding, IBuffer *resource )
{
    const DenOfIz_ResourceBindingSlot slot = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE );
    if ( ValidateSlot( slot ) )
    {
        m_boundBuffers.emplace_back( slot, resource );
    }
    return this;
}

IBindGroup *MetalBindGroup::Srv( const uint32_t binding, IBuffer *resource, size_t resourceOffset )
{
    const DenOfIz_ResourceBindingSlot slot = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE );
    if ( ValidateSlot( slot ) )
    {
        m_boundBuffersWithOffsets.emplace_back( MetalBufferBindingWithOffset{ slot, resource, resourceOffset } );
    }
    return this;
}

IBindGroup *MetalBindGroup::Srv( const uint32_t binding, ITexture *resource, const uint32_t mipLevel )
{
    const DenOfIz_ResourceBindingSlot slot = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE );
    if ( ValidateSlot( slot ) )
    {
        m_boundTextures.push_back( { slot, mipLevel, resource } );
    }
    return this;
}

IBindGroup *MetalBindGroup::SrvArray( const uint32_t binding, const DenOfIz_TextureArray &resources )
{
    for ( uint32_t i = 0; i < resources.NumElements; ++i )
    {
        auto *metalResource = dynamic_cast<MetalTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, resources.Elements[ i ] ) );
        SrvArrayIndex( binding, i, metalResource );
    }
    return this;
}

IBindGroup *MetalBindGroup::SrvArrayIndex( const uint32_t binding, uint32_t arrayIndex, ITexture *resource )
{
    const DenOfIz_ResourceBindingSlot slot = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE );
    if ( ValidateSlot( slot ) )
    {
        m_boundTextureArrayIndices.emplace_back( MetalTextureArrayIndexBinding{ slot, arrayIndex, resource } );
    }
    return this;
}

IBindGroup *MetalBindGroup::Srv( const uint32_t binding, ITopLevelAS *accelerationStructure )
{
    const DenOfIz_ResourceBindingSlot slot = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE );
    if ( ValidateSlot( slot ) )
    {
        m_boundAccelerationStructures.emplace_back( slot, accelerationStructure );
    }
    return this;
}

IBindGroup *MetalBindGroup::Uav( const uint32_t binding, IBuffer *resource )
{
    const DenOfIz_ResourceBindingSlot slot = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS );
    if ( ValidateSlot( slot ) )
    {
        m_boundBuffers.emplace_back( slot, resource );
    }
    return this;
}

IBindGroup *MetalBindGroup::Uav( const uint32_t binding, IBuffer *resource, size_t resourceOffset )
{
    const DenOfIz_ResourceBindingSlot slot = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS );
    if ( ValidateSlot( slot ) )
    {
        m_boundBuffersWithOffsets.emplace_back( MetalBufferBindingWithOffset{ slot, resource, resourceOffset } );
    }
    return this;
}

IBindGroup *MetalBindGroup::Uav( const uint32_t binding, ITexture *resource, const uint32_t mipLevel )
{
    const DenOfIz_ResourceBindingSlot slot = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS );
    if ( ValidateSlot( slot ) )
    {
        m_boundTextures.push_back( { slot, mipLevel, resource } );
    }
    return this;
}

IBindGroup *MetalBindGroup::Sampler( const uint32_t binding, ISampler *sampler )
{
    const DenOfIz_ResourceBindingSlot slot = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER );
    if ( ValidateSlot( slot ) )
    {
        m_boundSamplers.emplace_back( slot, sampler );
    }
    return this;
}

void MetalBindGroup::EndUpdate( )
{
    @autoreleasepool
    {
        for ( const auto &[ slot, resource ] : m_boundBuffers )
        {
            auto *metalBuffer = static_cast<MetalBuffer *>( resource );
            m_buffers.emplace_back( metalBuffer, m_bindGroupLayout->ShaderStages( slot.Type, slot.Binding ), metalBuffer->Usage( ) );
        }
        for ( const auto &item : m_boundBuffersWithOffsets )
        {
            auto *metalBuffer = static_cast<MetalBuffer *>( item.Resource );
            m_buffers.emplace_back( metalBuffer, m_bindGroupLayout->ShaderStages( item.Slot.Type, item.Slot.Binding ), metalBuffer->Usage( ) );
        }
        for ( const auto &[ slot, accelerationStructure ] : m_boundAccelerationStructures )
        {
            auto *metalAS = static_cast<MetalTopLevelAS *>( accelerationStructure );
            for ( const auto &item : metalAS->IndirectResources( ) )
            {
                m_indirectResources.emplace_back( item );
            }
        }
        for ( const auto &item : m_boundTextures )
        {
            const MTLResourceUsage usage = item.Slot.Type == DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS ? MTLResourceUsageRead | MTLResourceUsageWrite : MTLResourceUsageRead;
            m_textures.emplace_back( static_cast<MetalTexture *>( item.Resource ), m_bindGroupLayout->ShaderStages( item.Slot.Type, item.Slot.Binding ), usage );
        }
        for ( const auto &item : m_boundTextureArrayIndices )
        {
            const MTLResourceUsage usage = item.Slot.Type == DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS ? MTLResourceUsageRead | MTLResourceUsageWrite : MTLResourceUsageRead;
            m_textures.emplace_back( static_cast<MetalTexture *>( item.Resource ), m_bindGroupLayout->ShaderStages( item.Slot.Type, item.Slot.Binding ), usage );
        }
        for ( const auto &[ slot, sampler ] : m_boundSamplers )
        {
            m_samplers.emplace_back( static_cast<MetalSampler *>( sampler ), m_bindGroupLayout->ShaderStages( slot.Type, slot.Binding ), MTLResourceUsageRead );
        }

        for ( const auto &tables : m_tables )
        {
            EncodeTables( *tables );
        }
    }
}

const MetalBindGroupTables *MetalBindGroup::TablesFor( const MetalSpaceLayout &spaceLayout ) const
{
    for ( const auto &tables : m_tables )
    {
        if ( tables->Layout.Hash == spaceLayout.Hash )
        {
            return tables.get( );
        }
    }

    @autoreleasepool
    {
        auto tables    = std::make_unique<MetalBindGroupTables>( );
        tables->Layout = spaceLayout;
        EncodeTables( *tables );
        m_tables.push_back( std::move( tables ) );
    }
    return m_tables.back( ).get( );
}

void MetalBindGroup::EncodeTables( MetalBindGroupTables &tables ) const
{
    const MetalSpaceLayout &layout        = tables.Layout;
    const uint32_t          registerSpace = m_bindGroupLayout->RegisterSpace( );

    if ( layout.CbvSrvUavTableSize > 0 )
    {
        if ( !tables.CbvSrvUavTable )
        {
            tables.CbvSrvUavTable = std::make_unique<DescriptorTable>( m_context, layout.CbvSrvUavTableSize );
            tables.CbvSrvUavTable->SetDebugName( "CbvSrvUav Table[Space: " + std::to_string( registerSpace ) + "]" );
        }
        else
        {
            tables.CbvSrvUavTable->Reset( layout.CbvSrvUavTableSize );
        }
    }
    else
    {
        tables.CbvSrvUavTable.reset( );
    }

    if ( layout.SamplerTableSize > 0 )
    {
        if ( !tables.SamplerTable )
        {
            tables.SamplerTable = std::make_unique<DescriptorTable>( m_context, layout.SamplerTableSize );
            tables.SamplerTable->SetDebugName( "Sampler Table[Space: " + std::to_string( registerSpace ) + "]" );
        }
        else
        {
            tables.SamplerTable->Reset( layout.SamplerTableSize );
        }
    }
    else
    {
        tables.SamplerTable.reset( );
    }

    tables.RootParameters.clear( );

    const auto encodeBuffer = [ & ]( const DenOfIz_ResourceBindingSlot &slot, const id<MTLBuffer> &buffer, const uint32_t offset )
    {
        if ( const uint32_t *rootOffset = layout.RootDescriptorOffset( slot.Type, slot.Binding ) )
        {
            tables.RootParameters.emplace_back( *rootOffset, buffer );
            return;
        }
        if ( const uint32_t *index = layout.TableIndex( slot.Type, slot.Binding ) )
        {
            tables.CbvSrvUavTable->EncodeBuffer( buffer, *index, offset );
        }
    };

    for ( const auto &[ slot, resource ] : m_boundBuffers )
    {
        encodeBuffer( slot, static_cast<MetalBuffer *>( resource )->Instance( ), 0 );
    }
    for ( const auto &item : m_boundBuffersWithOffsets )
    {
        encodeBuffer( item.Slot, static_cast<MetalBuffer *>( item.Resource )->Instance( ), static_cast<uint32_t>( item.Offset ) );
    }
    for ( const auto &[ slot, accelerationStructure ] : m_boundAccelerationStructures )
    {
        auto *metalAS = static_cast<MetalTopLevelAS *>( accelerationStructure );
        if ( const uint32_t *rootOffset = layout.RootDescriptorOffset( slot.Type, slot.Binding ) )
        {
            tables.RootParameters.emplace_back( *rootOffset, metalAS->HeaderBuffer( ) );
            continue;
        }
        if ( const uint32_t *index = layout.TableIndex( slot.Type, slot.Binding ) )
        {
            tables.CbvSrvUavTable->EncodeAccelerationStructure( metalAS->HeaderBuffer( ), *index );
        }
    }
    for ( const auto &item : m_boundTextures )
    {
        if ( const uint32_t *index = layout.TableIndex( item.Slot.Type, item.Slot.Binding ) )
        {
            auto          *metalTexture = static_cast<MetalTexture *>( item.Resource );
            id<MTLTexture> textureView  = item.MipLevel == DZ_ALL_MIP_LEVELS ? metalTexture->Instance( ) : metalTexture->MipView( item.MipLevel );
            tables.CbvSrvUavTable->EncodeTexture( textureView, metalTexture->MinLODClamp( ), *index );
        }
    }
    for ( const auto &item : m_boundTextureArrayIndices )
    {
        if ( const uint32_t *index = layout.TableIndex( item.Slot.Type, item.Slot.Binding ) )
        {
            const uint32_t actualIndex = *index + item.ArrayIndex;
            if ( actualIndex >= tables.CbvSrvUavTable->NumEntries( ) )
            {
                spdlog::error( "Array index {} for binding {} in space {} exceeds the shader's array size.", item.ArrayIndex, item.Slot.Binding, registerSpace );
                continue;
            }
            auto *metalTexture = static_cast<MetalTexture *>( item.Resource );
            tables.CbvSrvUavTable->EncodeTexture( metalTexture->Instance( ), metalTexture->MinLODClamp( ), actualIndex );
        }
    }
    for ( const auto &[ slot, sampler ] : m_boundSamplers )
    {
        if ( const uint32_t *index = layout.TableIndex( slot.Type, slot.Binding ) )
        {
            auto *metalSampler = static_cast<MetalSampler *>( sampler );
            tables.SamplerTable->EncodeSampler( metalSampler->Instance( ), metalSampler->LODBias( ), *index );
        }
    }

    if ( getenv( "DZ_DEBUG_DISPATCH" ) && tables.CbvSrvUavTable )
    {
        static int dumpCount = 0;
        if ( dumpCount++ < 6 )
        {
            id<MTLBuffer>  tableBuffer = tables.CbvSrvUavTable->Buffer( );
            const auto    *entries     = static_cast<const IRDescriptorTableEntry *>( tableBuffer.contents );
            const uint32_t numEntries  = static_cast<uint32_t>( tableBuffer.length / sizeof( IRDescriptorTableEntry ) );
            spdlog::warn( "BindGroup[space {}] table gpuAddr={:#x} entries={}", registerSpace, tableBuffer.gpuAddress, numEntries );
            for ( uint32_t i = 0; i < numEntries; ++i )
            {
                spdlog::warn( "  entry[{}] gpuVA={:#x} texViewID={:#x} metadata={:#x}", i, entries[ i ].gpuVA, entries[ i ].textureViewID, entries[ i ].metadata );
            }
        }
    }
}

bool MetalBindGroup::ValidateSlot( const DenOfIz_ResourceBindingSlot &slot ) const
{
    if ( m_bindGroupLayout->FindBinding( slot.Type, slot.Binding ) == nullptr )
    {
        spdlog::error( "Resource binding with type[ {} ], binding[ {} ], register space[ {} ] is not declared in the bind group layout. Binding skipped.",
                       static_cast<int>( slot.Type ), slot.Binding, slot.RegisterSpace );
        return false;
    }
    return true;
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

MetalBindGroupLayout *MetalBindGroup::BindGroupLayout( ) const
{
    return m_bindGroupLayout;
}

DenOfIz_ResourceBindingSlot MetalBindGroup::GetSlot( uint32_t binding, const DenOfIz_ResourceBindingType &type ) const
{
    return DenOfIz_ResourceBindingSlot{ .Type = type, .Binding = binding, .RegisterSpace = m_bindGroupLayout->RegisterSpace( ) };
}

uint32_t MetalBindGroup::RegisterSpace( ) const
{
    return m_bindGroupLayout->RegisterSpace( );
}
