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

#include <DenOfIzGraphics/Backends/Vulkan/RayTracing/VulkanShaderBindingTable.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanBufferResource.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanFence.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanPipelineBarrierHelper.h>
#include <stdexcept>

using namespace DenOfIz;

VulkanShaderBindingTable::VulkanShaderBindingTable( VulkanContext *context, const ShaderBindingTableDesc &desc ) : m_context( context )
{
    m_pipeline              = dynamic_cast<VulkanPipeline *>( desc.Pipeline );
    m_shaderGroupHandleSize = Utilities::Align( m_context->RayTracingProperties.shaderGroupHandleSize, m_context->RayTracingProperties.shaderGroupHandleAlignment );
    Resize( desc.SizeDesc );
}

void VulkanShaderBindingTable::Resize( const SBTSizeDesc &desc )
{
    const uint32_t rayGenerationShaderNumBytes = AlignRecord( desc.NumRayGenerationShaders * m_shaderGroupHandleSize );
    const uint32_t hitGroupNumBytes            = AlignRecord( desc.NumInstances * desc.NumGeometries * desc.NumRayTypes * m_shaderGroupHandleSize );
    const uint32_t missShaderNumBytes          = AlignRecord( desc.NumMissShaders * m_shaderGroupHandleSize );
    m_numBufferBytes                           = rayGenerationShaderNumBytes + hitGroupNumBytes + missShaderNumBytes;

    BufferDesc bufferDesc{ };
    bufferDesc.NumBytes     = m_numBufferBytes;
    bufferDesc.Descriptor   = BitSet( ResourceDescriptor::Buffer );
    bufferDesc.Usages       = BitSet( ResourceUsage::CopySrc ) | ResourceUsage::ShaderBindingTable;
    bufferDesc.InitialUsage = ResourceUsage::CopySrc;
    bufferDesc.HeapType     = HeapType::CPU_GPU;
    bufferDesc.DebugName    = "Shader Binding Table Staging Buffer";

    m_stagingBuffer = std::make_unique<VulkanBufferResource>( m_context, bufferDesc );
    m_mappedMemory  = m_stagingBuffer->MapMemory( );

    if ( !m_mappedMemory )
    {
        throw std::runtime_error( "Failed to map memory for shader binding table." );
    }

    bufferDesc.InitialUsage = ResourceUsage::CopyDst;
    bufferDesc.Usages       = BitSet( ResourceUsage::ShaderBindingTable ) | ResourceUsage::CopyDst;
    bufferDesc.HeapType     = HeapType::GPU;
    bufferDesc.DebugName    = "Shader Binding Table Buffer";
    m_buffer                = std::make_unique<VulkanBufferResource>( m_context, bufferDesc );

    const VkDeviceAddress &bufferAddress = m_buffer->DeviceAddress( );
    // Set ranges for each shader type
    m_rayGenerationShaderRange               = { };
    m_rayGenerationShaderRange.deviceAddress = bufferAddress;
    m_rayGenerationShaderRange.size          = rayGenerationShaderNumBytes;
    m_rayGenerationShaderRange.stride        = rayGenerationShaderNumBytes;

    m_hitGroupOffset = m_rayGenerationShaderRange.size;

    m_hitGroupShaderRange               = { };
    m_hitGroupShaderRange.deviceAddress = bufferAddress + m_hitGroupOffset;
    m_hitGroupShaderRange.size          = hitGroupNumBytes;
    m_hitGroupShaderRange.stride        = Utilities::Align( m_shaderGroupHandleSize, m_context->RayTracingProperties.shaderGroupHandleAlignment );

    m_missGroupOffset = m_hitGroupOffset + hitGroupNumBytes;

    m_missShaderRange               = { };
    m_missShaderRange.deviceAddress = bufferAddress + m_missGroupOffset;
    m_missShaderRange.size          = missShaderNumBytes;
    m_missShaderRange.stride        = Utilities::Align( m_shaderGroupHandleSize, m_context->RayTracingProperties.shaderGroupHandleAlignment );

    m_callableShaderRange = { };
}

void VulkanShaderBindingTable::BindRayGenerationShader( const RayGenerationBindingDesc &desc )
{
    const void *shaderIdentifier = m_pipeline->GetShaderIdentifier( desc.ShaderName.Get( ) );
    memcpy( m_mappedMemory, shaderIdentifier, m_shaderGroupHandleSize );
}

void VulkanShaderBindingTable::BindHitGroup( const HitGroupBindingDesc &desc )
{
    if ( BindHitGroupRecursive( desc ) )
        return;

    const uint32_t instanceOffset = desc.InstanceIndex * m_desc.SizeDesc.NumGeometries * m_desc.SizeDesc.NumRayTypes;
    const uint32_t geometryOffset = desc.GeometryIndex * m_desc.SizeDesc.NumRayTypes;
    const uint32_t rayTypeOffset  = desc.RayTypeIndex;

    uint32_t offset = m_hitGroupOffset + ( instanceOffset + geometryOffset + rayTypeOffset ) * m_shaderGroupHandleSize;
    if ( desc.HitGroupExportName.IsEmpty( ) )
    {
        throw std::runtime_error( "Hit group name cannot be empty." );
    }

    auto shaderIdentifiers = m_pipeline->HitGroupIdentifiers( );

    // Bind each shader type to the correct slot in the SBT, using null if missing
    for ( const auto &[ shaderType, identifierOffset ] : shaderIdentifiers )
    {
        switch ( shaderType )
        {
        case ShaderStage::ClosestHit:
            break;
        case ShaderStage::AnyHit:
            offset += m_shaderGroupHandleSize;
            break;
        case ShaderStage::Intersection:
            offset += 2 * m_shaderGroupHandleSize;
            break;
        default:
            LOG( ERROR ) << "Unknown shader type in hit group.";
            break;
        }

        // Perform the actual binding
        void *hitGroupEntry = static_cast<uint8_t *>( m_mappedMemory ) + offset;
        memcpy( hitGroupEntry, m_pipeline->GetShaderIdentifier( identifierOffset ), m_shaderGroupHandleSize );
    }
}

bool VulkanShaderBindingTable::BindHitGroupRecursive( const HitGroupBindingDesc &desc )
{
    // The recursive binding logic adapted for Vulkan
    if ( desc.InstanceIndex == -1 )
    {
        for ( uint32_t i = 0; i < m_desc.SizeDesc.NumInstances; ++i )
        {
            HitGroupBindingDesc hitGroupDesc = desc;
            hitGroupDesc.InstanceIndex       = i;
            hitGroupDesc.GeometryIndex       = -1;
            hitGroupDesc.RayTypeIndex        = -1;
            BindHitGroupRecursive( hitGroupDesc );
        }
        return true;
    }
    if ( desc.GeometryIndex == -1 )
    {
        for ( uint32_t i = 0; i < m_desc.SizeDesc.NumGeometries; ++i )
        {
            HitGroupBindingDesc hitGroupDesc = desc;
            hitGroupDesc.GeometryIndex       = i;
            hitGroupDesc.RayTypeIndex        = -1;
            BindHitGroupRecursive( hitGroupDesc );
        }
        return true;
    }
    if ( desc.RayTypeIndex == -1 )
    {
        for ( uint32_t i = 0; i < m_desc.SizeDesc.NumRayTypes; ++i )
        {
            HitGroupBindingDesc hitGroupDesc = desc;
            hitGroupDesc.RayTypeIndex        = i;
            BindHitGroup( hitGroupDesc );
        }
        return true;
    }
    return false;
}

void VulkanShaderBindingTable::BindMissShader( const MissBindingDesc &desc )
{
    uint32_t    offset           = m_missGroupOffset + desc.RayTypeIndex * m_shaderGroupHandleSize;
    void       *missShaderEntry  = static_cast<uint8_t *>( m_mappedMemory ) + offset;
    const void *shaderIdentifier = m_pipeline->GetShaderIdentifier( desc.ShaderName.Get( ) );

    memcpy( missShaderEntry, shaderIdentifier, m_shaderGroupHandleSize );
}

void VulkanShaderBindingTable::Build( )
{
    m_stagingBuffer->UnmapMemory( );

    VkCommandBufferAllocateInfo bufferAllocateInfo{ };
    bufferAllocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    bufferAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    bufferAllocateInfo.commandPool        = m_context->ComputeQueueCommandPool;
    bufferAllocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers( m_context->LogicalDevice, &bufferAllocateInfo, &commandBuffer );

    VkCommandBufferBeginInfo beginInfo{ };
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK_RESULT( vkBeginCommandBuffer( commandBuffer, &beginInfo ) );

    VkBufferCopy copyRegion{ };
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size      = m_numBufferBytes;
    vkCmdCopyBuffer( commandBuffer, m_stagingBuffer->Instance( ), m_buffer->Instance( ), 1, &copyRegion );

    PipelineBarrierDesc barrier{ };
    barrier.BufferBarrier( BufferBarrierDesc{ .Resource = m_buffer.get( ), .OldState = ResourceUsage::CopyDst, .NewState = ResourceUsage::ShaderResource } );
    VulkanPipelineBarrierHelper::ExecutePipelineBarrier( m_context, commandBuffer, QueueType::RayTracing, barrier );

    VK_CHECK_RESULT( vkEndCommandBuffer( commandBuffer ) );

    VkSubmitInfo vkSubmitInfo{ };
    vkSubmitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo.commandBufferCount = 1;
    vkSubmitInfo.pCommandBuffers    = &commandBuffer;

    std::unique_ptr<VulkanFence> vkNotifyFence = std::make_unique<VulkanFence>( m_context );
    vkNotifyFence->Reset( );
    VK_CHECK_RESULT( vkQueueSubmit( m_context->Queues[ VulkanQueueType::Compute ], 1, &vkSubmitInfo, vkNotifyFence->GetFence( ) ) );
    vkNotifyFence->Wait( );
}

VulkanBufferResource *VulkanShaderBindingTable::VulkanBuffer( ) const
{
    return m_buffer.get( );
}

IBufferResource *VulkanShaderBindingTable::Buffer( ) const
{
    return m_buffer.get( );
}

const VkStridedDeviceAddressRegionKHR *VulkanShaderBindingTable::RayGenerationShaderRange( ) const
{
    return &m_rayGenerationShaderRange;
}

const VkStridedDeviceAddressRegionKHR *VulkanShaderBindingTable::MissShaderRange( ) const
{
    return &m_missShaderRange;
}

const VkStridedDeviceAddressRegionKHR *VulkanShaderBindingTable::HitGroupShaderRange( ) const
{
    return &m_hitGroupShaderRange;
}

const VkStridedDeviceAddressRegionKHR *VulkanShaderBindingTable::CallableShaderRange( ) const
{
    return &m_callableShaderRange;
}

uint32_t VulkanShaderBindingTable::AlignRecord( const uint32_t size ) const
{
    return Utilities::Align( size, m_context->RayTracingProperties.shaderGroupBaseAlignment );
}
