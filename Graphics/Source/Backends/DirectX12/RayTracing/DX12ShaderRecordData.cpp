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

#include <DenOfIzGraphics/Backends/DirectX12/DX12BufferResource.h>
#include <DenOfIzGraphics/Backends/DirectX12/DX12TextureResource.h>
#include <DenOfIzGraphics/Backends/DirectX12/RayTracing/DX12ShaderRecordData.h>

using namespace DenOfIz;

DX12ShaderRecordData::DX12ShaderRecordData( DX12Context *context, const ShaderRecordDataDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_layout = dynamic_cast<DX12ShaderRecordLayout *>( m_desc.Layout );
    m_data.resize( m_layout->ShaderRecordNumBytes( ) );
}

void DX12ShaderRecordData::Begin( )
{
}

void DX12ShaderRecordData::Cbv( const uint32_t binding, const IBufferResource *bufferResource )
{
    EncodeBuffer( m_layout->CbvIndex( binding ), bufferResource );
}

void DX12ShaderRecordData::Cbv( const uint32_t binding, const InteropArray<Byte> &data )
{
    const uint32_t numBytes = m_layout->CbvNumBytes( binding );
    if ( numBytes != data.NumElements( ) )
    {
        LOG( ERROR ) << "Bound data is not the same size as the expected data size: CBV(" << binding << "), expected: " << numBytes << ", got: " << data.NumElements( );
    }
    memcpy( m_data.data( ) + m_layout->CbvIndex( binding ), data.Data( ), numBytes );
}

void DX12ShaderRecordData::Srv( const uint32_t binding, const IBufferResource *bufferResource )
{
    EncodeBuffer( m_layout->SrvIndex( binding ), bufferResource );
}

void DX12ShaderRecordData::Srv( const uint32_t binding, const ITextureResource *textureResource )
{
    EncodeTexture( m_layout->SrvIndex( binding ), textureResource );
}

void DX12ShaderRecordData::Uav( const uint32_t binding, const IBufferResource *bufferResource )
{
    EncodeBuffer( m_layout->UavIndex( binding ), bufferResource );
}

void DX12ShaderRecordData::Uav( const uint32_t binding, const ITextureResource *textureResource )
{
    EncodeTexture( m_layout->UavIndex( binding ), textureResource );
}

void DX12ShaderRecordData::Sampler( uint32_t binding, const ISampler *sampler )
{
}

void DX12ShaderRecordData::End( )
{
}

uint32_t DX12ShaderRecordData::DataNumBytes( ) const
{
    return m_data.size( );
}

const Byte *DX12ShaderRecordData::Data( ) const
{
    return m_data.data( );
}

void DX12ShaderRecordData::EncodeTexture( const uint32_t index, const ITextureResource *texture )
{
    const auto *dx12TextureResource = dynamic_cast<const DX12TextureResource *>( texture );
    EncodeVA( index, dx12TextureResource->Resource( ) );
}

void DX12ShaderRecordData::EncodeBuffer( const uint32_t index, const IBufferResource *buffer )
{
    const auto *dx12BufferResource = dynamic_cast<const DX12BufferResource *>( buffer );
    EncodeVA( index, dx12BufferResource->Resource( ) );
}

void DX12ShaderRecordData::EncodeVA( const uint32_t index, ID3D12Resource *resource )
{
    const D3D12_GPU_VIRTUAL_ADDRESS &va = resource->GetGPUVirtualAddress( );
    memcpy( m_data.data( ) + index, &va, sizeof( D3D12_GPU_VIRTUAL_ADDRESS ) );
}
