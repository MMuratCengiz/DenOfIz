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

#include "DenOfIzGraphicsInternal/Backends/Vulkan/RayTracing/VulkanShaderBindingTable.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/RayTracing/VulkanShaderLocalData.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanBufferResource.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanFence.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanPipeline.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanPipelineBarrierHelper.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

using namespace DenOfIz;

VulkanShaderBindingTable::VulkanShaderBindingTable( VulkanContext *context, const ShaderBindingTableDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_pipeline = dynamic_cast<VulkanPipeline *>( desc.Pipeline );

    m_shaderGroupHandleSize = Utilities::Align( m_context->RayTracingProperties.shaderGroupHandleSize, m_context->RayTracingProperties.shaderGroupHandleAlignment );
    m_rayGenNumBytes        = Utilities::Align( m_shaderGroupHandleSize + m_desc.MaxRayGenDataBytes, m_context->RayTracingProperties.shaderGroupHandleAlignment );
    m_hitGroupNumBytes      = Utilities::Align( m_shaderGroupHandleSize + m_desc.MaxHitGroupDataBytes, m_context->RayTracingProperties.shaderGroupHandleAlignment );
    m_missGroupNumBytes     = Utilities::Align( m_shaderGroupHandleSize + m_desc.MaxMissDataBytes, m_context->RayTracingProperties.shaderGroupHandleAlignment );

    m_debugData.RayGenNumBytes   = m_rayGenNumBytes;
    m_debugData.MissNumBytes     = m_missGroupNumBytes;
    m_debugData.HitGroupNumBytes = m_hitGroupNumBytes;

    Resize( desc.SizeDesc );
}

void VulkanShaderBindingTable::Resize( const SBTSizeDesc &desc )
{
    const uint32_t rayGenerationShaderNumBytes = desc.NumRayGenerationShaders * m_rayGenNumBytes;
    const uint32_t hitGroupNumBytes            = desc.NumHitGroups * m_hitGroupNumBytes;
    const uint32_t missShaderNumBytes          = desc.NumMissShaders * m_missGroupNumBytes;
    m_numBufferBytes                           = AlignRecord( rayGenerationShaderNumBytes ) + AlignRecord( hitGroupNumBytes ) + AlignRecord( missShaderNumBytes );

    BufferDesc bufferDesc{ };
    bufferDesc.NumBytes   = m_numBufferBytes;
    bufferDesc.Descriptor = BitSet( ResourceDescriptor::Buffer );
    bufferDesc.Usages     = BitSet( ResourceUsage::CopySrc ) | ResourceUsage::ShaderBindingTable;
    bufferDesc.HeapType   = HeapType::CPU_GPU;
    bufferDesc.DebugName  = "Shader Binding Table Staging Buffer";

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

    m_rayGenerationShaderRange               = { };
    m_rayGenerationShaderRange.deviceAddress = bufferAddress;
    m_rayGenerationShaderRange.size          = rayGenerationShaderNumBytes;
    m_rayGenerationShaderRange.stride        = rayGenerationShaderNumBytes;

    m_hitGroupOffset                    = AlignRecord( m_rayGenerationShaderRange.size );
    m_hitGroupShaderRange               = { };
    m_hitGroupShaderRange.deviceAddress = bufferAddress + m_hitGroupOffset;
    m_hitGroupShaderRange.size          = hitGroupNumBytes;
    m_hitGroupShaderRange.stride        = m_hitGroupNumBytes;

    m_missGroupOffset               = AlignRecord( m_hitGroupOffset + hitGroupNumBytes );
    m_missShaderRange               = { };
    m_missShaderRange.deviceAddress = bufferAddress + m_missGroupOffset;
    m_missShaderRange.size          = missShaderNumBytes;
    m_missShaderRange.stride        = m_missGroupNumBytes;

    m_callableShaderRange = { };
}

void VulkanShaderBindingTable::BindRayGenerationShader( const RayGenerationBindingDesc &desc )
{
    const void *shaderIdentifier = m_pipeline->GetShaderIdentifier( desc.ShaderName.Get( ) );
    void       *entry            = m_mappedMemory;

    memcpy( entry, shaderIdentifier, m_rayGenNumBytes );
    EncodeData( entry, desc.Data );

#ifndef NDEBUG
    m_debugData.RayGenerationShaders.AddElement(
        { shaderIdentifier, m_shaderGroupHandleSize, desc.Data ? dynamic_cast<VulkanShaderLocalData *>( desc.Data )->DataNumBytes( ) : 0, desc.ShaderName.Get( ) } );
#endif
}

void VulkanShaderBindingTable::BindHitGroup( const HitGroupBindingDesc &desc )
{
    const uint32_t offset = m_hitGroupOffset + desc.Offset * m_hitGroupNumBytes;
    if ( desc.HitGroupExportName.IsEmpty( ) )
    {
        throw std::runtime_error( "Hit group name cannot be empty." );
    }

    void       *hitGroupEntry      = static_cast<uint8_t *>( m_mappedMemory ) + offset;
    const void *hitGroupIdentifier = m_pipeline->GetShaderIdentifier( desc.HitGroupExportName.Get( ) );
    if ( !hitGroupIdentifier )
    {
        LOG( ERROR ) << "Hit group export not found in pipeline.";
        return;
    }

    memcpy( hitGroupEntry, hitGroupIdentifier, m_shaderGroupHandleSize );
    if ( desc.Data )
    {
        const VulkanShaderLocalData *data = dynamic_cast<VulkanShaderLocalData *>( desc.Data );
        EncodeData( hitGroupEntry, desc.Data );
#ifndef NDEBUG
        m_debugData.HitGroups.AddElement( { hitGroupIdentifier, m_shaderGroupHandleSize, data->DataNumBytes( ), desc.HitGroupExportName.Get( ) } );
    }
    else
    {
        m_debugData.HitGroups.AddElement( { hitGroupIdentifier, m_shaderGroupHandleSize, 0, desc.HitGroupExportName.Get( ) } );
#endif
    }
}

void VulkanShaderBindingTable::BindMissShader( const MissBindingDesc &desc )
{
    const uint32_t offset           = m_missGroupOffset + desc.Offset * m_missGroupNumBytes;
    void          *missShaderEntry  = static_cast<uint8_t *>( m_mappedMemory ) + offset;
    const void    *shaderIdentifier = m_pipeline->GetShaderIdentifier( desc.ShaderName.Get( ) );

    memcpy( missShaderEntry, shaderIdentifier, m_shaderGroupHandleSize );
    EncodeData( missShaderEntry, desc.Data );

#ifndef NDEBUG
    m_debugData.MissShaders.AddElement(
        { shaderIdentifier, m_shaderGroupHandleSize, desc.Data ? dynamic_cast<VulkanShaderLocalData *>( desc.Data )->DataNumBytes( ) : 0, desc.ShaderName.Get( ) } );
#endif
}

void VulkanShaderBindingTable::Build( )
{
#ifndef NDEBUG
    PrintShaderBindingTableDebugData( m_debugData );
#endif
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
    VulkanPipelineBarrierHelper::ExecutePipelineBarrier( m_context, commandBuffer, QueueType::Compute, barrier );

    VK_CHECK_RESULT( vkEndCommandBuffer( commandBuffer ) );

    VkSubmitInfo vkSubmitInfo{ };
    vkSubmitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    vkSubmitInfo.commandBufferCount = 1;
    vkSubmitInfo.pCommandBuffers    = &commandBuffer;

    const auto vkNotifyFence = std::make_unique<VulkanFence>( m_context );
    vkNotifyFence->Reset( );
    VK_CHECK_RESULT( vkQueueSubmit( m_context->Queues[ VulkanQueueType::Compute ], 1, &vkSubmitInfo, vkNotifyFence->GetFence( ) ) );
    vkNotifyFence->Wait( );
}

VulkanBufferResource *VulkanShaderBindingTable::VulkanBuffer( ) const
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

void VulkanShaderBindingTable::EncodeData( void *entry, IShaderLocalData *iData ) const
{
    if ( iData )
    {
        void                        *localData = static_cast<uint8_t *>( entry ) + m_shaderGroupHandleSize;
        const VulkanShaderLocalData *data      = dynamic_cast<VulkanShaderLocalData *>( iData );

        if ( data->DataNumBytes( ) > 0 )
        {
            memcpy( localData, data->Data( ), data->DataNumBytes( ) );
        }

        if ( *data->DescriptorSet( ) != nullptr )
        {
            memcpy( static_cast<uint8_t *>( localData ) + data->DataNumBytes( ), data->DescriptorSet( ), sizeof( VkDescriptorSet ) );
        }
    }
}
