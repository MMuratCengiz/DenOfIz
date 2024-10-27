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
#include <DenOfIzGraphics/Backends/Vulkan/VulkanUtilities.h>
#include <stdexcept>

using namespace DenOfIz;

VulkanShaderBindingTable::VulkanShaderBindingTable( VulkanContext *context, const ShaderBindingTableDesc &desc ) : m_context( context )
{
    m_pipeline = dynamic_cast<VulkanPipeline *>( desc.Pipeline );

    shaderGroupHandleSize    = m_context->RayTracingProperties.shaderGroupHandleSize;
    shaderGroupBaseAlignment = m_context->RayTracingProperties.shaderGroupBaseAlignment;

    Resize( desc.SizeDesc );
}

void VulkanShaderBindingTable::Resize( const SBTSizeDesc &desc )
{
    const uint32_t rayGenerationShaderNumBytes = AlignRecord( desc.NumRayGenerationShaders * shaderGroupHandleSize );
    const uint32_t hitGroupNumBytes            = AlignRecord( desc.NumInstances * desc.NumGeometries * desc.NumRayTypes * shaderGroupHandleSize );
    const uint32_t missShaderNumBytes          = AlignRecord( desc.NumMissShaders * shaderGroupHandleSize );
    m_numBufferBytes                           = rayGenerationShaderNumBytes + hitGroupNumBytes + missShaderNumBytes;

    BufferDesc bufferDesc{ };
    bufferDesc.NumBytes     = m_numBufferBytes;
    bufferDesc.Descriptor   = ResourceDescriptor::Buffer;
    bufferDesc.HeapType     = HeapType::CPU_GPU;
    bufferDesc.InitialState = ResourceState::CopySrc;
    bufferDesc.DebugName    = "Shader Binding Table Staging Buffer";

    m_stagingBuffer = std::make_unique<VulkanBufferResource>( m_context, bufferDesc );
    m_mappedMemory  = m_stagingBuffer->MapMemory( );

    if ( !m_mappedMemory )
    {
        throw std::runtime_error( "Failed to map memory for shader binding table." );
    }

    bufferDesc.InitialState = ResourceState::CopyDst;
    bufferDesc.HeapType     = HeapType::GPU;
    bufferDesc.DebugName    = "Shader Binding Table Buffer";
    m_buffer                = std::make_unique<VulkanBufferResource>( m_context, bufferDesc );

    const VkDeviceAddress &bufferAddress = m_buffer->DeviceAddress( );
    // Set ranges for each shader type
    m_rayGenerationShaderRange.deviceAddress = bufferAddress;
    m_rayGenerationShaderRange.size          = rayGenerationShaderNumBytes;
    m_rayGenerationShaderRange.stride        = shaderGroupHandleSize;

    m_hitGroupOffset = m_rayGenerationShaderRange.size;

    m_hitGroupShaderRange.deviceAddress = bufferAddress + m_hitGroupOffset;
    m_hitGroupShaderRange.size          = hitGroupNumBytes;
    m_hitGroupShaderRange.stride        = shaderGroupHandleSize;

    m_missGroupOffset = m_hitGroupOffset + hitGroupNumBytes;

    m_missShaderRange.deviceAddress = bufferAddress + m_missGroupOffset;
    m_missShaderRange.size          = missShaderNumBytes;
    m_missShaderRange.stride        = shaderGroupHandleSize;
}

void VulkanShaderBindingTable::BindRayGenerationShader( const RayGenerationBindingDesc &desc )
{
    const void *shaderIdentifier = m_pipeline->GetShaderIdentifier( desc.ShaderName.Get( ) );
    memcpy( m_mappedMemory, shaderIdentifier, shaderGroupHandleSize );
}

void VulkanShaderBindingTable::BindHitGroup( const HitGroupBindingDesc &desc )
{
    if ( BindHitGroupRecursive( desc ) )
        return;

    const uint32_t instanceOffset = desc.InstanceIndex * m_desc.SizeDesc.NumGeometries * m_desc.SizeDesc.NumRayTypes;
    const uint32_t geometryOffset = desc.GeometryIndex * m_desc.SizeDesc.NumRayTypes;
    const uint32_t rayTypeOffset  = desc.RayTypeIndex;

    const uint32_t offset        = m_hitGroupOffset + ( instanceOffset + geometryOffset + rayTypeOffset ) * shaderGroupHandleSize;
    void          *hitGroupEntry = static_cast<uint8_t *>( m_mappedMemory ) + offset;

    const void *hitGroupIdentifier = m_pipeline->GetShaderIdentifier( desc.HitGroupExportName.Get( ) );
    if ( desc.HitGroupExportName.IsEmpty( ) )
    {
        throw std::runtime_error( "Hit group name cannot be empty." );
    }

    memcpy( hitGroupEntry, hitGroupIdentifier, shaderGroupHandleSize );
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
    uint32_t    offset           = m_missGroupOffset + desc.RayTypeIndex * shaderGroupHandleSize;
    void       *missShaderEntry  = static_cast<uint8_t *>( m_mappedMemory ) + offset;
    const void *shaderIdentifier = m_pipeline->GetShaderIdentifier( desc.ShaderName.Get( ) );

    memcpy( missShaderEntry, shaderIdentifier, shaderGroupHandleSize );
}

void VulkanShaderBindingTable::Build( )
{
    m_stagingBuffer->UnmapMemory( );
    VulkanUtilities::RunOneTimeCommand( m_context,
                                        [ this ]( auto &commandBuffer )
                                        {
                                            VkBufferCopy copyRegion{ };
                                            copyRegion.srcOffset = 0;
                                            copyRegion.dstOffset = 0;
                                            copyRegion.size      = m_numBufferBytes;
                                            vkCmdCopyBuffer( commandBuffer, m_stagingBuffer->Instance( ), m_buffer->Instance( ), 1, &copyRegion );

                                            VkBufferMemoryBarrier bufferBarrier{ };
                                            bufferBarrier.sType               = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
                                            bufferBarrier.srcAccessMask       = VK_ACCESS_TRANSFER_WRITE_BIT;
                                            bufferBarrier.dstAccessMask       = VK_ACCESS_SHADER_READ_BIT;
                                            bufferBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                                            bufferBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
                                            bufferBarrier.buffer              = m_buffer->Instance( );
                                            bufferBarrier.offset              = 0;
                                            bufferBarrier.size                = m_numBufferBytes;
                                            vkCmdPipelineBarrier( commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV, 0, 0, nullptr, 1,
                                                                  &bufferBarrier, 0, nullptr );
                                        } );
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

uint32_t VulkanShaderBindingTable::AlignRecord( const uint32_t size ) const
{
    return Utilities::Align( size, shaderGroupBaseAlignment );
}
