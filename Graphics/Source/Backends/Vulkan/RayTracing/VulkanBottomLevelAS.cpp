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

#include "DenOfIzGraphicsInternal/Backends/Vulkan/RayTracing/VulkanBottomLevelAS.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanBuffer.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

VulkanBottomLevelAS::VulkanBottomLevelAS( VulkanContext *context, const DenOfIz_BottomLevelASDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_flags              = DenOfIz_VulkanEnumConverter_ConvertAccelerationStructureBuildFlags( desc.BuildFlags );
    size_t numGeometries = desc.Geometries.NumElements;
    m_geometryDescs.resize( numGeometries );
    m_buildRangeInfos.resize( numGeometries );
    std::vector<uint32_t> maxPrimitives( numGeometries );

    for ( uint32_t i = 0; i < numGeometries; ++i )
    {
        const DenOfIz_ASGeometryDesc      &geometry   = desc.Geometries.Elements[ i ];
        VkAccelerationStructureGeometryKHR vkGeometry = { };
        vkGeometry.sType                              = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        if ( geometry.Flags & DENOFIZ_BLAS_GEOMETRY_OPAQUE_BIT )
        {
            vkGeometry.flags |= VK_GEOMETRY_OPAQUE_BIT_KHR;
        }
        if ( geometry.Flags & DENOFIZ_BLAS_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT )
        {
            vkGeometry.flags |= VK_GEOMETRY_NO_DUPLICATE_ANY_HIT_INVOCATION_BIT_KHR;
        }

        size_t numPrimitives = 0;
        switch ( geometry.Type )
        {
        case DENOFIZ_HIT_GROUP_TYPE_TRIANGLES:
            InitializeTriangles( geometry.Triangles, vkGeometry );
            numPrimitives = geometry.Triangles.NumVertices / 3;
            if ( geometry.Triangles.NumIndices > 0 )
            {
                numPrimitives = geometry.Triangles.NumIndices / 3;
            }
            break;
        case DENOFIZ_HIT_GROUP_TYPE_AABBS:
            InitializeAABBs( geometry.AABBs, vkGeometry );
            numPrimitives = geometry.AABBs.NumAABBs;
            break;
        }

        m_geometryDescs[ i ] = vkGeometry;
        maxPrimitives[ i ]   = numPrimitives;

        VkAccelerationStructureBuildRangeInfoKHR &rangeInfo = m_buildRangeInfos[ i ];
        rangeInfo.primitiveCount                            = numPrimitives;
        rangeInfo.primitiveOffset                           = 0;
        rangeInfo.firstVertex                               = 0;
        rangeInfo.transformOffset                           = 0;
        m_buildRangeInfoPtrs.push_back( &rangeInfo );
    }

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = { };
    buildInfo.sType                                       = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.flags                                       = m_flags;
    buildInfo.type                                        = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    buildInfo.mode                                        = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildInfo.geometryCount                               = static_cast<uint32_t>( m_geometryDescs.size( ) );
    buildInfo.pGeometries                                 = m_geometryDescs.data( );

    VkAccelerationStructureBuildSizesInfoKHR sizeInfo = { };
    sizeInfo.sType                                    = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    vkGetAccelerationStructureBuildSizesKHR( m_context->LogicalDevice, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, maxPrimitives.data( ), &sizeInfo );

    DenOfIz_BufferDesc asBufferDesc = { };
    asBufferDesc.Usage              = DENOFIZ_BUFFER_USAGE_STORAGE_BIT | DENOFIZ_BUFFER_USAGE_ACCELERATION_STRUCTURE_BIT;
    asBufferDesc.NumBytes           = sizeInfo.accelerationStructureSize;
    m_asBuffer                      = std::make_unique<VulkanBuffer>( m_context, asBufferDesc );

    DenOfIz_BufferDesc scratchBufferDesc = { };
    scratchBufferDesc.Usage              = DENOFIZ_BUFFER_USAGE_STORAGE_BIT;
    scratchBufferDesc.NumBytes           = sizeInfo.buildScratchSize;
    m_scratchBuffer                      = std::make_unique<VulkanBuffer>( m_context, scratchBufferDesc );

    // Create the actual acceleration structure
    VkAccelerationStructureCreateInfoKHR accelCreateInfo = { };
    accelCreateInfo.sType                                = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    accelCreateInfo.buffer                               = m_asBuffer->Instance( );
    accelCreateInfo.size                                 = sizeInfo.accelerationStructureSize;
    accelCreateInfo.type                                 = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
    vkCreateAccelerationStructureKHR( m_context->LogicalDevice, &accelCreateInfo, nullptr, &m_accelerationStructure );
}

VulkanBottomLevelAS::~VulkanBottomLevelAS( )
{
    vkDestroyAccelerationStructureKHR( m_context->LogicalDevice, m_accelerationStructure, nullptr );
}

void VulkanBottomLevelAS::InitializeTriangles( const DenOfIz_ASGeometryTriangleDesc &triangle, VkAccelerationStructureGeometryKHR &vkGeometry ) const
{
    vkGeometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;

    const IBuffer      *vertexBufferInterface = DENOFIZ_FROM_HANDLE( IBuffer, triangle.VertexBuffer );
    const VulkanBuffer *vertexBuffer          = dynamic_cast<const VulkanBuffer *>( vertexBufferInterface );
    const IBuffer      *indexBufferInterface  = DENOFIZ_FROM_HANDLE( IBuffer, triangle.IndexBuffer );
    const VulkanBuffer *indexBuffer           = dynamic_cast<const VulkanBuffer *>( indexBufferInterface );

    if ( vertexBuffer == nullptr )
    {
        spdlog::warn( "Geometry has no vertices, or vertex buffer is null." );
        return;
    }
    if ( indexBuffer == nullptr && triangle.NumIndices > 0 )
    {
        spdlog::warn( "Geometry.NumIndices > 0, but Geometry.IndexBuffer == nullptr." );
        return;
    }

    VkAccelerationStructureGeometryTrianglesDataKHR &triangles = vkGeometry.geometry.triangles;
    triangles.sType                                            = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
    triangles.vertexData.deviceAddress                         = vertexBuffer->DeviceAddress( ) + triangle.VertexOffset;
    triangles.vertexStride                                     = triangle.VertexStride;
    triangles.maxVertex                                        = triangle.NumVertices;
    triangles.vertexFormat                                     = DenOfIz_VulkanEnumConverter_ConvertImageFormat( triangle.VertexFormat );

    if ( triangle.NumIndices > 0 )
    {
        triangles.indexData.deviceAddress = indexBuffer->DeviceAddress( ) + triangle.IndexOffset;
        triangles.indexType               = triangle.IndexType == DENOFIZ_INDEX_TYPE_UINT16 ? VK_INDEX_TYPE_UINT16 : VK_INDEX_TYPE_UINT32;
    }

    // Validation
    const static std::unordered_set allowedFormats = { DENOFIZ_FORMAT_R32G32_FLOAT,       DENOFIZ_FORMAT_R32G32B32_FLOAT, DENOFIZ_FORMAT_R16G16_FLOAT,
                                                       DENOFIZ_FORMAT_R16G16B16A16_FLOAT, DENOFIZ_FORMAT_R16G16_SNORM,    DENOFIZ_FORMAT_R16G16B16A16_SNORM };
    if ( !allowedFormats.contains( triangle.VertexFormat ) )
    {
        spdlog::warn( "Invalid vertex format for acceleration structure geometry." );
    }
}

void VulkanBottomLevelAS::InitializeAABBs( const DenOfIz_ASGeometryAABBDesc &aabb, VkAccelerationStructureGeometryKHR &vkGeometry ) const
{
    vkGeometry.geometryType = VK_GEOMETRY_TYPE_AABBS_KHR;

    const IBuffer                               *aabbBufferInterface = DENOFIZ_FROM_HANDLE( IBuffer, aabb.Buffer );
    const VulkanBuffer                          *aabbBuffer          = dynamic_cast<const VulkanBuffer *>( aabbBufferInterface );
    VkAccelerationStructureGeometryAabbsDataKHR &aabbs               = vkGeometry.geometry.aabbs;
    aabbs.sType                                                      = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_AABBS_DATA_KHR;
    aabbs.data.deviceAddress                                         = aabbBuffer->DeviceAddress( ) + aabb.Offset;
    aabbs.stride                                                     = aabb.Stride;
}

const VkAccelerationStructureKHR &VulkanBottomLevelAS::Instance( ) const
{
    return m_accelerationStructure;
}

const std::vector<VkAccelerationStructureGeometryKHR> &VulkanBottomLevelAS::GeometryDescs( ) const
{
    return m_geometryDescs;
}

const VkAccelerationStructureBuildRangeInfoKHR *const *VulkanBottomLevelAS::BuildRangeInfos( ) const
{
    return m_buildRangeInfoPtrs.data( );
}

const VkBuildAccelerationStructureFlagsKHR &VulkanBottomLevelAS::Flags( ) const
{
    return m_flags;
}

const VulkanBuffer *VulkanBottomLevelAS::ScratchBuffer( ) const
{
    return m_scratchBuffer.get( );
}

uint64_t VulkanBottomLevelAS::DeviceAddress( ) const
{
    return m_asBuffer->DeviceAddress( );
}

size_t VulkanBottomLevelAS::BuildNumBytes( ) const
{
    return m_scratchBuffer->NumBytes( );
}
