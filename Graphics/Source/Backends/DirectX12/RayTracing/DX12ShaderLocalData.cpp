/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12ShaderLocalData.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Buffer.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Texture.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

DX12ShaderLocalData::DX12ShaderLocalData( DX12Context *context, const DenOfIz_ShaderLocalDataDesc &desc ) : m_context( context ), m_desc( desc )
{
    ILocalRootSignature *layoutInterface = DENOFIZ_FROM_HANDLE( ILocalRootSignature, m_desc.Layout );
    m_layout                             = dynamic_cast<DX12LocalRootSignature *>( layoutInterface );
    m_data.resize( m_layout->LocalDataNumBytes( ) );
}

void DX12ShaderLocalData::Begin( )
{
}

void DX12ShaderLocalData::Cbv( const uint32_t binding, IBuffer *bufferResource )
{
    EncodeBuffer( m_layout->CbvIndex( binding ), bufferResource );
}

void DX12ShaderLocalData::Cbv( const uint32_t binding, const DenOfIz_ByteArrayView &data )
{
    const uint32_t numBytes = m_layout->CbvNumBytes( binding );
    if ( data.NumElements > numBytes )
    {
        spdlog::error( "Data larger than expected: [ {} vs {} ] for binding: {} This could lead to data corruption. Binding skipped.", data.NumElements, numBytes, binding );
        return;
    }
    memcpy( m_data.data( ) + m_layout->CbvOffset( binding ), data.Elements, numBytes );
}

void DX12ShaderLocalData::Srv( const uint32_t binding, const IBuffer *bufferResource )
{
    EncodeBuffer( m_layout->SrvIndex( binding ), bufferResource );
}

void DX12ShaderLocalData::Srv( const uint32_t binding, const ITexture *textureResource )
{
    EncodeTexture( m_layout->SrvIndex( binding ), textureResource );
}

void DX12ShaderLocalData::Uav( const uint32_t binding, const IBuffer *bufferResource )
{
    EncodeBuffer( m_layout->UavIndex( binding ), bufferResource );
}

void DX12ShaderLocalData::Uav( const uint32_t binding, const ITexture *textureResource )
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

void DX12ShaderLocalData::EncodeTexture( const uint32_t index, const ITexture *texture )
{
    const auto *dx12TextureResource = dynamic_cast<const DX12Texture *>( texture );
    EncodeVA( index, dx12TextureResource->Resource( ) );
}

void DX12ShaderLocalData::EncodeBuffer( const uint32_t index, const IBuffer *buffer )
{
    const auto *dx12BufferResource = dynamic_cast<const DX12Buffer *>( buffer );
    EncodeVA( index, dx12BufferResource->Resource( ) );
}

void DX12ShaderLocalData::EncodeVA( const uint32_t index, ID3D12Resource *resource )
{
    const D3D12_GPU_VIRTUAL_ADDRESS &va = resource->GetGPUVirtualAddress( );
    memcpy( m_data.data( ) + index, &va, sizeof( D3D12_GPU_VIRTUAL_ADDRESS ) );
}
