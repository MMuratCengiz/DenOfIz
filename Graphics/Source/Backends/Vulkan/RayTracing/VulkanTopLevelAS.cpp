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

        VkBuffer buffer = vkBLASBuffer->Instance( );

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

    // Calculate size requirements for top-level acceleration structure
    VkAccelerationStructureBuildGeometryInfoKHR buildInfo = { };
    buildInfo.sType                                       = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
    buildInfo.type                                        = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
    buildInfo.flags                                       = m_flags;
    buildInfo.mode                                        = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;

    VkAccelerationStructureBuildSizesInfoKHR sizeInfo = { };
    sizeInfo.sType                                    = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
    uint32_t numElements                              = desc.Instances.NumElements( );
    vkGetAccelerationStructureBuildSizesKHR( m_context->LogicalDevice, VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR, &buildInfo, &numElements, &sizeInfo );

    // Create buffer for instance data
    BufferDesc instanceBufferDesc = { };
    instanceBufferDesc.NumBytes   = desc.Instances.NumElements( ) * sizeof( VkAccelerationStructureInstanceKHR );
    instanceBufferDesc.Descriptor = ResourceDescriptor::RWBuffer;
    instanceBufferDesc.HeapType   = HeapType::CPU_GPU;
    m_instanceBuffer              = std::make_unique<VulkanBufferResource>( m_context, instanceBufferDesc );

    void *instanceBufferMemory = m_instanceBuffer->MapMemory( );
    memcpy( instanceBufferMemory, m_instances.data( ), instanceBufferDesc.NumBytes );
    m_instanceBuffer->UnmapMemory( );

    // Create acceleration structure buffer
    BufferDesc bufferDesc = { };
    bufferDesc.NumBytes   = sizeInfo.accelerationStructureSize;
    bufferDesc.Descriptor = ResourceDescriptor::AccelerationStructure;
    bufferDesc.HeapType   = HeapType::GPU;
    m_buffer              = std::make_unique<VulkanBufferResource>( m_context, bufferDesc );

    // Initialize scratch buffer
    BufferDesc scratchBufferDesc   = { };
    scratchBufferDesc.NumBytes     = sizeInfo.buildScratchSize;
    scratchBufferDesc.InitialState = ResourceState::UnorderedAccess;
    scratchBufferDesc.Descriptor   = ResourceDescriptor::RWBuffer;
    scratchBufferDesc.HeapType     = HeapType::GPU;
    m_scratch                      = std::make_unique<VulkanBufferResource>( m_context, scratchBufferDesc );
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

const VkAccelerationStructureKHR &VulkanTopLevelAS::AccelerationStructure( ) const
{
    return m_accelerationStructure;
}

const VkAccelerationStructureBuildRangeInfoKHR &VulkanTopLevelAS::BuildRangeInfo( ) const
{
    return m_buildRangeInfo;
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

VulkanTopLevelAS::~VulkanTopLevelAS( ) = default;
