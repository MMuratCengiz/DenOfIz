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

#import <DenOfIzGraphics/Backends/Metal/RayTracing/MetalBottomLevelAS.h>
#import "DenOfIzGraphics/Backends/Metal/MetalEnumConverter.h"

using namespace DenOfIz;

MetalBottomLevelAS::MetalBottomLevelAS( MetalContext *context, const BottomLevelASDesc &desc ) : m_context( context ), m_desc( desc )
{
    NSMutableArray *geometryDescriptors = [NSMutableArray arrayWithCapacity:desc.Geometries.NumElements( )];

    MTLAccelerationStructureInstanceOptions options = MTLAccelerationStructureInstanceOptionNone;
    for ( size_t i = 0; i < desc.Geometries.NumElements( ); ++i )
    {
        const ASGeometryDesc &geometry = desc.Geometries.GetElement( i );

        switch ( geometry.Type )
        {
        case ASGeometryType::Triangles:
            [geometryDescriptors addObject:InitializeTriangles( geometry.Triangles )];
            break;
        case ASGeometryType::AABBs:
            [geometryDescriptors addObject:InitializeAABBs( geometry.AABBs )];
            break;
        default:
            LOG( ERROR ) << "Invalid geometry type: " << static_cast<int>( geometry.Type );
            break;
        }

        if ( geometry.Flags.IsSet( GeometryFlags::Opaque ) )
        {
            options |= MTLAccelerationStructureInstanceOptionOpaque;
        }
    }

    MTLAccelerationStructureUsage usage = MTLAccelerationStructureUsageNone;
    if ( desc.BuildFlags.IsSet( ASBuildFlags::FastBuild ) )
    {
        usage = MTLAccelerationStructureUsagePreferFastBuild;
    }
    else if ( desc.BuildFlags.IsSet( ASBuildFlags::AllowUpdate ) )
    {
        usage = MTLAccelerationStructureUsageRefit;
    }

    m_geometryDescriptors = [geometryDescriptors copy];

    m_descriptor                     = [MTLPrimitiveAccelerationStructureDescriptor descriptor];
    m_descriptor.usage               = MTLAccelerationStructureUsagePreferFastBuild;
    m_descriptor.geometryDescriptors = m_geometryDescriptors;

    MTLAccelerationStructureSizes asSize = [m_context->Device accelerationStructureSizesWithDescriptor:m_descriptor];

    BufferDesc bufferDesc   = { };
    bufferDesc.Descriptor   = BitSet( ResourceDescriptor::RWBuffer ) | ResourceDescriptor::AccelerationStructure;
    bufferDesc.HeapType     = HeapType::GPU;
    bufferDesc.NumBytes     = asSize.accelerationStructureSize;
    bufferDesc.InitialUsage = ResourceUsage::AccelerationStructureWrite;
    bufferDesc.DebugName    = "Bottom Level Acceleration Structure";
    m_buffer                = std::make_unique<MetalBufferResource>( m_context, bufferDesc );

    BufferDesc scratchBufferDesc   = { };
    scratchBufferDesc.HeapType     = HeapType::GPU;
    scratchBufferDesc.NumBytes     = asSize.buildScratchBufferSize;
    scratchBufferDesc.Descriptor   = BitSet( ResourceDescriptor::RWBuffer );
    scratchBufferDesc.InitialUsage = ResourceUsage::UnorderedAccess;
    scratchBufferDesc.DebugName    = "Bottom Level Acceleration Structure Scratch";
    m_scratch                      = std::make_unique<MetalBufferResource>( m_context, scratchBufferDesc );

    m_accelerationStructure = [context->Device newAccelerationStructureWithSize:asSize.accelerationStructureSize];
}

MTLAccelerationStructureTriangleGeometryDescriptor *MetalBottomLevelAS::InitializeTriangles( const ASGeometryTriangleDesc &triangle )
{
    MTLAccelerationStructureTriangleGeometryDescriptor *triangleDesc = [MTLAccelerationStructureTriangleGeometryDescriptor descriptor];

    MetalBufferResource *vertexBuffer = (MetalBufferResource *)triangle.VertexBuffer;
    if ( !vertexBuffer || triangle.NumVertices == 0 )
    {
        LOG( ERROR ) << "Invalid triangle geometry: no vertices or vertex buffer.";
        return nil;
    }

    triangleDesc.vertexBuffer = vertexBuffer->Instance( );
    triangleDesc.vertexStride = triangle.VertexStride;
    triangleDesc.vertexFormat = MetalEnumConverter::ConvertFormatToAttributeFormat( triangle.VertexFormat );

    // Overwrite below if the geometry has indices
    triangleDesc.triangleCount = triangle.NumVertices / 3;
    if ( triangle.NumIndices > 0 )
    {
        MetalBufferResource *indexBuffer = (MetalBufferResource *)triangle.IndexBuffer;
        if ( !indexBuffer )
        {
            LOG( ERROR ) << "Geometry.NumIndices > 0, but Geometry.IndexBuffer == nullptr.";
            return nil;
        }

        triangleDesc.indexBuffer   = indexBuffer->Instance( );
        triangleDesc.indexType     = triangle.IndexType == IndexType::Uint16 ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32;
        triangleDesc.triangleCount = triangle.NumIndices / 3;
    }

    return triangleDesc;
}

MTLAccelerationStructureBoundingBoxGeometryDescriptor *MetalBottomLevelAS::InitializeAABBs( const ASGeometryAABBDesc &aabb )
{
    MTLAccelerationStructureBoundingBoxGeometryDescriptor *aabbDesc = [MTLAccelerationStructureBoundingBoxGeometryDescriptor descriptor];

    MetalBufferResource *aabbBuffer = (MetalBufferResource *)aabb.Buffer;
    if ( !aabbBuffer || aabb.NumAABBs == 0 )
    {
        LOG( ERROR ) << "Invalid AABB geometry: no AABBs or AABB buffer.";
        return nil;
    }

    aabbDesc.boundingBoxBuffer = aabbBuffer->Instance( );
    aabbDesc.boundingBoxStride = aabb.Stride;
    aabbDesc.boundingBoxCount  = aabb.NumAABBs;
    return aabbDesc;
}

id<MTLAccelerationStructure> MetalBottomLevelAS::AccelerationStructure( ) const
{
    return m_accelerationStructure;
}

MetalBufferResource *MetalBottomLevelAS::MetalBuffer( ) const
{
    return m_buffer.get( );
}

IBufferResource *MetalBottomLevelAS::Buffer( ) const
{
    return m_buffer.get( );
}

MetalBufferResource *MetalBottomLevelAS::Scratch( ) const
{
    return m_scratch.get( );
}

MTLAccelerationStructureDescriptor *MetalBottomLevelAS::Descriptor( )
{
    return m_descriptor;
}

MTLAccelerationStructureInstanceOptions MetalBottomLevelAS::Options( ) const
{
    return m_options;
}
