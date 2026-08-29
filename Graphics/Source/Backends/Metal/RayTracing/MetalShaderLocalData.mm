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

#include "DenOfIzGraphicsInternal/Backends/Metal/MetalBuffer.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/MetalTexture.h"
#include "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalShaderLocalData.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

MetalShaderLocalData::MetalShaderLocalData( MetalContext *context, const DenOfIz_ShaderLocalDataDesc &desc ) : m_context( context ), m_desc( desc )
{
    const ILocalRootSignature *layoutInterface = DENOFIZ_FROM_HANDLE( ILocalRootSignature, desc.Layout );
    m_layout                                   = dynamic_cast<MetalLocalRootSignature *>( const_cast<ILocalRootSignature *>( layoutInterface ) );
}

void MetalShaderLocalData::Begin( )
{
    m_constants.clear( );
    m_resources.clear( );
    m_usedResources.clear( );
}

void MetalShaderLocalData::Cbv( uint32_t binding, IBuffer *bufferResource )
{
    const DenOfIz_LocalResourceBindingDesc *bindingDesc = m_layout->FindBinding( DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER, binding );
    if ( bindingDesc == nullptr )
    {
        ValidateBinding( DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER, binding );
        return;
    }

    auto *metalBuffer = static_cast<MetalBuffer *>( bufferResource );
    auto  numBytes    = std::min<uint64_t>( bindingDesc->NumBytes, metalBuffer->NumBytes( ) );

    MetalLocalConstantBinding &constant = m_constants.emplace_back( );
    constant.Binding                    = binding;
    constant.Data.resize( numBytes );
    memcpy( constant.Data.data( ), [metalBuffer->Instance( ) contents], numBytes );
}

void MetalShaderLocalData::Cbv( uint32_t binding, const DenOfIz_ByteArrayView &data )
{
    const DenOfIz_LocalResourceBindingDesc *bindingDesc = m_layout->FindBinding( DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER, binding );
    if ( bindingDesc == nullptr )
    {
        ValidateBinding( DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER, binding );
        return;
    }
    if ( data.NumElements > bindingDesc->NumBytes )
    {
        spdlog::error( "Data larger than expected: [ {} vs {} ] for binding: {} This could lead to data corruption. Binding skipped.", data.NumElements, bindingDesc->NumBytes,
                       binding );
        return;
    }

    MetalLocalConstantBinding &constant = m_constants.emplace_back( );
    constant.Binding                    = binding;
    constant.Data.assign( data.Elements, data.Elements + data.NumElements );
}

void MetalShaderLocalData::Srv( uint32_t binding, const IBuffer *resource )
{
    if ( !ValidateBinding( DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE, binding ) )
    {
        return;
    }
    auto *metalBuffer = static_cast<const MetalBuffer *>( resource );
    m_resources.push_back( { DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE, binding, metalBuffer->Instance( ), nil, nil } );
    m_usedResources.push_back( metalBuffer->Instance( ) );
}

void MetalShaderLocalData::Srv( uint32_t binding, const ITexture *resource )
{
    if ( !ValidateBinding( DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE, binding ) )
    {
        return;
    }
    auto *metalTexture = static_cast<const MetalTexture *>( resource );
    m_resources.push_back( { DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE, binding, nil, metalTexture->Instance( ), nil } );
    m_usedResources.push_back( metalTexture->Instance( ) );
}

void MetalShaderLocalData::Uav( uint32_t binding, const IBuffer *resource )
{
    if ( !ValidateBinding( DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS, binding ) )
    {
        return;
    }
    auto *metalBuffer = static_cast<const MetalBuffer *>( resource );
    m_resources.push_back( { DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS, binding, metalBuffer->Instance( ), nil, nil } );
    m_usedResources.push_back( metalBuffer->Instance( ) );
}

void MetalShaderLocalData::Uav( uint32_t binding, const ITexture *resource )
{
    if ( !ValidateBinding( DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS, binding ) )
    {
        return;
    }
    auto *metalTexture = static_cast<const MetalTexture *>( resource );
    m_resources.push_back( { DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS, binding, nil, metalTexture->Instance( ), nil } );
    m_usedResources.push_back( metalTexture->Instance( ) );
}

void MetalShaderLocalData::Sampler( uint32_t binding, const ISampler *sampler )
{
    if ( !ValidateBinding( DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER, binding ) )
    {
        return;
    }
    auto *metalSampler = static_cast<const MetalSampler *>( sampler );
    m_resources.push_back( { DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER, binding, nil, nil, metalSampler->Instance( ) } );
}

void MetalShaderLocalData::End( )
{
    for ( const auto &record : m_records )
    {
        EncodeRecord( *record );
    }
}

const MetalLocalRecord *MetalShaderLocalData::RecordFor( const MetalShaderLayout &layout )
{
    for ( const auto &record : m_records )
    {
        if ( record->LayoutHash == layout.Hash( ) )
        {
            return record.get( );
        }
    }

    auto record           = std::make_unique<MetalLocalRecord>( );
    record->LayoutHash    = layout.Hash( );
    record->NumBytes      = layout.NumBytes( );
    record->RootConstants = layout.RootConstants( );
    for ( const auto &space : layout.Spaces( ) )
    {
        if ( !space )
        {
            continue;
        }
        MetalLocalSpaceTables &tables = record->Spaces.emplace_back( );
        tables.Layout                 = *space;
        if ( space->CbvSrvUavTableSize > 0 )
        {
            tables.CbvSrvUavTable = std::make_unique<DescriptorTable>( m_context, space->CbvSrvUavTableSize );
            tables.CbvSrvUavTable->SetDebugName( "Local CbvSrvUav Table[Space: " + std::to_string( space->RegisterSpace ) + "]" );
        }
        if ( space->SamplerTableSize > 0 )
        {
            tables.SamplerTable = std::make_unique<DescriptorTable>( m_context, space->SamplerTableSize );
            tables.SamplerTable->SetDebugName( "Local Sampler Table[Space: " + std::to_string( space->RegisterSpace ) + "]" );
        }
    }
    EncodeRecord( *record );
    m_records.push_back( std::move( record ) );
    return m_records.back( ).get( );
}

void MetalShaderLocalData::EncodeRecord( MetalLocalRecord &record ) const
{
    record.Data.assign( record.NumBytes, 0 );
    record.UsedResources.clear( );

    for ( const MetalLocalConstantBinding &constant : m_constants )
    {
        for ( const MetalRootConstantLayout &rootConstant : record.RootConstants )
        {
            if ( rootConstant.Binding != constant.Binding )
            {
                continue;
            }
            const size_t numBytes = std::min<size_t>( constant.Data.size( ), rootConstant.NumBytes );
            memcpy( record.Data.data( ) + rootConstant.Offset, constant.Data.data( ), numBytes );
            break;
        }
    }

    for ( MetalLocalSpaceTables &tables : record.Spaces )
    {
        if ( tables.CbvSrvUavTable )
        {
            tables.CbvSrvUavTable->Reset( tables.Layout.CbvSrvUavTableSize );
        }
        if ( tables.SamplerTable )
        {
            tables.SamplerTable->Reset( tables.Layout.SamplerTableSize );
        }
    }

    for ( const MetalLocalResourceBinding &resource : m_resources )
    {
        MetalLocalSpaceTables *tables = nullptr;
        uint32_t               index  = 0;
        for ( MetalLocalSpaceTables &candidate : record.Spaces )
        {
            if ( const uint32_t *tableIndex = candidate.Layout.TableIndex( resource.Type, resource.Binding ) )
            {
                tables = &candidate;
                index  = *tableIndex;
                break;
            }
        }
        if ( tables == nullptr )
        {
            continue;
        }

        if ( resource.Type == DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER )
        {
            tables->SamplerTable->EncodeSampler( resource.Sampler, 0.0f, index );
        }
        else if ( resource.Texture != nil )
        {
            tables->CbvSrvUavTable->EncodeTexture( resource.Texture, 0.0f, index );
        }
        else
        {
            tables->CbvSrvUavTable->EncodeBuffer( resource.Buffer, index );
        }
    }

    for ( const MetalLocalSpaceTables &tables : record.Spaces )
    {
        if ( tables.CbvSrvUavTable && tables.Layout.CbvSrvUavTableOffset != UINT32_MAX )
        {
            const uint64_t address = tables.CbvSrvUavTable->Buffer( ).gpuAddress;
            memcpy( record.Data.data( ) + tables.Layout.CbvSrvUavTableOffset, &address, sizeof( uint64_t ) );
            record.UsedResources.push_back( tables.CbvSrvUavTable->Buffer( ) );
        }
        if ( tables.SamplerTable && tables.Layout.SamplerTableOffset != UINT32_MAX )
        {
            const uint64_t address = tables.SamplerTable->Buffer( ).gpuAddress;
            memcpy( record.Data.data( ) + tables.Layout.SamplerTableOffset, &address, sizeof( uint64_t ) );
            record.UsedResources.push_back( tables.SamplerTable->Buffer( ) );
        }
    }

    for ( const id<MTLResource> &resource : m_usedResources )
    {
        record.UsedResources.push_back( resource );
    }
}

bool MetalShaderLocalData::ValidateBinding( const DenOfIz_ResourceBindingType type, const uint32_t binding ) const
{
    if ( m_layout->FindBinding( type, binding ) == nullptr )
    {
        spdlog::error( "Local resource binding with type[ {} ], binding[ {} ] is not declared in the local root signature. Binding skipped.", static_cast<int>( type ), binding );
        return false;
    }
    return true;
}
