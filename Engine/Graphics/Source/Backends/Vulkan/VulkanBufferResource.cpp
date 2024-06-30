/*
Blazar Engine - 3D Game Engine
Copyright (c) 2020-2021 Muhammed Murat Cengiz

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
#include <DenOfIzGraphics/Backends/Vulkan/VulkanBufferResource.h>
#include "DenOfIzGraphics/Backends/Vulkan/VulkanEnumConverter.h"
#include "DenOfIzGraphics/Backends/Vulkan/VulkanUtilities.h"

using namespace DenOfIz;

VulkanBufferResource::VulkanBufferResource(VulkanContext *context, const BufferDesc &desc) : m_desc(desc), m_context(context), m_allocation(nullptr) {}

void VulkanBufferResource::Allocate(const void *newData)
{
    if ( m_alreadyAllocated )
    {
        UpdateAllocation(newData);
        return;
    }

    m_alreadyDisposed = false;
    m_alreadyAllocated = true;

    std::pair<vk::Buffer, VmaAllocation> stagingBuffer;

    bool useStagingBuffer = m_desc.HeapType == HeapType::GPU;
    if ( useStagingBuffer )
    {
        VulkanUtilities::InitStagingBuffer(m_context, stagingBuffer.first, stagingBuffer.second, newData, m_size);
    }

    vk::BufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.usage = VulkanEnumConverter::ConvertBufferUsage(m_desc.Descriptor, m_desc.InitialState);
    bufferCreateInfo.size = m_size;
    bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;

    VmaAllocationCreateInfo allocationCreateInfo{};

    allocationCreateInfo.usage = VulkanEnumConverter::ConvertMemoryLocation(m_desc.HeapType);

    // Todo more flexibility, or a clearer interface here:
    if ( m_desc.HeapType == HeapType::CPU_GPU )
    {
        const auto gpuVisible = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eDeviceLocal;
        allocationCreateInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(vk::MemoryPropertyFlagBits::eHostVisible);
        allocationCreateInfo.preferredFlags = static_cast<VkMemoryPropertyFlags>(gpuVisible);
    }
    else
    {
        bufferCreateInfo.usage = bufferCreateInfo.usage | vk::BufferUsageFlagBits::eTransferDst;
        allocationCreateInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(vk::MemoryPropertyFlagBits::eDeviceLocal);
    }

    VmaAllocationInfo allocationInfo;
    vmaCreateBuffer(m_context->Vma, reinterpret_cast<VkBufferCreateInfo *>(&bufferCreateInfo), &allocationCreateInfo, reinterpret_cast<VkBuffer *>(&Instance), &m_allocation,
                    &allocationInfo);

    if ( useStagingBuffer )
    {
        VulkanUtilities::CopyBuffer(m_context, stagingBuffer.first, Instance, m_size);
        vmaDestroyBuffer(m_context->Vma, stagingBuffer.first, stagingBuffer.second);
    }
    else if ( m_mappedMemory != nullptr && m_desc.KeepMemoryMapped )
    {
        memcpy(m_mappedMemory, newData, m_size);
    }
    else
    {
        vmaMapMemory(this->m_context->Vma, m_allocation, &m_mappedMemory);
        memcpy(m_mappedMemory, newData, m_size);

        if ( !m_desc.KeepMemoryMapped )
        {
            vmaUnmapMemory(m_context->Vma, m_allocation);
        }
    }

    DescriptorInfo.buffer = Instance;
    DescriptorInfo.offset = 0;
    DescriptorInfo.range = m_size;
}

void VulkanBufferResource::UpdateAllocation(const void *newData)
{
    if ( m_desc.HeapType == HeapType::GPU )
    {
        std::pair<vk::Buffer, VmaAllocation> stagingBuffer;
        VulkanUtilities::InitStagingBuffer(m_context, stagingBuffer.first, stagingBuffer.second, newData, m_size);
        VulkanUtilities::CopyBuffer(m_context, stagingBuffer.first, Instance, m_size);
        vmaDestroyBuffer(m_context->Vma, stagingBuffer.first, stagingBuffer.second);
    }
    else if ( m_desc.KeepMemoryMapped )
    {
        memcpy(m_mappedMemory, newData, m_size);
    }
    else
    {
        vmaMapMemory(m_context->Vma, m_allocation, &m_mappedMemory);
        memcpy(m_mappedMemory, newData, m_size);
        vmaUnmapMemory(m_context->Vma, m_allocation);
    }
}

void VulkanBufferResource::Deallocate()
{
    m_alreadyAllocated = false;
    DZ_RETURN_IF(m_alreadyDisposed);
    m_alreadyDisposed = true;

    if ( m_desc.KeepMemoryMapped )
    {
        vmaUnmapMemory(m_context->Vma, m_allocation);
    }

    vmaDestroyBuffer(m_context->Vma, Instance, m_allocation);
}

VulkanBufferResource::~VulkanBufferResource() { VulkanBufferResource::Deallocate(); }
