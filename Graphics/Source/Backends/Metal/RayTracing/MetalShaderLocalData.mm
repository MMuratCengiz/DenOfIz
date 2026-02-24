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
    const ILocalRootSignature     *layoutInterface = DENOFIZ_FROM_HANDLE( ILocalRootSignature, desc.Layout );
    m_layout                                       = dynamic_cast<MetalLocalRootSignature *>( const_cast<ILocalRootSignature *>( layoutInterface ) );
    if ( m_layout->NumSrvUavs( ) > 0 )
    {
        m_srvUavTable = std::make_unique<DescriptorTable>( context, m_layout->NumSrvUavs( ) );
    }
    if ( m_layout->NumSamplers( ) > 0 )
    {
        m_samplerTable = std::make_unique<DescriptorTable>( context, m_layout->NumSamplers( ) );
    }

    m_data.resize( m_layout->NumInlineBytes( ) );
}

void MetalShaderLocalData::Begin( )
{
}

void MetalShaderLocalData::Cbv( uint32_t binding, IBuffer *bufferResource )
{
    auto *metalBuffer = static_cast<MetalBuffer *>( bufferResource );
    auto  offset      = m_layout->InlineDataOffset( binding );
    auto  numBytes    = m_layout->InlineNumBytes( binding );

    void *srcData = [metalBuffer->Instance( ) contents];
    memcpy( m_data.data( ) + offset, srcData, numBytes );
}

void MetalShaderLocalData::Cbv( uint32_t binding, const DenOfIz_ByteArrayView &data )
{
    auto offset   = m_layout->InlineDataOffset( binding );
    auto numBytes = m_layout->InlineNumBytes( binding );
    if ( data.NumElements > numBytes )
    {
        spdlog::error("Data larger than expected: [ {} vs {} ] for binding: {} This could lead to data corruption. Binding skipped.", data.NumElements, numBytes, binding);
        return;
    }

    memcpy( m_data.data( ) + offset, data.Elements, data.NumElements );
}

void MetalShaderLocalData::Srv( uint32_t binding, const IBuffer *resource )
{
    EnsureSrvUavTable( );

    auto       *metalBuffer = static_cast<const MetalBuffer *>( resource );
    const auto &bindingInfo = m_layout->SrvBinding( binding );

    m_srvUavTable->EncodeBuffer( metalBuffer->Instance( ), bindingInfo.DescriptorTableIndex );
}

void MetalShaderLocalData::Srv( uint32_t binding, const ITexture *resource )
{
    EnsureSrvUavTable( );

    auto       *metalTexture = static_cast<const MetalTexture *>( resource );
    const auto &bindingInfo  = m_layout->SrvBinding( binding );

    m_srvUavTable->EncodeTexture( metalTexture->Instance( ), 0.0f, bindingInfo.DescriptorTableIndex );
}

void MetalShaderLocalData::Uav( uint32_t binding, const IBuffer *resource )
{
    EnsureSrvUavTable( );
    Srv( binding, resource );
}

void MetalShaderLocalData::Uav( uint32_t binding, const ITexture *resource )
{
    EnsureSrvUavTable( );
    Srv( binding, resource );
}

void MetalShaderLocalData::Sampler( uint32_t binding, const ISampler *sampler )
{
    EnsureSamplerTable( );
    auto       *metalSampler = static_cast<const MetalSampler *>( sampler );
    const auto &bindingInfo  = m_layout->UavBinding( binding );

    m_samplerTable->EncodeSampler( metalSampler->Instance( ), 0.0f, bindingInfo.DescriptorTableIndex );
}

void MetalShaderLocalData::End( )
{
}

const DescriptorTable *MetalShaderLocalData::SrvUavTable( ) const
{
    return m_srvUavTable.get( );
}

const DescriptorTable *MetalShaderLocalData::SamplerTable( ) const
{
    return m_samplerTable.get( );
}

const std::vector<id<MTLResource>> &MetalShaderLocalData::UsedResources( ) const
{
    return m_usedResources;
}

uint32_t MetalShaderLocalData::DataNumBytes( ) const
{
    return m_layout->NumInlineBytes( );
}

const Byte *MetalShaderLocalData::Data( ) const
{
    return m_data.data( );
}

void MetalShaderLocalData::EnsureSrvUavTable( )
{
    if ( !m_srvUavTable )
    {
        spdlog::error("There are no SRV or UAV bindings.");
    }
}
void MetalShaderLocalData::EnsureSamplerTable( )
{
    if ( !m_srvUavTable )
    {
        spdlog::error("There are no sampler bindings.");
    }
}
