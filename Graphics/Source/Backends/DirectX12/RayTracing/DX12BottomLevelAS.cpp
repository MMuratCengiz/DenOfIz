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

#include "DenOfIzGraphicsInternal/Backends/DirectX12/RayTracing/DX12BottomLeveLAS.h"
#include "DenOfIzGraphicsInternal/Backends/DirectX12/DX12EnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

DX12BottomLevelAS::DX12BottomLevelAS( DX12Context *context, const DenOfIz_BottomLevelASDesc &desc ) : m_context( context )
{
    m_flags                    = DenOfIz_DX12EnumConverter_ConvertAccelerationStructureBuildFlags( desc.BuildFlags );
    const size_t numGeometries = desc.Geometries.NumElements;
    m_geometryDescs.resize( numGeometries );
    for ( uint32_t i = 0; i < numGeometries; ++i )
    {
        const DenOfIz_ASGeometryDesc   &geometry     = desc.Geometries.Elements[ i ];
        D3D12_RAYTRACING_GEOMETRY_DESC &dx12Geometry = m_geometryDescs[ i ];
        if ( geometry.Flags & DENOFIZ_BLAS_GEOMETRY_OPAQUE_BIT )
        {
            dx12Geometry.Flags |= D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
        }
        if ( geometry.Flags & DENOFIZ_BLAS_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT )
        {
            dx12Geometry.Flags |= D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION;
        }
        switch ( geometry.Type )
        {
        case DENOFIZ_HIT_GROUP_TYPE_TRIANGLES:
            InitializeTriangles( geometry.Triangles, dx12Geometry );
            break;
        case DENOFIZ_HIT_GROUP_TYPE_AABBS:
            InitializeAABBs( geometry.AABBs, dx12Geometry );
            break;
        }
    }

    D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS prebuildDesc = { };
    prebuildDesc.DescsLayout                                          = D3D12_ELEMENTS_LAYOUT_ARRAY;
    prebuildDesc.Flags                                                = m_flags;
    prebuildDesc.NumDescs                                             = static_cast<UINT>( m_geometryDescs.size( ) );
    prebuildDesc.pGeometryDescs                                       = m_geometryDescs.data( );
    prebuildDesc.Type                                                 = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;

    D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO info = { };
    m_context->D3DDevice->GetRaytracingAccelerationStructurePrebuildInfo( &prebuildDesc, &info );

    DenOfIz_BufferDesc bufferDesc = { };
    bufferDesc.Usage              = DENOFIZ_BUFFER_USAGE_STORAGE_BIT | DENOFIZ_BUFFER_USAGE_ACCELERATION_STRUCTURE_BIT;
    bufferDesc.HeapType           = DENOFIZ_HEAP_TYPE_GPU;
    bufferDesc.NumBytes           = info.ResultDataMaxSizeInBytes;
    bufferDesc.DebugName          = DENOFIZ_STRING( "Bottom Level Acceleration Structure" );
    m_asBuffer                    = std::make_unique<DX12Buffer>( m_context, bufferDesc );

    DenOfIz_BufferDesc scratchBufferDesc = { };
    scratchBufferDesc.HeapType           = DENOFIZ_HEAP_TYPE_GPU;
    scratchBufferDesc.NumBytes           = static_cast<UINT>( info.ScratchDataSizeInBytes );
    scratchBufferDesc.Usage              = DENOFIZ_BUFFER_USAGE_STORAGE_BIT;
    scratchBufferDesc.DebugName          = DENOFIZ_STRING( "Bottom Level Acceleration Structure Scratch" );
    m_scratch                            = std::make_unique<DX12Buffer>( m_context, scratchBufferDesc );
}

void DX12BottomLevelAS::InitializeTriangles( const DenOfIz_ASGeometryTriangleDesc &triangle, D3D12_RAYTRACING_GEOMETRY_DESC &dx12Geometry ) const
{
    const IBuffer    *vertexBuffer     = DENOFIZ_FROM_HANDLE( IBuffer, triangle.VertexBuffer );
    const DX12Buffer *dx12VertexBuffer = dynamic_cast<const DX12Buffer *>( vertexBuffer );
    if ( triangle.NumVertices == 0 || dx12VertexBuffer == nullptr )
    {
        spdlog::warn( "Geometry has no vertices, or vertex buffer is null." );
        return;
    }

    dx12Geometry.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;

    if ( triangle.NumIndices > 0 )
    {
        const IBuffer    *indexBuffer     = DENOFIZ_FROM_HANDLE( IBuffer, triangle.IndexBuffer );
        const DX12Buffer *dx12IndexBuffer = dynamic_cast<const DX12Buffer *>( indexBuffer );
        if ( dx12IndexBuffer == nullptr )
        {
            spdlog::warn( "Geometry.NumIndices > 0, but Geometry.IndexBuffer == nullptr." );
            return;
        }

        dx12Geometry.Triangles.IndexBuffer = dx12IndexBuffer->Resource( )->GetGPUVirtualAddress( ) + triangle.IndexOffset;
        dx12Geometry.Triangles.IndexCount  = triangle.NumIndices;
        dx12Geometry.Triangles.IndexFormat = triangle.IndexType == DENOFIZ_INDEX_TYPE_UINT16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
    }

    dx12Geometry.Triangles.VertexBuffer.StartAddress  = dx12VertexBuffer->Resource( )->GetGPUVirtualAddress( ) + triangle.VertexOffset;
    dx12Geometry.Triangles.VertexBuffer.StrideInBytes = triangle.VertexStride;
    dx12Geometry.Triangles.VertexCount                = triangle.NumVertices;
    dx12Geometry.Triangles.VertexFormat               = DenOfIz_DX12EnumConverter_ConvertFormat( triangle.VertexFormat );

    const static std::unordered_set allowedFormats{ DENOFIZ_FORMAT_R32G32_FLOAT,       DENOFIZ_FORMAT_R32G32B32_FLOAT, DENOFIZ_FORMAT_R16G16_FLOAT,
                                                    DENOFIZ_FORMAT_R16G16B16A16_FLOAT, DENOFIZ_FORMAT_R16G16_SNORM,    DENOFIZ_FORMAT_R16G16B16A16_SNORM };
    if ( !allowedFormats.contains( triangle.VertexFormat ) )
    {
        spdlog::warn( "Invalid vertex format for acceleration structure geometry." );
    }
}

void DX12BottomLevelAS::InitializeAABBs( const DenOfIz_ASGeometryAABBDesc &aabb, D3D12_RAYTRACING_GEOMETRY_DESC &dx12Geometry ) const
{
    const IBuffer    *aabbBuffer     = DENOFIZ_FROM_HANDLE( IBuffer, aabb.Buffer );
    const DX12Buffer *dx12AABBBuffer = dynamic_cast<const DX12Buffer *>( aabbBuffer );
    if ( aabb.NumAABBs == 0 || dx12AABBBuffer == nullptr )
    {
        spdlog::warn( "Geometry has no AABBs, or AABB buffer is null." );
        return;
    }

    dx12Geometry.Type                      = D3D12_RAYTRACING_GEOMETRY_TYPE_PROCEDURAL_PRIMITIVE_AABBS;
    dx12Geometry.AABBs.AABBs.StartAddress  = dx12AABBBuffer->Resource( )->GetGPUVirtualAddress( ) + aabb.Offset;
    dx12Geometry.AABBs.AABBs.StrideInBytes = aabb.Stride;
    dx12Geometry.AABBs.AABBCount           = aabb.NumAABBs;
}

D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAGS DX12BottomLevelAS::Flags( ) const
{
    return m_flags;
}

const std::vector<D3D12_RAYTRACING_GEOMETRY_DESC> &DX12BottomLevelAS::GeometryDescs( ) const
{
    return m_geometryDescs;
}

DX12Buffer *DX12BottomLevelAS::Buffer( ) const
{
    return m_asBuffer.get( );
}

const DX12Buffer *DX12BottomLevelAS::Scratch( ) const
{
    return m_scratch.get( );
}

size_t DX12BottomLevelAS::BuildNumBytes( ) const
{
    return m_scratch->NumBytes( );
}
