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

MetalBottomLevelAS::MetalBottomLevelAS( MetalContext *context, const DenOfIz_BottomLevelASDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_geometryDescriptors = [NSMutableArray arrayWithCapacity:desc.Geometries.NumElements];
    m_options             = MTLAccelerationStructureInstanceOptionNone;

    for ( size_t i = 0; i < desc.Geometries.NumElements; ++i )
    {
        const DenOfIz_ASGeometryDesc &geometry = desc.Geometries.Elements[ i ];
        m_hitGroupType                 = geometry.Type;
        if ( i > 0 && geometry.Type != m_hitGroupType )
        {
            spdlog::error("All geometries in a BLAS must have the same type in Metal.");
            return;
        }

        switch ( geometry.Type )
        {
        case DENOFIZ_HIT_GROUP_TYPE_TRIANGLES:
            [m_geometryDescriptors addObject:InitializeTriangles( geometry )];
            break;
        case DENOFIZ_HIT_GROUP_TYPE_AABBS:
            [m_geometryDescriptors addObject:InitializeAABBs( geometry )];
            break;
        default:
            spdlog::error("Invalid geometry type: {}", static_cast<int>( geometry.Type ));
            break;
        }

        if ( geometry.Flags & DENOFIZ_BLAS_GEOMETRY_OPAQUE_BIT )
        {
            m_options |= MTLAccelerationStructureInstanceOptionOpaque;
        }
    }

    MTLAccelerationStructureUsage usage = MTLAccelerationStructureUsageNone;
    if ( desc.BuildFlags & DENOFIZ_AS_BUILD_PREFER_FAST_BUILD_BIT )
    {
        usage = MTLAccelerationStructureUsagePreferFastBuild;
    }
    else if ( desc.BuildFlags & DENOFIZ_AS_BUILD_ALLOW_UPDATE_BIT )
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

MTLAccelerationStructureTriangleGeometryDescriptor *MetalBottomLevelAS::InitializeTriangles( const DenOfIz_ASGeometryDesc &geometry )
{
    const DenOfIz_ASGeometryTriangleDesc &triangle = geometry.Triangles;

    MTLAccelerationStructureTriangleGeometryDescriptor *triangleDesc = [MTLAccelerationStructureTriangleGeometryDescriptor descriptor];

    const IBuffer      *vertexBufferInterface = DENOFIZ_FROM_HANDLE( IBuffer, triangle.VertexBuffer );
    const MetalBuffer  *vertexBuffer          = dynamic_cast<const MetalBuffer *>( vertexBufferInterface );
    if ( !vertexBuffer || triangle.NumVertices == 0 )
    {
        spdlog::error("Invalid triangle geometry: no vertices or vertex buffer.");
        return nil;
    }

    m_indirectResources.push_back( vertexBuffer->Instance( ) );
    triangleDesc.intersectionFunctionTableOffset = DENOFIZ_METAL_TRIANGLE_INTERSECTION_SHADER;
    triangleDesc.vertexBufferOffset              = triangle.VertexOffset;
    triangleDesc.vertexBuffer                    = vertexBuffer->Instance( );
    triangleDesc.vertexStride                    = triangle.VertexStride;
    triangleDesc.vertexFormat                    = DenOfIz_MetalEnumConverter_ConvertFormatToAttributeFormat( triangle.VertexFormat );

    // Overwrite below if the geometry has indices
    triangleDesc.triangleCount = triangle.NumVertices / 3;
    if ( triangle.NumIndices > 0 )
    {
        const IBuffer     *indexBufferInterface = DENOFIZ_FROM_HANDLE( IBuffer, triangle.IndexBuffer );
        const MetalBuffer *indexBuffer          = dynamic_cast<const MetalBuffer *>( indexBufferInterface );
        if ( !indexBuffer )
        {
            spdlog::error("Geometry.NumIndices > 0, but Geometry.IndexBuffer == nullptr.");
            return nil;
        }

        triangleDesc.indexBufferOffset = triangle.IndexOffset;
        triangleDesc.indexBuffer       = indexBuffer->Instance( );
        triangleDesc.indexType         = triangle.IndexType == DENOFIZ_INDEX_TYPE_UINT16 ? MTLIndexTypeUInt16 : MTLIndexTypeUInt32;
        triangleDesc.triangleCount     = triangle.NumIndices / 3;
        m_indirectResources.push_back( indexBuffer->Instance( ) );
    }

    triangleDesc.opaque                                       = geometry.Flags & DENOFIZ_BLAS_GEOMETRY_OPAQUE_BIT;
    triangleDesc.allowDuplicateIntersectionFunctionInvocation = !geometry.Flags & DENOFIZ_BLAS_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT;

    return triangleDesc;
}

MTLAccelerationStructureBoundingBoxGeometryDescriptor *MetalBottomLevelAS::InitializeAABBs( const DenOfIz_ASGeometryDesc &geometry )
{
    const DenOfIz_ASGeometryAABBDesc                              &aabb     = geometry.AABBs;
    MTLAccelerationStructureBoundingBoxGeometryDescriptor *aabbDesc = [MTLAccelerationStructureBoundingBoxGeometryDescriptor descriptor];

    const IBuffer     *aabbBufferInterface = DENOFIZ_FROM_HANDLE( IBuffer, aabb.Buffer );
    const MetalBuffer *aabbBuffer          = dynamic_cast<const MetalBuffer *>( aabbBufferInterface );
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

    aabbDesc.opaque                                       = geometry.Flags & DENOFIZ_BLAS_GEOMETRY_OPAQUE_BIT;
    aabbDesc.allowDuplicateIntersectionFunctionInvocation = !(geometry.Flags & DENOFIZ_BLAS_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT);

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

const DenOfIz_HitGroupType &MetalBottomLevelAS::GeometryType( ) const
{
    return m_hitGroupType;
}

[[nodiscard]] const std::vector<id<MTLResource>> &MetalBottomLevelAS::IndirectResources( ) const
{
    return m_indirectResources;
}

size_t MetalBottomLevelAS::BuildNumBytes( ) const
{
    return m_scratch.length;
}
