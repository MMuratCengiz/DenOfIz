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
#include <stdexcept>

using namespace DenOfIz;

VulkanShaderBindingTable::VulkanShaderBindingTable(VulkanContext* context, const ShaderBindingTableDesc& desc) : m_context(context) {
    m_pipeline = dynamic_cast<VulkanPipeline*>(desc.Pipeline);
    Resize(desc.SizeDesc);
}

void VulkanShaderBindingTable::Resize(const SBTSizeDesc& desc) {
    const uint32_t rayGenerationShaderNumBytes = AlignRecord(desc.NumRayGenerationShaders * VK_SHADER_GROUP_HANDLE_SIZE_NV);
    const uint32_t hitGroupNumBytes = AlignRecord(desc.NumInstances * desc.NumGeometries * desc.NumRayTypes * VK_SHADER_GROUP_HANDLE_SIZE_NV);
    const uint32_t missShaderNumBytes = AlignRecord(desc.NumMissShaders * VK_SHADER_GROUP_HANDLE_SIZE_NV);
    m_numBufferBytes = rayGenerationShaderNumBytes + hitGroupNumBytes + missShaderNumBytes;

    BufferDesc bufferDesc{};
    bufferDesc.NumBytes = m_numBufferBytes;
    bufferDesc.Usage = BufferUsage::StagingBuffer; // or specific staging buffer settings
    bufferDesc.DebugName = "Shader Binding Table Staging Buffer";

    m_stagingBuffer = std::make_unique<VulkanBufferResource>(m_context, bufferDesc);
    m_mappedMemory = m_stagingBuffer->MapMemory();

    if (!m_mappedMemory) {
        throw std::runtime_error("Failed to map memory for shader binding table.");
    }

    bufferDesc.Usage = BufferUsage::StorageBuffer; // or GPU-only buffer settings
    bufferDesc.DebugName = "Shader Binding Table Buffer";
    m_buffer = std::make_unique<VulkanBufferResource>(m_context, bufferDesc);

    // Set ranges for each shader type
    m_rayGenerationShaderRange.buffer = m_buffer->GetBuffer();
    m_rayGenerationShaderRange.offset = 0;
    m_rayGenerationShaderRange.size = rayGenerationShaderNumBytes;

    m_hitGroupOffset = m_rayGenerationShaderRange.size;

    m_hitGroupShaderRange.buffer = m_buffer->GetBuffer();
    m_hitGroupShaderRange.offset = m_hitGroupOffset;
    m_hitGroupShaderRange.size = hitGroupNumBytes;
    m_hitGroupShaderRange.stride = VK_SHADER_GROUP_HANDLE_SIZE_NV;

    m_missGroupOffset = m_hitGroupOffset + hitGroupNumBytes;

    m_missShaderRange.buffer = m_buffer->GetBuffer();
    m_missShaderRange.offset = m_missGroupOffset;
    m_missShaderRange.size = missShaderNumBytes;
    m_missShaderRange.stride = VK_SHADER_GROUP_HANDLE_SIZE_NV;
}

void VulkanShaderBindingTable::BindRayGenerationShader(const RayGenerationBindingDesc& desc) {
    const void* shaderIdentifier = m_pipeline->GetShaderIdentifier(desc.ShaderName.Get());
    memcpy(m_mappedMemory, shaderIdentifier, VK_SHADER_GROUP_HANDLE_SIZE_NV);
}

void VulkanShaderBindingTable::BindHitGroup(const HitGroupBindingDesc& desc) {
    if (BindHitGroupRecursive(desc)) return;

    const uint32_t instanceOffset = desc.InstanceIndex * m_desc.SizeDesc.NumGeometries * m_desc.SizeDesc.NumRayTypes;
    const uint32_t geometryOffset = desc.GeometryIndex * m_desc.SizeDesc.NumRayTypes;
    const uint32_t rayTypeOffset = desc.RayTypeIndex;

    const uint32_t offset = m_hitGroupOffset + (instanceOffset + geometryOffset + rayTypeOffset) * VK_SHADER_GROUP_HANDLE_SIZE_NV;
    void* hitGroupEntry = static_cast<uint8_t*>(m_mappedMemory) + offset;

    const void* hitGroupIdentifier = m_pipeline->GetShaderIdentifier(desc.HitGroupExportName.Get());
    if (desc.HitGroupExportName.IsEmpty()) {
        throw std::runtime_error("Hit group name cannot be empty.");
    }

    memcpy(hitGroupEntry, hitGroupIdentifier, VK_SHADER_GROUP_HANDLE_SIZE_NV);
}

bool VulkanShaderBindingTable::BindHitGroupRecursive(const HitGroupBindingDesc& desc) {
    // The recursive binding logic adapted for Vulkan
    if (desc.InstanceIndex == -1) {
        for (uint32_t i = 0; i < m_desc.SizeDesc.NumInstances; ++i) {
            HitGroupBindingDesc hitGroupDesc = desc;
            hitGroupDesc.InstanceIndex = i;
            hitGroupDesc.GeometryIndex = -1;
            hitGroupDesc.RayTypeIndex = -1;
            BindHitGroupRecursive(hitGroupDesc);
        }
        return true;
    }
    if (desc.GeometryIndex == -1) {
        for (uint32_t i = 0; i < m_desc.SizeDesc.NumGeometries; ++i) {
            HitGroupBindingDesc hitGroupDesc = desc;
            hitGroupDesc.GeometryIndex = i;
            hitGroupDesc.RayTypeIndex = -1;
            BindHitGroupRecursive(hitGroupDesc);
        }
        return true;
    }
    if (desc.RayTypeIndex == -1) {
        for (uint32_t i = 0; i < m_desc.SizeDesc.NumRayTypes; ++i) {
            HitGroupBindingDesc hitGroupDesc = desc;
            hitGroupDesc.RayTypeIndex = i;
            BindHitGroup(hitGroupDesc);
        }
        return true;
    }
    return false;
}

void VulkanShaderBindingTable::BindMissShader(const MissBindingDesc& desc) {
    uint32_t offset = m_missGroupOffset + desc.RayTypeIndex * VK_SHADER_GROUP_HANDLE_SIZE_NV;
    void* missShaderEntry = static_cast<uint8_t*>(m_mappedMemory) + offset;
    const void* shaderIdentifier = m_pipeline->GetShaderIdentifier(desc.ShaderName.Get());

    memcpy(missShaderEntry, shaderIdentifier, VK_SHADER_GROUP_HANDLE_SIZE_NV);
}

void VulkanShaderBindingTable::Build() {
    m_stagingBuffer->UnmapMemory();

    VkCommandBuffer commandBuffer = m_context->BeginSingleTimeCommands();

    // Copy staging buffer to the device-local buffer
    VkBufferCopy copyRegion{};
    copyRegion.srcOffset = 0;
    copyRegion.dstOffset = 0;
    copyRegion.size = m_numBufferBytes;
    vkCmdCopyBuffer(commandBuffer, m_stagingBuffer->GetBuffer(), m_buffer->GetBuffer(), 1, &copyRegion);

    m_context->EndSingleTimeCommands(commandBuffer);

    m_context->TransitionBuffer(m_buffer->GetBuffer(), VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_NV);
}

VkBuffer VulkanShaderBindingTable::Buffer() const {
    return m_buffer->GetBuffer();
}

VkDeviceAddress VulkanShaderBindingTable::RayGenerationShaderRecord() const {
    return m_rayGenerationShaderRange.offset;
}

VkDeviceAddress VulkanShaderBindingTable::HitGroupShaderRange() const {
    return m_hitGroupShaderRange.offset;
}

VkDeviceAddress VulkanShaderBindingTable::MissShaderRange() const {
    return m_missShaderRange.offset;
}

uint32_t VulkanShaderBindingTable::AlignRecord(const uint32_t size) const {
    return Utilities::Align(size, VK_SHADER_GROUP_HANDLE_ALIGNMENT_NV);
}
