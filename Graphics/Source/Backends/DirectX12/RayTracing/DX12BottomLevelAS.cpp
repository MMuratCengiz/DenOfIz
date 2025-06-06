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

DX12BottomLevelAS::DX12BottomLevelAS( DX12Context *context, const BottomLevelASDesc &desc ) : m_context( context )
{
    m_flags                    = DX12EnumConverter::ConvertAccelerationStructureBuildFlags( desc.BuildFlags );
    const size_t numGeometries = desc.Geometries.NumElements( );
    m_geometryDescs.resize( numGeometries );
    for ( uint32_t i = 0; i < numGeometries; ++i )
    {
        const ASGeometryDesc           &geometry     = desc.Geometries.GetElement( i );
        D3D12_RAYTRACING_GEOMETRY_DESC &dx12Geometry = m_geometryDescs[ i ];
        if ( geometry.Flags.IsSet( GeometryFlags::Opaque ) )
        {
            dx12Geometry.Flags |= D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
        }
        if ( geometry.Flags.IsSet( GeometryFlags::NoDuplicateAnyHitInvocation ) )
        {
            dx12Geometry.Flags |= D3D12_RAYTRACING_GEOMETRY_FLAG_NO_DUPLICATE_ANYHIT_INVOCATION;
        }
        switch ( geometry.Type )
        {
        case HitGroupType::Triangles:
            InitializeTriangles( geometry.Triangles, dx12Geometry );
            break;
        case HitGroupType::AABBs:
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

    BufferDesc bufferDesc = { };
    bufferDesc.Descriptor = BitSet( ResourceDescriptor::RWBuffer ) | ResourceDescriptor::AccelerationStructure;
    bufferDesc.HeapType   = HeapType::GPU;
    bufferDesc.NumBytes   = info.ResultDataMaxSizeInBytes;
    bufferDesc.Usages     = ResourceUsage::AccelerationStructureWrite;
    bufferDesc.DebugName  = "Bottom Level Acceleration Structure";
    m_asBuffer            = std::make_unique<DX12BufferResource>( m_context, bufferDesc );

    BufferDesc scratchBufferDesc = { };
    scratchBufferDesc.HeapType   = HeapType::GPU;
    scratchBufferDesc.NumBytes   = static_cast<UINT>( info.ScratchDataSizeInBytes );
    scratchBufferDesc.Descriptor = BitSet( ResourceDescriptor::RWBuffer );
    scratchBufferDesc.Usages     = ResourceUsage::UnorderedAccess;
    scratchBufferDesc.DebugName  = "Bottom Level Acceleration Structure Scratch";
    m_scratch                    = std::make_unique<DX12BufferResource>( m_context, scratchBufferDesc );
}

void DX12BottomLevelAS::InitializeTriangles( const ASGeometryTriangleDesc &triangle, D3D12_RAYTRACING_GEOMETRY_DESC &dx12Geometry ) const
{
    const DX12BufferResource *dx12VertexBuffer = dynamic_cast<DX12BufferResource *>( triangle.VertexBuffer );
    if ( triangle.NumVertices == 0 || dx12VertexBuffer == nullptr )
    {
        spdlog::warn("Geometry has no vertices, or vertex buffer is null.");
        return;
    }

    dx12Geometry.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;

    if ( triangle.NumIndices > 0 )
    {
        const DX12BufferResource *dx12IndexBuffer = dynamic_cast<DX12BufferResource *>( triangle.IndexBuffer );
        if ( dx12IndexBuffer == nullptr )
        {
            spdlog::warn("Geometry.NumIndices > 0, but Geometry.IndexBuffer == nullptr.");
            return;
        }

        dx12Geometry.Triangles.IndexBuffer = dx12IndexBuffer->Resource( )->GetGPUVirtualAddress( ) + triangle.IndexOffset;
        dx12Geometry.Triangles.IndexCount  = triangle.NumIndices;
        dx12Geometry.Triangles.IndexFormat = triangle.IndexType == IndexType::Uint16 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
    }

    dx12Geometry.Triangles.VertexBuffer.StartAddress  = dx12VertexBuffer->Resource( )->GetGPUVirtualAddress( ) + triangle.VertexOffset;
    dx12Geometry.Triangles.VertexBuffer.StrideInBytes = triangle.VertexStride;
    dx12Geometry.Triangles.VertexCount                = triangle.NumVertices;
    dx12Geometry.Triangles.VertexFormat               = DX12EnumConverter::ConvertFormat( triangle.VertexFormat );

    const static std::unordered_set allowedFormats{ Format::R32G32Float,       Format::R32G32B32Float, Format::R16G16Float,
                                                    Format::R16G16B16A16Float, Format::R16G16Snorm,    Format::R16G16B16A16Snorm };
    if ( !allowedFormats.contains( triangle.VertexFormat ) )
    {
        spdlog::warn("Invalid vertex format for acceleration structure geometry.");
    }
}

void DX12BottomLevelAS::InitializeAABBs( const ASGeometryAABBDesc &aabb, D3D12_RAYTRACING_GEOMETRY_DESC &dx12Geometry ) const
{
    const DX12BufferResource *dx12AABBBuffer = dynamic_cast<DX12BufferResource *>( aabb.Buffer );
    if ( aabb.NumAABBs == 0 || dx12AABBBuffer == nullptr )
    {
        spdlog::warn("Geometry has no AABBs, or AABB buffer is null.");
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

DX12BufferResource *DX12BottomLevelAS::Buffer( ) const
{
    return m_asBuffer.get( );
}

const DX12BufferResource *DX12BottomLevelAS::Scratch( ) const
{
    return m_scratch.get( );
}
