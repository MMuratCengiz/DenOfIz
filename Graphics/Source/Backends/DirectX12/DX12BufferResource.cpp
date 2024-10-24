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

#include <utility>
#include "DenOfIzGraphics/Backends/DirectX12/DX12Fence.h"
#include "directxtk12/BufferHelpers.h"

typedef size_t i;
using namespace DenOfIz;

DX12BufferResource::DX12BufferResource( DX12Context *context, BufferDesc desc ) : m_context( context ), m_desc( std::move( desc ) ), m_cpuHandle( )
{
    m_stride                   = FormatNumBytes( m_desc.Format );
    m_numBytes                 = Utilities::Align( m_desc.NumBytes, std::max<uint32_t>( m_desc.Alignment, D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT ) );
    D3D12_RESOURCE_FLAGS flags = D3D12_RESOURCE_FLAG_NONE;

    if ( m_desc.Descriptor.IsSet( ResourceDescriptor::RWBuffer ) )
    {
        flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
    }
    if ( m_desc.Descriptor.IsSet( ResourceDescriptor::AccelerationStructure ) )
    {
        flags |= D3D12_RESOURCE_FLAG_RAYTRACING_ACCELERATION_STRUCTURE;
    }

    const CD3DX12_RESOURCE_DESC resourceDesc = CD3DX12_RESOURCE_DESC::Buffer( DX12DescriptorHeap::RoundUp( m_numBytes ), flags );

    D3D12MA::ALLOCATION_DESC allocationDesc = { };
    allocationDesc.HeapType                 = DX12EnumConverter::ConvertHeapType( m_desc.HeapType );

    const D3D12_RESOURCE_STATES start_state = DX12EnumConverter::ConvertResourceState( m_desc.InitialState );

    const HRESULT hr = m_context->DX12MemoryAllocator->CreateResource( &allocationDesc, &resourceDesc, start_state, nullptr, &m_allocation, IID_PPV_ARGS( &m_resource ) );
    DX_CHECK_RESULT( hr );
    std::string        debugName = m_desc.DebugName.Get( );
    const std::wstring name      = std::wstring( debugName.begin( ), debugName.end( ) );
    DX_CHECK_RESULT( m_resource->SetName( name.c_str( ) ) );
    m_allocation->SetName( name.c_str( ) );
}

void DX12BufferResource::CreateDefaultView( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle )
{
    DZ_RETURN_IF( m_cpuHandle.ptr != 0 && m_cpuHandle.ptr == cpuHandle.ptr );
    if ( m_desc.Descriptor.None( ) )
    {
        LOG( WARNING ) << "Unable to create buffer view for buffer without a descriptor.";
        return;
    }

    // The views here are set explicitly in CommandList::BindVertexBuffer and CommandList::BindIndexBuffer
    DZ_RETURN_IF( m_desc.Descriptor.Any( { ResourceDescriptor::VertexBuffer, ResourceDescriptor::IndexBuffer } ) );
    m_cpuHandle = cpuHandle;
    if ( m_desc.Descriptor.IsSet( ResourceDescriptor::UniformBuffer ) )
    {
        CreateView( cpuHandle, DX12BufferViewType::ConstantBuffer );
    }
    else if ( m_desc.Descriptor.IsSet( ResourceDescriptor::AccelerationStructure ) )
    {
        CreateView( cpuHandle, DX12BufferViewType::AccelerationStructure );
    }
    else
    {
        if ( m_desc.Descriptor.IsSet( ResourceDescriptor::RWBuffer ) )
        {
            CreateView( cpuHandle, DX12BufferViewType::UnorderedAccess );
        }
        else
        {
            CreateView( cpuHandle, DX12BufferViewType::ShaderResource );
        }
    }
}

void DX12BufferResource::CreateView( D3D12_CPU_DESCRIPTOR_HANDLE cpuHandle, DX12BufferViewType type )
{
    uint64_t stride = m_desc.BufferView.Stride;

    switch ( type )
    {

    case DX12BufferViewType::ShaderResource:
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC desc = { };
            desc.Format                          = DX12EnumConverter::ConvertFormat( m_desc.Format );
            desc.ViewDimension                   = D3D12_SRV_DIMENSION_BUFFER;
            desc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            desc.Buffer.FirstElement             = m_desc.BufferView.Offset;
            if ( stride != 0 )
            {
                desc.Buffer.NumElements         = m_numBytes / stride;
                desc.Buffer.StructureByteStride = stride;
            }
            desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
            m_context->D3DDevice->CreateShaderResourceView( m_resource.get( ), &desc, cpuHandle );
        }
        break;
    case DX12BufferViewType::UnorderedAccess:
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC desc = { };
            desc.Format                           = DX12EnumConverter::ConvertFormat( m_desc.Format );
            desc.ViewDimension                    = D3D12_UAV_DIMENSION_BUFFER;
            desc.Buffer.FirstElement              = m_desc.BufferView.Offset;
            if ( stride != 0 )
            {
                desc.Buffer.NumElements         = m_numBytes / stride;
                desc.Buffer.StructureByteStride = stride;
            }
            desc.Buffer.CounterOffsetInBytes = 0;
            desc.Buffer.Flags                = D3D12_BUFFER_UAV_FLAG_NONE;
            m_context->D3DDevice->CreateUnorderedAccessView( m_resource.get( ), nullptr, &desc, cpuHandle );
        }
        break;
    case DX12BufferViewType::ConstantBuffer:
        {
            D3D12_CONSTANT_BUFFER_VIEW_DESC desc = { };
            desc.BufferLocation                  = m_allocation->GetResource( )->GetGPUVirtualAddress( );
            desc.SizeInBytes                     = DX12DescriptorHeap::RoundUp( m_numBytes );
            m_context->D3DDevice->CreateConstantBufferView( &desc, cpuHandle );
        }
        break;
    case DX12BufferViewType::AccelerationStructure:
        {
            D3D12_SHADER_RESOURCE_VIEW_DESC desc          = { };
            desc.ViewDimension                            = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
            desc.Shader4ComponentMapping                  = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            desc.RaytracingAccelerationStructure.Location = m_allocation->GetResource( )->GetGPUVirtualAddress( );
            m_context->D3DDevice->CreateShaderResourceView( nullptr, &desc, m_cpuHandle );
        }
        break;
    }
    m_cpuHandles[ static_cast<int>( type ) ] = cpuHandle;
}

void *DX12BufferResource::MapMemory( )
{
    DZ_ASSERTM( m_mappedMemory == nullptr, std::format( "Memory already mapped {}", m_desc.DebugName.Get( ) ) );
    D3D12_RANGE range = { 0, 0 };
    DX_CHECK_RESULT( m_resource->Map( 0, nullptr, &m_mappedMemory ) );
    return m_mappedMemory;
}

void DX12BufferResource::UnmapMemory( )
{
    DZ_ASSERTM( m_mappedMemory != nullptr, std::format( "Memory not mapped, buffer: {}", m_desc.DebugName.Get( ) ) );
    m_resource->Unmap( 0, nullptr );
    m_mappedMemory = nullptr;
}

InteropArray<Byte> DX12BufferResource::GetData( ) const
{
    InteropArray<Byte> data( m_numBytes );
    std::memcpy( data.Data( ), m_mappedMemory, m_numBytes );
    return std::move( data );
}

void DX12BufferResource::SetData( const InteropArray<Byte> &data, bool keepMapped )
{
    if ( m_mappedMemory == nullptr )
    {
        MapMemory( );
    }

    std::memcpy( m_mappedMemory, data.Data( ), data.NumElements( ) );

    if ( !keepMapped )
    {
        UnmapMemory( );
    }
}

DX12BufferResource::~DX12BufferResource( )
{
    if ( m_mappedMemory != nullptr )
    {
        LOG( WARNING ) << "Memory for buffer: " << m_desc.DebugName.Get( ) << " not unmapped before lifetime of the buffer.";
        m_resource->Unmap( 0, nullptr );
        m_mappedMemory = nullptr;
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12BufferResource::CPUHandle( DX12BufferViewType type ) const
{
    return m_cpuHandles[ static_cast<int>( type ) ];
}

ID3D12Resource2 *DX12BufferResource::Resource( ) const
{
    return m_resource.get( );
}

BitSet<ResourceState> DX12BufferResource::InitialState( ) const
{
    return m_state;
}
i DX12BufferResource::NumBytes( ) const
{
    return m_numBytes;
}
const void *DX12BufferResource::Data( ) const
{
    return m_data;
}
