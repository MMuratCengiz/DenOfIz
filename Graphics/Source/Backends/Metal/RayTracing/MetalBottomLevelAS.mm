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

#import "DenOfIzGraphicsInternal/Backends/Metal/RayTracing/MetalBottomLevelAS.h"
#import "DenOfIzGraphicsInternal/Backends/Metal/MetalEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

MetalBottomLevelAS::MetalBottomLevelAS( MetalContext *context, const BottomLevelASDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_geometryDescriptors = [NSMutableArray arrayWithCapacity:desc.Geometries.NumElements( )];
    m_options             = MTLAccelerationStructureInstanceOptionNone;

    for ( size_t i = 0; i < desc.Geometries.NumElements( ); ++i )
    {
        const ASGeometryDesc &geometry = desc.Geometries.GetElement( i );
        m_hitGroupType                 = geometry.Type;
        if ( i > 0 && geometry.Type != m_hitGroupType )
        {
            spdlog::error("All geometries in a BLAS must have the same type in Metal.");
            return;
        }

        switch ( geometry.Type )
        {
        case HitGroupType::Triangles:
            [m_geometryDescriptors addObject:InitializeTriangles( geometry )];
            break;
        case HitGroupType::AABBs:
            [m_geometryDescriptors addObject:InitializeAABBs( geometry )];
            break;
        default:
            spdlog::error("Invalid geometry type: {}", static_cast<int>( geometry.Type ));
            break;
        }

        if ( geometry.Flags & GeometryFlags::Opaque )
        {
            m_options |= MTLAccelerationStructureInstanceOptionOpaque;
        }
    }

    MTLAccelerationStructureUsage usage = MTLAccelerationStructureUsageNone;
    if ( desc.BuildFlags & ASBuildFlags::FastBuild )
    {
        usage = MTLAccelerationStructureUsagePreferFastBuild;
    }
    else if ( desc.BuildFlags & ASBuildFlags::AllowUpdate )
    {
        usage = MTLAccelerationStructureUsageRefit;
    }

    m_descriptor                     = [MTLPrimitiveAccelerationStructureDescriptor descriptor];
    m_descriptor.usage               = usage;
    m_descriptor.geometryDescriptors = m_geometryDescriptors;

    MTLAccelerationStructureSizes asSize = [m_context->Device accelerationStructureSizesWithDescriptor:m_descriptor];

    m_scratch = [m_context->Device newBufferWithLength:asSize.buildScratchBufferSize options:MTLResourceStorageModePrivate];
    [m_scratch setLabel:@"Bottom Level Acceleration Structure Scratch"];
    m_indirectResources.push_back( m_scratch );

    m_accelerationStructure = [context->Device newAccelerationStructureWithSize:asSize.accelerationStructureSize];
    [m_accelerationStructure setLabel:@"Bottom Level Acceleration Structure"];
    m_indirectResources.push_back( m_accelerationStructure );
}

MTLAccelerationStructureTriangleGeometryDescriptor *MetalBottomLevelAS::InitializeTriangles( const ASGeometryDesc &geometry )
{
    const ASGeometryTriangleDesc &triangle = geometry.Triangles;

    MTLAccelerationStructureTriangleGeometryDescriptor *triangleDesc = [MTLAccelerationStructureTriangleGeometryDescriptor descriptor];

    MetalBufferResource *vertexBuffer = (MetalBufferResource *)triangle.VertexBuffer;
    if ( !vertexBuffer || triangle.NumVertices == 0 )
    {
        spdlog::error("Invalid triangle geometry: no vertices or vertex buffer.");
        return nil;
    }

    m_indirectResources.push_back( vertexBuffer->Instance( ) );
    triangleDesc.intersectionFunctionTableOffset = TriangleIntersectionShader;
    triangleDesc.vertexBufferOffset              = triangle.VertexOffset;
    triangleDesc.vertexBuffer                    = vertexBuffer->Instance( );
    triangleDesc.vertexStride                    = triangle.VertexStride;
    triangleDesc.vertexFormat                    = MetalEnumConverter::ConvertFormatToAttributeFormat( triangle.VertexFormat );

    // Overwrite below if the geometry has indices
    triangleDesc.triangleCount = triangle.NumVertices / 3;
    if ( triangle.NumIndices > 0 )
    {
        MetalBufferResource *indexBuffer = (MetalBufferResource *)triangle.IndexBuffer;
        if ( !indexBuffer )
        {
            spdlog::error("Geometry.NumIndices > 0, but Geometry.IndexBuffer == nullptr.");
            return nil;
        }

        triangleDesc.indexBufferOffset = triangle.IndexOffset;
        triangleDesc.indexBuffer       = indexBuffer->Instance( );
        triangleDesc.indexType         = triangle.IndexType == IndexType::Uint16 ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32;
        triangleDesc.triangleCount     = triangle.NumIndices / 3;
        m_indirectResources.push_back( indexBuffer->Instance( ) );
    }

    triangleDesc.opaque                                       = geometry.Flags & GeometryFlags::Opaque;
    triangleDesc.allowDuplicateIntersectionFunctionInvocation = !geometry.Flags & GeometryFlags::NoDuplicateAnyHitInvocation;

    return triangleDesc;
}

MTLAccelerationStructureBoundingBoxGeometryDescriptor *MetalBottomLevelAS::InitializeAABBs( const ASGeometryDesc &geometry )
{
    const ASGeometryAABBDesc                              &aabb     = geometry.AABBs;
    MTLAccelerationStructureBoundingBoxGeometryDescriptor *aabbDesc = [MTLAccelerationStructureBoundingBoxGeometryDescriptor descriptor];

    MetalBufferResource *aabbBuffer = (MetalBufferResource *)aabb.Buffer;
    if ( !aabbBuffer || aabb.NumAABBs == 0 )
    {
        spdlog::error("Invalid AABB geometry: no AABBs or AABB buffer.");
        return nil;
    }

    m_indirectResources.push_back( aabbBuffer->Instance( ) );
    aabbDesc.intersectionFunctionTableOffset = 0;
    aabbDesc.boundingBoxBuffer               = aabbBuffer->Instance( );
    aabbDesc.boundingBoxBufferOffset         = aabb.Offset;
    aabbDesc.boundingBoxStride               = sizeof( MTLAxisAlignedBoundingBox );
    aabbDesc.boundingBoxCount                = aabb.NumAABBs;

    aabbDesc.opaque                                       = geometry.Flags & GeometryFlags::Opaque;
    aabbDesc.allowDuplicateIntersectionFunctionInvocation = !(geometry.Flags & GeometryFlags::NoDuplicateAnyHitInvocation);

    return aabbDesc;
}

id<MTLAccelerationStructure> MetalBottomLevelAS::AccelerationStructure( ) const
{
    return m_accelerationStructure;
}

id<MTLBuffer> MetalBottomLevelAS::Scratch( ) const
{
    return m_scratch;
}

MTLAccelerationStructureDescriptor *MetalBottomLevelAS::Descriptor( )
{
    return m_descriptor;
}

MTLAccelerationStructureInstanceOptions MetalBottomLevelAS::Options( ) const
{
    return m_options;
}

const HitGroupType &MetalBottomLevelAS::GeometryType( ) const
{
    return m_hitGroupType;
}

[[nodiscard]] const std::vector<id<MTLResource>> &MetalBottomLevelAS::IndirectResources( ) const
{
    return m_indirectResources;
}
