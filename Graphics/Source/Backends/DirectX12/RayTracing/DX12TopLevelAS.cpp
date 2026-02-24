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
#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12TopLevelAS.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12EnumConverter.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12BottomLevelAS.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

DX12TopLevelAS::DX12TopLevelAS( DX12Context *context, const DenOfIz_TopLevelASDesc &desc ) : m_context( context )
{
    m_flags = DenOfIz_DX12EnumConverter_ConvertAccelerationStructureBuildFlags( desc.BuildFlags );

    m_instanceDescs.resize( desc.Instances.NumElements );
    for ( uint32_t i = 0; i < desc.Instances.NumElements; ++i )
    {
        const DenOfIz_ASInstanceDesc &instanceDesc = desc.Instances.Elements[ i ];
        const IBottomLevelAS         *blas         = DENOFIZ_FROM_HANDLE( IBottomLevelAS, instanceDesc.BLAS );
        const DX12BottomLevelAS      *dx12Blas     = dynamic_cast<const DX12BottomLevelAS *>( blas );
        if ( dx12Blas == nullptr )
        {
            spdlog::warn( "Blas is null." );
            continue;
        }

        m_instanceDescs[ i ].AccelerationStructure               = dx12Blas->Buffer( )->Resource( )->GetGPUVirtualAddress( );
        m_instanceDescs[ i ].Flags                               = D3D12_RAYTRACING_INSTANCE_FLAG_NONE;
        m_instanceDescs[ i ].InstanceContributionToHitGroupIndex = instanceDesc.ContributionToHitGroupIndex;
        m_instanceDescs[ i ].InstanceID                          = instanceDesc.ID;
        m_instanceDescs[ i ].InstanceMask                        = instanceDesc.Mask;

        memcpy( m_instanceDescs[ i ].Transform, &instanceDesc.Transform._11, 12 * sizeof( float ) );
    }
    // Calculate required size for the acceleration structure
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS prebuildDesc = { };
    prebuildDesc.DescsLayout                                          = D3D12_ELEMENTS_LAYOUT_ARRAY;
    prebuildDesc.Flags                                                = m_flags;
    prebuildDesc.NumDescs                                             = desc.Instances.NumElements;
    prebuildDesc.Type                                                 = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = { };
    m_context->D3DDevice->GetRaytracingAccelerationStructurePrebuildInfo( &prebuildDesc, &info );

    DenOfIz_BufferDesc instanceDesc = { };
    instanceDesc.HeapType           = DENOFIZ_HEAP_TYPE_CPU_GPU;
    instanceDesc.NumBytes           = desc.Instances.NumElements * sizeof( D3D12_RAYTRACING_INSTANCE_DESC );
    m_instanceBuffer                = std::make_unique<DX12Buffer>( m_context, instanceDesc );
    void *instanceBufferMemory      = m_instanceBuffer->MapMemory( );
    memcpy( instanceBufferMemory, m_instanceDescs.data( ), instanceDesc.NumBytes );
    m_instanceBuffer->UnmapMemory( );

    DenOfIz_BufferDesc bufferDesc = { };
    bufferDesc.Usage              = DENOFIZ_BUFFER_USAGE_STORAGE_BIT | DENOFIZ_BUFFER_USAGE_ACCELERATION_STRUCTURE_BIT;
    bufferDesc.HeapType           = DENOFIZ_HEAP_TYPE_GPU;
    bufferDesc.NumBytes           = info.ResultDataMaxSizeInBytes;
    bufferDesc.DebugName          = DENOFIZ_STRING( "Top Level Acceleration Structure Buffer" );

    m_buffer      = std::make_unique<DX12Buffer>( m_context, bufferDesc );
    m_asSrvHandle = m_context->ShaderVisibleCbvSrvUavDescriptorHeap->AllocateDescriptors( 1 ).Cpu;
    m_buffer->CreateView( m_asSrvHandle, DX12BufferViewType::AccelerationStructure );

    DenOfIz_BufferDesc scratchBufferDesc = { };
    scratchBufferDesc.HeapType           = DENOFIZ_HEAP_TYPE_GPU;
    scratchBufferDesc.NumBytes           = static_cast<UINT>( info.ScratchDataSizeInBytes );
    scratchBufferDesc.Usage              = DENOFIZ_BUFFER_USAGE_STORAGE_BIT;
    scratchBufferDesc.DebugName          = DENOFIZ_STRING( "Top Level Acceleration Structure Scratch Buffer" );
    m_scratch                            = std::make_unique<DX12Buffer>( m_context, scratchBufferDesc );
}

D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS DX12TopLevelAS::Flags( ) const
{
    return m_flags;
}

size_t DX12TopLevelAS::NumInstances( ) const
{
    return m_instanceDescs.size( );
}

const DX12Buffer *DX12TopLevelAS::InstanceBuffer( ) const
{
    return m_instanceBuffer.get( );
}

const DX12Buffer *DX12TopLevelAS::GetDX12Buffer( ) const
{
    return m_buffer.get( );
}

DX12Buffer *DX12TopLevelAS::Buffer( ) const
{
    return m_buffer.get( );
}

const DX12Buffer *DX12TopLevelAS::Scratch( ) const
{
    return m_scratch.get( );
}

void DX12TopLevelAS::UpdateInstanceTransforms( const DenOfIz_UpdateTransformsDesc &desc )
{
    const auto instanceBufferMemory = static_cast<D3D12_RAYTRACING_INSTANCE_DESC *>( m_instanceBuffer->MapMemory( ) );
    for ( uint32_t i = 0; i < desc.Transforms.NumElements; i++ )
    {
        const DenOfIz_Float4x4 &transform = desc.Transforms.Elements[ i ];

        memcpy( instanceBufferMemory[ i ].Transform, &transform._11, 12 * sizeof( float ) );
    }
    m_instanceBuffer->UnmapMemory( );
}

size_t DX12TopLevelAS::BuildNumBytes( ) const
{
    return m_scratch->NumBytes( );
}
