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

#include <array>

#include "DenOfIzGraphicsInternal/Backends/Vulkan/RayTracing/VulkanShaderLocalData.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanBuffer.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanFence.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanPipeline.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanPipelineBarrierHelper.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

#define VK_PIPELINE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::VulkanPipeline, handle )
#define VK_SHADER_LOCAL_DATA_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::VulkanShaderLocalData, handle )

using namespace DenOfIz;

VulkanShaderBindingTable::VulkanShaderBindingTable( VulkanContext *context, const DenOfIz_ShaderBindingTableDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_pipeline = VK_PIPELINE_IMPL( desc.Pipeline );

    m_shaderGroupHandleSize = Utilities::Align( m_context->RayTracingProperties.shaderGroupHandleSize, m_context->RayTracingProperties.shaderGroupHandleAlignment );
    m_rayGenNumBytes        = Utilities::Align( m_shaderGroupHandleSize + m_desc.MaxRayGenDataBytes, m_context->RayTracingProperties.shaderGroupHandleAlignment );
    m_hitGroupNumBytes      = Utilities::Align( m_shaderGroupHandleSize + m_desc.MaxHitGroupDataBytes, m_context->RayTracingProperties.shaderGroupHandleAlignment );
    m_missGroupNumBytes     = Utilities::Align( m_shaderGroupHandleSize + m_desc.MaxMissDataBytes, m_context->RayTracingProperties.shaderGroupHandleAlignment );

    m_debugData.RayGenNumBytes   = m_rayGenNumBytes;
    m_debugData.MissNumBytes     = m_missGroupNumBytes;
    m_debugData.HitGroupNumBytes = m_hitGroupNumBytes;

    Resize( desc.SizeDesc );
}

void VulkanShaderBindingTable::Resize( const DenOfIz_SBTSizeDesc &desc )
{
    const uint32_t rayGenerationShaderNumBytes = desc.NumRayGenerationShaders * m_rayGenNumBytes;
    const uint32_t hitGroupNumBytes            = desc.NumHitGroups * m_hitGroupNumBytes;
    const uint32_t missShaderNumBytes          = desc.NumMissShaders * m_missGroupNumBytes;
    m_numBufferBytes                           = AlignRecord( rayGenerationShaderNumBytes ) + AlignRecord( hitGroupNumBytes ) + AlignRecord( missShaderNumBytes );

    DenOfIz_BufferDesc bufferDesc{ };
    bufferDesc.NumBytes  = m_numBufferBytes;
    bufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_COPY_SRC_BIT | DENOFIZ_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT;
    bufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU_GPU;
    bufferDesc.DebugName = DENOFIZ_STRING( "Shader Binding Table Staging Buffer" );

    m_stagingBuffer = std::make_unique<VulkanBuffer>( m_context, bufferDesc );
    m_mappedMemory  = m_stagingBuffer->MapMemory( );

    if ( !m_mappedMemory )
    {
        throw std::runtime_error( "Failed to map memory for shader binding table." );
    }

    bufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT;
    bufferDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
    bufferDesc.DebugName = DENOFIZ_STRING( "Shader Binding Table Buffer" );
    m_buffer             = std::make_unique<VulkanBuffer>( m_context, bufferDesc );

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

void VulkanShaderBindingTable::BindRayGenerationShader( const DenOfIz_RayGenerationBindingDesc &desc )
{
    const std::string      shaderName( desc.ShaderName.Chars, desc.ShaderName.NumChars );
    const void            *shaderIdentifier = m_pipeline->GetShaderIdentifier( shaderName );
    void                  *entry            = m_mappedMemory;
    VulkanShaderLocalData *localData        = DENOFIZ_HANDLE_IS_VALID( desc.Data ) ? VK_SHADER_LOCAL_DATA_IMPL( desc.Data ) : nullptr;

    memcpy( entry, shaderIdentifier, m_rayGenNumBytes );
    EncodeData( entry, localData );

#ifndef NDEBUG
    m_debugData.RayGenerationShaders.push_back( { shaderIdentifier, m_shaderGroupHandleSize, localData ? localData->DataNumBytes( ) : 0, shaderName } );
#endif
}

void VulkanShaderBindingTable::BindHitGroup( const DenOfIz_HitGroupBindingDesc &desc )
{
    const uint32_t offset = m_hitGroupOffset + desc.Offset * m_hitGroupNumBytes;
    if ( desc.HitGroupExportName.Chars == nullptr || desc.HitGroupExportName.NumChars == 0 )
    {
        spdlog::critical( "Hit group export name not specified." );
    }

    void             *hitGroupEntry = static_cast<uint8_t *>( m_mappedMemory ) + offset;
    const std::string hitGroupExportName( desc.HitGroupExportName.Chars, desc.HitGroupExportName.NumChars );
    const void       *hitGroupIdentifier = m_pipeline->GetShaderIdentifier( hitGroupExportName );
    if ( !hitGroupIdentifier )
    {
        spdlog::error( "Hit group export not found in pipeline." );
        return;
    }

    memcpy( hitGroupEntry, hitGroupIdentifier, m_shaderGroupHandleSize );

    VulkanShaderLocalData *localData = DENOFIZ_HANDLE_IS_VALID( desc.Data ) ? VK_SHADER_LOCAL_DATA_IMPL( desc.Data ) : nullptr;
    EncodeData( hitGroupEntry, localData );

#ifndef NDEBUG
    m_debugData.HitGroups.push_back( { hitGroupIdentifier, m_shaderGroupHandleSize, localData ? localData->DataNumBytes( ) : 0, hitGroupExportName } );
#endif
}

void VulkanShaderBindingTable::BindMissShader( const DenOfIz_MissBindingDesc &desc )
{
    const uint32_t         offset          = m_missGroupOffset + desc.Offset * m_missGroupNumBytes;
    void                  *missShaderEntry = static_cast<uint8_t *>( m_mappedMemory ) + offset;
    const std::string      shaderName( desc.ShaderName.Chars, desc.ShaderName.NumChars );
    const void            *shaderIdentifier = m_pipeline->GetShaderIdentifier( shaderName );
    VulkanShaderLocalData *localData        = DENOFIZ_HANDLE_IS_VALID( desc.Data ) ? VK_SHADER_LOCAL_DATA_IMPL( desc.Data ) : nullptr;

    memcpy( missShaderEntry, shaderIdentifier, m_shaderGroupHandleSize );
    EncodeData( missShaderEntry, localData );

#ifndef NDEBUG
    m_debugData.MissShaders.push_back( { shaderIdentifier, m_shaderGroupHandleSize, localData ? localData->DataNumBytes( ) : 0, std::string( desc.ShaderName.Chars ) } );
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

    std::array                  bufferBarriers = { DenOfIz_BufferBarrierDesc{
                         .Resource = DENOFIZ_TO_HANDLE( m_buffer.get( ) ), .OldState = DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT, .NewState = DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT } };
    DenOfIz_PipelineBarrierDesc bufferBarrierDesc{ };
    bufferBarrierDesc.BufferBarriers.Elements    = bufferBarriers.data( );
    bufferBarrierDesc.BufferBarriers.NumElements = bufferBarriers.size( );
    VulkanPipelineBarrierHelper::ExecutePipelineBarrier( m_context, commandBuffer, DENOFIZ_QUEUE_TYPE_COMPUTE, &bufferBarrierDesc );

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

VulkanBuffer *VulkanShaderBindingTable::GetVulkanBuffer( ) const
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

void VulkanShaderBindingTable::EncodeData( void *entry, VulkanShaderLocalData *data ) const
{
    if ( data )
    {
        void *localData = static_cast<uint8_t *>( entry ) + m_shaderGroupHandleSize;

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
