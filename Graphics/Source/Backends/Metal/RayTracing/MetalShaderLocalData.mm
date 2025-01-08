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

#include <DenOfIzGraphics/Backends/Metal/MetalBufferResource.h>
#include <DenOfIzGraphics/Backends/Metal/MetalTextureResource.h>
#include <DenOfIzGraphics/Backends/Metal/RayTracing/MetalShaderLocalData.h>

using namespace DenOfIz;

MetalShaderLocalData::MetalShaderLocalData( MetalContext *context, const ShaderLocalDataDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_layout = dynamic_cast<MetalLocalRootSignature *>( desc.Layout );
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

void MetalShaderLocalData::Cbv( uint32_t binding, IBufferResource *bufferResource )
{
    auto *metalBuffer = static_cast<MetalBufferResource *>( bufferResource );
    auto  offset      = m_layout->InlineDataOffset( binding );
    auto  numBytes    = m_layout->InlineNumBytes( binding );

    void *srcData = [metalBuffer->Instance( ) contents];
    memcpy( m_data.data( ) + offset, srcData, numBytes );
}

void MetalShaderLocalData::Cbv( uint32_t binding, const InteropArray<Byte> &data )
{
    auto offset   = m_layout->InlineDataOffset( binding );
    auto numBytes = m_layout->InlineNumBytes( binding );
    if ( data.NumElements( ) > numBytes )
    {
        LOG( ERROR ) << "Data larger than expected: [" << data.NumElements( ) << " vs " << numBytes << "] for binding: " << binding
                     << " This could lead to data corruption. Binding skipped.";
        return;
    }

    memcpy( m_data.data( ) + offset, data.Data( ), data.NumElements( ) );
}

void MetalShaderLocalData::Srv( uint32_t binding, const IBufferResource *resource )
{
    EnsureSrvUavTable( );

    auto       *metalBuffer = static_cast<const MetalBufferResource *>( resource );
    const auto &bindingInfo = m_layout->SrvBinding( binding );

    m_srvUavTable->EncodeBuffer( metalBuffer->Instance( ), bindingInfo.DescriptorTableIndex );
}

void MetalShaderLocalData::Srv( uint32_t binding, const ITextureResource *resource )
{
    EnsureSrvUavTable( );

    auto       *metalTexture = static_cast<const MetalTextureResource *>( resource );
    const auto &bindingInfo  = m_layout->SrvBinding( binding );

    m_srvUavTable->EncodeTexture( metalTexture->Instance( ), 0.0f, bindingInfo.DescriptorTableIndex );
}

void MetalShaderLocalData::Uav( uint32_t binding, const IBufferResource *resource )
{
    EnsureSrvUavTable( );
    Srv( binding, resource );
}

void MetalShaderLocalData::Uav( uint32_t binding, const ITextureResource *resource )
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
        LOG( ERROR ) << "There are no SRV or UAV bindings.";
    }
}
void MetalShaderLocalData::EnsureSamplerTable( )
{
    if ( !m_srvUavTable )
    {
        LOG( ERROR ) << "There are no sampler bindings.";
    }
}
