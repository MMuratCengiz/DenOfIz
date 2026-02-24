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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Buffer.h"
#include <utility>
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12EnumConverter.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12Fence.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/StringUtilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

typedef size_t i;
using namespace DenOfIz;

DX12Buffer::DX12Buffer( DX12Context *context, DenOfIz_BufferDesc desc ) :
    m_context( context ), m_desc( std::move( desc ) ), m_debugName( StringViewToStdString( m_desc.DebugName ) ), m_cpuHandles( { } )
{
    if ( !m_debugName.empty( ) )
    {
        m_desc.DebugName = DenOfIz_StringView( m_debugName.c_str( ), static_cast<uint32_t>( m_debugName.size( ) ) );
    }
    else
    {
        m_desc.DebugName = { };
    }

    uint32_t alignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
    if ( m_desc.StructureDesc.Stride > 0 )
    {
        alignment = std::max( alignment, static_cast<uint32_t>( m_desc.StructureDesc.Stride ) );
    }
    m_numBytes = Utilities::Align( m_desc.NumBytes, alignment );

    D3D12_RESOURCE_FLAGS  flags        = DenOfIz_DX12EnumConverter_ConvertBufferUsageToResourceFlags( m_desc.Usage );
    D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON;

    if ( m_desc.Usage & DENOFIZ_BUFFER_USAGE_ACCELERATION_STRUCTURE_BIT )
    {
        initialState = D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE;
    }

    CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer( DX12DescriptorHeap::RoundUp( m_numBytes ), flags );
    resourceDesc.Alignment             = 0;
    resourceDesc.Layout                = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

    UINT64 padded_size = 0;
    context->D3DDevice->GetCopyableFootprints( &resourceDesc, 0, 1, 0, NULL, NULL, NULL, &padded_size );
    m_numBytes = padded_size;

    D3D12MA::ALLOCATION_DESC allocationDesc = { };
    allocationDesc.HeapType                 = DenOfIz_DX12EnumConverter_ConvertHeapType( m_desc.HeapType );

    const HRESULT hr = m_context->DX12MemoryAllocator->CreateResource( &allocationDesc, &resourceDesc, initialState, nullptr, &m_allocation, IID_PPV_ARGS( &m_resource ) );
    DX_CHECK_RESULT( hr );
    if ( !m_debugName.empty( ) )
    {
        const std::wstring name( m_debugName.begin( ), m_debugName.end( ) );
        DX_CHECK_RESULT( m_resource->SetName( name.c_str( ) ) );
        m_allocation->SetName( name.c_str( ) );
    }
}

void DX12Buffer::CreateView( const D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, const DenOfIz_ResourceBindingType type, const uint32_t offset )
{
    switch ( type )
    {
    case DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS:
        CreateView( cpuHandle, DX12BufferViewType::UnorderedAccess, offset );
        break;
    case DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE:
        if ( m_desc.Usage & DENOFIZ_BUFFER_USAGE_ACCELERATION_STRUCTURE_BIT )
        {
            CreateView( cpuHandle, DX12BufferViewType::AccelerationStructure, offset );
        }
        else
        {
            CreateView( cpuHandle, DX12BufferViewType::ShaderResource, offset );
        }
        break;
    case DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER:
        CreateView( cpuHandle, DX12BufferViewType::ConstantBuffer, offset );
        break;
    case DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER:
        break;
    }
}

void DX12Buffer::CreateView( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, DX12BufferViewType type, uint32_t offset )
{
    uint64_t stride          = std::max<uint64_t>( m_desc.StructureDesc.Stride, 1 );
    bool     isStructured    = m_desc.StructureDesc.Stride > 0;
    bool     isRawBuffer     = ( m_desc.Usage & DENOFIZ_BUFFER_USAGE_STORAGE_BIT ) && m_desc.StructureDesc.Stride == 0 && m_desc.Format == DENOFIZ_FORMAT_UNDEFINED;
    bool     isStorageBuffer = ( m_desc.Usage & DENOFIZ_BUFFER_USAGE_STORAGE_BIT ) != 0;

    switch ( type )
    {
    case DX12BufferViewType::ShaderResource:
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC desc = { };
            desc.Format                          = DenOfIz_DX12EnumConverter_ConvertFormat( m_desc.Format );
            desc.ViewDimension                   = D3D12_SRV_DIMENSION_BUFFER;
            desc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            desc.Buffer.Flags                    = D3D12_BUFFER_SRV_FLAG_NONE;
            desc.Buffer.FirstElement             = m_desc.StructureDesc.Offset + offset / stride;

            if ( isStructured )
            {
                desc.Format                     = DXGI_FORMAT_UNKNOWN;
                desc.Buffer.NumElements         = offset > 0 ? m_desc.StructureDesc.NumElements - offset / stride : m_desc.StructureDesc.NumElements;
                desc.Buffer.StructureByteStride = stride;
            }
            else if ( isRawBuffer )
            {
                desc.Format                     = DXGI_FORMAT_R32_TYPELESS;
                desc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_RAW;
                desc.Buffer.NumElements         = offset > 0 ? ( m_numBytes - offset ) / 4 : m_numBytes / 4;
                desc.Buffer.StructureByteStride = 0;
            }
            else if ( isStorageBuffer )
            {
                desc.Format                     = DXGI_FORMAT_UNKNOWN;
                desc.Buffer.NumElements         = offset > 0 ? ( m_numBytes - offset ) / stride : m_numBytes / stride;
                desc.Buffer.StructureByteStride = stride;
            }
            else if ( m_desc.Format == DENOFIZ_FORMAT_UNDEFINED )
            {
                desc.Format                     = DXGI_FORMAT_R32_TYPELESS;
                desc.Buffer.Flags               = D3D12_BUFFER_SRV_FLAG_RAW;
                desc.Buffer.NumElements         = 1;
                desc.Buffer.StructureByteStride = 0;
            }
            m_context->D3DDevice->CreateShaderResourceView( m_resource.get( ), &desc, cpuHandle );
        }
        break;
    case DX12BufferViewType::UnorderedAccess:
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC desc = { };
            desc.Format                           = DenOfIz_DX12EnumConverter_ConvertFormat( m_desc.Format );
            desc.ViewDimension                    = D3D12_UAV_DIMENSION_BUFFER;
            desc.Buffer.FirstElement              = m_desc.StructureDesc.Offset + offset / stride;

            if ( isRawBuffer )
            {
                desc.Format                     = DXGI_FORMAT_R32_TYPELESS;
                desc.Buffer.FirstElement        = offset / 4;
                desc.Buffer.NumElements         = offset > 0 ? ( m_numBytes - offset ) / 4 : m_numBytes / 4;
                desc.Buffer.StructureByteStride = 0;
                desc.Buffer.Flags               = D3D12_BUFFER_UAV_FLAG_RAW;
            }
            else if ( stride != 0 )
            {
                desc.Buffer.NumElements         = offset > 0 ? ( m_numBytes - offset ) / stride : m_numBytes / stride;
                desc.Buffer.StructureByteStride = stride;
                desc.Buffer.Flags               = D3D12_BUFFER_UAV_FLAG_NONE;
            }
            desc.Buffer.CounterOffsetInBytes = 0;
            m_context->D3DDevice->CreateUnorderedAccessView( m_resource.get( ), nullptr, &desc, cpuHandle );
        }
        break;
    case DX12BufferViewType::ConstantBuffer:
        {
            D3D12_CONSTANT_BUFFER_VIEW_DESC desc = { };
            desc.BufferLocation                  = m_allocation->GetResource( )->GetGPUVirtualAddress( ) + offset;
            desc.SizeInBytes                     = DX12DescriptorHeap::RoundUp( offset > 0 ? m_numBytes - offset : m_numBytes );
            m_context->D3DDevice->CreateConstantBufferView( &desc, cpuHandle );
        }
        break;
    case DX12BufferViewType::AccelerationStructure:
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC desc          = { };
            desc.ViewDimension                            = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
            desc.Shader4ComponentMapping                  = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            desc.RaytracingAccelerationStructure.Location = m_allocation->GetResource( )->GetGPUVirtualAddress( );
            m_context->D3DDevice->CreateShaderResourceView( nullptr, &desc, cpuHandle );
        }
        break;
    }
    m_cpuHandles[ static_cast<int>( type ) ] = cpuHandle;
}

void *DX12Buffer::MapMemory( )
{
    DZ_ASSERTM( m_mappedMemory == nullptr, std::format( "Memory already mapped {}", m_debugName ) );
    D3D12_RANGE range = { 0, 0 };
    DX_CHECK_RESULT( m_resource->Map( 0, nullptr, &m_mappedMemory ) );
    return m_mappedMemory;
}

void DX12Buffer::UnmapMemory( )
{
    DZ_ASSERTM( m_mappedMemory != nullptr, std::format( "Memory not mapped, buffer: {}", m_debugName ) );
    m_resource->Unmap( 0, nullptr );
    m_mappedMemory = nullptr;
}

DenOfIz_ByteArray DX12Buffer::GetData( ) const
{
    return { static_cast<Byte *>( m_mappedMemory ), m_numBytes };
}

void DX12Buffer::SetData( const DenOfIz_ByteArrayView &data, const bool keepMapped )
{
    if ( m_mappedMemory == nullptr )
    {
        MapMemory( );
    }

    std::memcpy( m_mappedMemory, data.Elements, data.NumElements );

    if ( !keepMapped )
    {
        UnmapMemory( );
    }
}

void DX12Buffer::WriteData( const DenOfIz_ByteArrayView &data, const uint32_t bufferOffset )
{
    if ( m_mappedMemory == nullptr )
    {
        MapMemory( );
    }

    std::memcpy( static_cast<Byte *>( m_mappedMemory ) + bufferOffset, data.Elements, data.NumElements );
}

DX12Buffer::~DX12Buffer( )
{
    if ( m_mappedMemory != nullptr )
    {
        spdlog::warn( "Memory for buffer: {} not unmapped before lifetime of the buffer.", m_debugName );
        m_resource->Unmap( 0, nullptr );
        m_mappedMemory = nullptr;
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12Buffer::CPUHandle( DX12BufferViewType type ) const
{
    return m_cpuHandles[ static_cast<int>( type ) ];
}

ID3D12Resource2 *DX12Buffer::Resource( ) const
{
    return m_resource.get( );
}

size_t DX12Buffer::NumBytes( ) const
{
    return m_numBytes;
}

const void *DX12Buffer::Data( ) const
{
    return m_data;
}
