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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12BufferResource.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12TextureResource.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12ShaderLocalData.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

DX12ShaderLocalData::DX12ShaderLocalData( DX12Context *context, const ShaderLocalDataDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_layout = dynamic_cast<DX12LocalRootSignature *>( m_desc.Layout );
    m_data.resize( m_layout->LocalDataNumBytes( ) );
}

void DX12ShaderLocalData::Begin( )
{
}

void DX12ShaderLocalData::Cbv( const uint32_t binding, IBufferResource *bufferResource )
{
    EncodeBuffer( m_layout->CbvIndex( binding ), bufferResource );
}

void DX12ShaderLocalData::Cbv( const uint32_t binding, const ByteArrayView &data )
{
    const uint32_t numBytes = m_layout->CbvNumBytes( binding );
    if ( data.NumElements > numBytes )
    {
        spdlog::error( "Data larger than expected: [ {} vs {} ] for binding: {} This could lead to data corruption. Binding skipped.", data.NumElements, numBytes, binding );
        return;
    }
    memcpy( m_data.data( ) + m_layout->CbvOffset( binding ), data.Elements, numBytes );
}

void DX12ShaderLocalData::Srv( const uint32_t binding, const IBufferResource *bufferResource )
{
    EncodeBuffer( m_layout->SrvIndex( binding ), bufferResource );
}

void DX12ShaderLocalData::Srv( const uint32_t binding, const ITextureResource *textureResource )
{
    EncodeTexture( m_layout->SrvIndex( binding ), textureResource );
}

void DX12ShaderLocalData::Uav( const uint32_t binding, const IBufferResource *bufferResource )
{
    EncodeBuffer( m_layout->UavIndex( binding ), bufferResource );
}

void DX12ShaderLocalData::Uav( const uint32_t binding, const ITextureResource *textureResource )
{
    EncodeTexture( m_layout->UavIndex( binding ), textureResource );
}

void DX12ShaderLocalData::Sampler( uint32_t binding, const ISampler *sampler )
{
}

void DX12ShaderLocalData::End( )
{
}

uint32_t DX12ShaderLocalData::DataNumBytes( ) const
{
    return m_data.size( );
}

const Byte *DX12ShaderLocalData::Data( ) const
{
    return m_data.data( );
}

void DX12ShaderLocalData::EncodeTexture( const uint32_t index, const ITextureResource *texture )
{
    const auto *dx12TextureResource = dynamic_cast<const DX12TextureResource *>( texture );
    EncodeVA( index, dx12TextureResource->Resource( ) );
}

void DX12ShaderLocalData::EncodeBuffer( const uint32_t index, const IBufferResource *buffer )
{
    const auto *dx12BufferResource = dynamic_cast<const DX12BufferResource *>( buffer );
    EncodeVA( index, dx12BufferResource->Resource( ) );
}

void DX12ShaderLocalData::EncodeVA( const uint32_t index, ID3D12Resource *resource )
{
    const D3D12_GPU_VIRTUAL_ADDRESS &va = resource->GetGPUVirtualAddress( );
    memcpy( m_data.data( ) + index, &va, sizeof( D3D12_GPU_VIRTUAL_ADDRESS ) );
}
