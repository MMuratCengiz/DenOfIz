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

#include <DenOfIzGraphics/Backends/Vulkan/RayTracing/VulkanTopLevelAS.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanEnumConverter.h>

using namespace DenOfIz;

VulkanTopLevelAS::VulkanTopLevelAS( VulkanContext *context, const TopLevelASDesc &desc ) : m_context( context )
{
    m_flags = VulkanEnumConverter::ConvertAccelerationStructureBuildFlags( desc.BuildFlags );

    // Prepare instance descriptions
    m_instances.resize( desc.Instances.NumElements( ) );
    for ( uint32_t i = 0; i < desc.Instances.NumElements( ); ++i )
    {
        const ASInstanceDesc &instanceDesc = desc.Instances.GetElement( i );
        auto                 *vkBLASBuffer = dynamic_cast<VulkanBufferResource *>( instanceDesc.BLASBuffer );
        if ( vkBLASBuffer == nullptr )
        {
            LOG( WARNING ) << "BLAS buffer is null.";
            continue;
        }

        VkAccelerationStructureInstanceKHR &vkInstance = m_instances[ i ];
        memcpy( vkInstance.transform.matrix, instanceDesc.Transform.Data( ), 12 * sizeof( float ) );
        vkInstance.instanceCustomIndex                    = instanceDesc.ID;
        vkInstance.mask                                   = instanceDesc.Mask;
        vkInstance.instanceShaderBindingTableRecordOffset = instanceDesc.ContributionToHitGroupIndex;
        vkInstance.accelerationStructureReference         = vkBLASBuffer->DeviceAddress( );
        vkInstance.flags                                  = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR;
    }

    m_buildRangeInfo.primitiveCount  = desc.Instances.NumElements( );
    m_buildRangeInfo.primitiveOffset = 0;
    m_buildRangeInfo.firstVertex     = 0;
    m_buildRangeInfo.transformOffset = 0;
    m_buildRangeInfoPtr[ 0 ]         = &m_buildRangeInfo;

    BufferDesc instanceBufferDesc = { };
    instanceBufferDesc.NumBytes   = desc.Instances.NumElements( ) * sizeof( VkAccelerationStructureInstanceKHR );
    instanceBufferDesc.Descriptor = BitSet( ResourceDescriptor::RWBuffer );
    instanceBufferDesc.Usages     = ResourceUsage::AccelerationStructureGeometry;
    instanceBufferDesc.HeapType   = HeapType::CPU_GPU;
    m_instanceBuffer              = std::make_unique<VulkanBufferResource>( m_context, instanceBufferDesc );

    VkAccelerationStructureGeometryKHR geometry    = { };
    geometry.sType                                 = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    geometry.geometryType                          = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    geometry.geometry.instances.sType              = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    geometry.geometry.instances.arrayOfPointers    = VK_FALSE;
    geometry.geometry.instances.data.deviceAddress = m_instanceBuffer->DeviceAddress( );

    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = { };
    buildInfo.sType                                       = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.type                                        = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    buildInfo.flags                                       = m_flags;
    buildInfo.mode                                        = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
    buildInfo.geometryCount                               = 1;
    buildInfo.pGeometries                                 = &geometry;

    VkAccelerationStructureBuildSizesInfoKHR sizeInfo = { };
    sizeInfo.sType                                    = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    uint32_t numElements                              = desc.Instances.NumElements( );
    vkGetAccelerationStructureBuildSizesKHR( m_context->LogicalDevice, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &numElements, &sizeInfo );

    void *instanceBufferMemory = m_instanceBuffer->MapMemory( );
    memcpy( instanceBufferMemory, m_instances.data( ), instanceBufferDesc.NumBytes );
    m_instanceBuffer->UnmapMemory( );

    // Create acceleration structure buffer
    BufferDesc bufferDesc = { };
    bufferDesc.NumBytes   = sizeInfo.accelerationStructureSize;
    bufferDesc.Descriptor = BitSet( ResourceDescriptor::RWBuffer ) | ResourceDescriptor::AccelerationStructure;
    bufferDesc.HeapType   = HeapType::GPU;
    m_buffer              = std::make_unique<VulkanBufferResource>( m_context, bufferDesc );

    // Initialize scratch buffer
    BufferDesc scratchBufferDesc   = { };
    scratchBufferDesc.NumBytes     = sizeInfo.buildScratchSize;
    scratchBufferDesc.InitialUsage = ResourceUsage::UnorderedAccess;
    scratchBufferDesc.Descriptor   = ResourceDescriptor::RWBuffer;
    scratchBufferDesc.HeapType     = HeapType::GPU;
    m_scratch                      = std::make_unique<VulkanBufferResource>( m_context, scratchBufferDesc );

    // Create acceleration structure
    VkAccelerationStructureCreateInfoKHR createInfo = { };
    createInfo.sType                                = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
    createInfo.buffer                               = m_buffer->Instance( );
    createInfo.size                                 = sizeInfo.accelerationStructureSize;
    createInfo.type                                 = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    vkCreateAccelerationStructureKHR( m_context->LogicalDevice, &createInfo, nullptr, &m_accelerationStructure );

    m_buildGeometryInfo                                       = { };
    m_buildGeometryInfo.sType                                 = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
    m_buildGeometryInfo.geometryType                          = VK_GEOMETRY_TYPE_INSTANCES_KHR;
    m_buildGeometryInfo.geometry.instances.sType              = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
    m_buildGeometryInfo.geometry.instances.arrayOfPointers    = VK_FALSE;
    m_buildGeometryInfo.geometry.instances.data.deviceAddress = m_instanceBuffer->DeviceAddress( );
}

void VulkanTopLevelAS::Update( const TopLevelASDesc &desc )
{
}

VkBuildAccelerationStructureFlagsKHR VulkanTopLevelAS::Flags( ) const
{
    return m_flags;
}

size_t VulkanTopLevelAS::NumInstances( ) const
{
    return m_instances.size( );
}

const VkAccelerationStructureKHR &VulkanTopLevelAS::Instance( ) const
{
    return m_accelerationStructure;
}

const VkAccelerationStructureGeometryKHR *VulkanTopLevelAS::GeometryDesc( ) const
{
    return &m_buildGeometryInfo;
}

const VkAccelerationStructureBuildRangeInfoKHR *const *VulkanTopLevelAS::BuildRangeInfo( ) const
{
    return m_buildRangeInfoPtr.data( );
}

const VulkanBufferResource *VulkanTopLevelAS::InstanceBuffer( ) const
{
    return m_instanceBuffer.get( );
}

VulkanBufferResource *VulkanTopLevelAS::VulkanBuffer( ) const
{
    return m_buffer.get( );
}

IBufferResource *VulkanTopLevelAS::Buffer( ) const
{
    return m_buffer.get( );
}

const VulkanBufferResource *VulkanTopLevelAS::Scratch( ) const
{
    return m_scratch.get( );
}

VulkanTopLevelAS::~VulkanTopLevelAS( )
{
    vkDestroyAccelerationStructureKHR( m_context->LogicalDevice, m_accelerationStructure, nullptr );
}
