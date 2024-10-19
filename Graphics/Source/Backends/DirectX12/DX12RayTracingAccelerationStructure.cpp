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

#include <DenOfIzGraphics/Backends/DirectX12/DX12RayTracingAccelerationStructure.h>

using namespace DenOfIz;

DX12RayTracingAccelerationStructure::DX12RayTracingAccelerationStructure( DX12Context *context ) : m_context( context )
{
}

void DX12RayTracingAccelerationStructure::Build( const AccelerationStructureDesc &desc )
{
}
void DX12RayTracingAccelerationStructure::Update( const AccelerationStructureDesc &desc )
{
}

void DX12RayTracingAccelerationStructure::BuildTopLevel( const AccelerationStructureTopLevelDesc &desc )
{
    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS prebuildDesc = { };
    prebuildDesc.DescsLayout                                          = D3D12_ELEMENTS_LAYOUT_ARRAY;
    prebuildDesc.Flags                                                = DX12EnumConverter::ConvertAccelerationStructureBuildFlags( desc.BuildFlags );
    prebuildDesc.NumDescs                                             = desc.Instances.NumElements( );
    prebuildDesc.pGeometryDescs                                       = NULL;
    prebuildDesc.Type                                                 = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = { };
    m_context->D3DDevice->GetRaytracingAccelerationStructurePrebuildInfo( &prebuildDesc, &info );

    std::vector<D3D12_RAYTRACING_INSTANCE_DESC> instanceDescs( desc.Instances.NumElements( ) );
    for ( uint32_t i = 0; i < desc.Instances.NumElements( ); ++i )
    {
        const AccelerationStructureInstanceDesc &instanceDesc       = desc.Instances.GetElement( i );
        DX12BufferResource                      *dx12InstanceBuffer = dynamic_cast<DX12BufferResource *>( instanceDesc.Buffer );
        if ( dx12InstanceBuffer == nullptr )
        {
            LOG( WARNING ) << "Instance buffer is null.";
            continue;
        }

        instanceDescs[ i ].AccelerationStructure               = dx12InstanceBuffer->GetResource( )->GetGPUVirtualAddress( );
        instanceDescs[ i ].Flags                               = D3D12_RAYTRACING_INSTANCE_FLAG_NONE; // todo
        instanceDescs[ i ].InstanceContributionToHitGroupIndex = instanceDesc.ContributionToHitGroupIndex;
        instanceDescs[ i ].InstanceID                          = instanceDesc.ID;
        instanceDescs[ i ].InstanceMask                        = instanceDesc.Mask;

        memcpy( instanceDescs[ i ].Transform, instanceDesc.Transform, sizeof( float[ 12 ] ) ); //-V595
    }

    BufferDesc instanceDesc    = { };
    instanceDesc.HeapType      = HeapType::CPU_GPU;
    instanceDesc.NumBytes      = desc.Instances.NumElements( ) * sizeof( instanceDescs[ 0 ] );
    m_instanceBuffer           = std::make_unique<DX12BufferResource>( m_context, instanceDesc );
    void *instanceBufferMemory = m_instanceBuffer->MapMemory( );
    memcpy( instanceBufferMemory, instanceDescs.data( ), instanceDesc.NumBytes );

    BufferDesc bufferDesc   = { };
    bufferDesc.Descriptor   = BitSet<ResourceDescriptor>( ResourceDescriptor::RWBuffer ) | ResourceDescriptor::AccelerationStructure;
    bufferDesc.HeapType     = HeapType::GPU;
    bufferDesc.NumBytes     = info.ResultDataMaxSizeInBytes;
    bufferDesc.InitialState = ResourceState::AccelerationStructureWrite;
    bufferDesc.DebugName    = "Top Level Acceleration Structure Buffer";

    m_tlasBuffer  = std::make_unique<DX12BufferResource>( m_context, bufferDesc );
    m_asSrvHandle = m_context->ShaderVisibleCbvSrvUavDescriptorHeap->GetNextHandle( ).Cpu;
    m_tlasBuffer->CreateDefaultView( m_asSrvHandle );

    BufferDesc scratchBufferDesc   = { };
    scratchBufferDesc.HeapType     = HeapType::GPU;
    scratchBufferDesc.NumBytes     = (UINT)info.ScratchDataSizeInBytes;
    scratchBufferDesc.Descriptor   = BitSet<ResourceDescriptor>( ResourceDescriptor::RWBuffer );
    scratchBufferDesc.InitialState = ResourceState::AccelerationStructureWrite;
    m_tlasScratch                  = std::make_unique<DX12BufferResource>( m_context, scratchBufferDesc );
}

void DX12RayTracingAccelerationStructure::BuildBottomLevel( const AccelerationStructureBottomLevelDesc &desc )
{
    m_geometryDescs.resize( desc.Geometries.NumElements( ) );
    for ( uint32_t i = 0; i < desc.Geometries.NumElements( ); ++i )
    {
        const AccelerationStructureGeometryDesc &geometry = desc.Geometries.GetElement( i );

        DX12BufferResource *dx12VertexBuffer = dynamic_cast<DX12BufferResource *>( geometry.VertexBuffer );
        if ( geometry.NumVertices == 0 || dx12VertexBuffer == nullptr )
        {
            LOG( WARNING ) << "Geometry has no vertices, or vertex buffer is null.";
            continue;
        }

        D3D12_RAYTRACING_GEOMETRY_DESC &dx12Geometry = m_geometryDescs[ i ];
        dx12Geometry.Flags                           = D3D12_RAYTRACING_GEOMETRY_FLAG_NONE; // todo

        if ( geometry.NumIndices )
        {
            DX12BufferResource *dx12IndexBuffer = dynamic_cast<DX12BufferResource *>( geometry.IndexBuffer );
            if ( dx12IndexBuffer == nullptr )
            {
                LOG( WARNING ) << "Geometry has indices but index buffer is null.";
                continue;
            }

            dx12Geometry.Triangles.IndexBuffer = dx12IndexBuffer->GetResource( )->GetGPUVirtualAddress( ) + geometry.IndexOffset;
            dx12Geometry.Triangles.IndexCount  = geometry.NumIndices;
            dx12Geometry.Triangles.IndexFormat = geometry.IndexType == IndexType::Uint16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        }

        dx12Geometry.Triangles.VertexBuffer.StartAddress  = dx12VertexBuffer->GetResource( )->GetGPUVirtualAddress( ) + geometry.VertexOffset;
        dx12Geometry.Triangles.VertexBuffer.StrideInBytes = geometry.VertexStride;
        dx12Geometry.Triangles.VertexCount                = geometry.NumVertices;
        dx12Geometry.Triangles.VertexFormat               = DX12EnumConverter::ConvertFormat( geometry.VertexFormat );

        const static std::unordered_set<Format> allowedFormats{ Format::R32G32Float,       Format::R32G32B32Float, Format::R16G16Float,
                                                                Format::R16G16B16A16Float, Format::R16G16Snorm,    Format::R16G16B16A16Snorm };
        if ( !allowedFormats.contains( geometry.VertexFormat ) )
        {
            LOG( WARNING ) << "Invalid vertex format for acceleration structure geometry.";
        }
    }

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS prebuildDesc = { };
    prebuildDesc.DescsLayout                                          = D3D12_ELEMENTS_LAYOUT_ARRAY;
    prebuildDesc.Flags                                                = DX12EnumConverter::ConvertAccelerationStructureBuildFlags( desc.BuildFlags );
    prebuildDesc.NumDescs                                             = (UINT)m_geometryDescs.size( );
    prebuildDesc.pGeometryDescs                                       = m_geometryDescs.data( );
    prebuildDesc.Type                                                 = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = { };
    m_context->D3DDevice->GetRaytracingAccelerationStructurePrebuildInfo( &prebuildDesc, &info );

    BufferDesc bufferDesc   = { };
    bufferDesc.Descriptor   = BitSet<ResourceDescriptor>( ResourceDescriptor::RWBuffer ) | ResourceDescriptor::AccelerationStructure;
    bufferDesc.HeapType     = HeapType::GPU;
    bufferDesc.NumBytes     = info.ResultDataMaxSizeInBytes;
    bufferDesc.InitialState = ResourceState::AccelerationStructureWrite;
    m_blasBuffer            = std::make_unique<DX12BufferResource>( m_context, bufferDesc );

    BufferDesc scratchBufferDesc   = { };
    scratchBufferDesc.HeapType     = HeapType::GPU;
    scratchBufferDesc.NumBytes     = (UINT)info.ScratchDataSizeInBytes;
    scratchBufferDesc.Descriptor   = BitSet<ResourceDescriptor>( ResourceDescriptor::RWBuffer );
    scratchBufferDesc.InitialState = ResourceState::AccelerationStructureWrite;
    m_blasScratch                  = std::make_unique<DX12BufferResource>( m_context, scratchBufferDesc );
}
