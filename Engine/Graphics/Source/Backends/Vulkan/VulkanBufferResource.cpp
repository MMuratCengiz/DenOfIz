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

VulkanBufferResource::VulkanBufferResource(VulkanContext *context, const BufferDesc &desc) : m_desc(desc), m_context(context), m_allocation(nullptr)
{
    m_numBytes = m_desc.NumBytes;

    vk::BufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.usage       = VulkanEnumConverter::ConvertBufferUsage(m_desc.Descriptor, m_desc.InitialState);
    bufferCreateInfo.size        = m_numBytes;
    bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;

    VmaAllocationCreateInfo allocationCreateInfo{};

    allocationCreateInfo.usage = VulkanEnumConverter::ConvertHeapType(m_desc.HeapType);

    // Todo more flexibility, or a clearer interface here:
    if ( m_desc.HeapType == HeapType::CPU || m_desc.HeapType == HeapType::CPU_GPU )
    {
        allocationCreateInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(vk::MemoryPropertyFlagBits::eHostVisible);
    }
    if ( m_desc.HeapType == HeapType::CPU_GPU )
    {
        allocationCreateInfo.preferredFlags = static_cast<VkMemoryPropertyFlags>(vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eDeviceLocal);
    }
    if ( m_desc.HeapType == HeapType::GPU || m_desc.HeapType == HeapType::GPU_CPU )
    {
        allocationCreateInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(vk::MemoryPropertyFlagBits::eDeviceLocal);
    }
    if ( m_desc.HeapType == HeapType::GPU_CPU )
    {
        allocationCreateInfo.preferredFlags = static_cast<VkMemoryPropertyFlags>(vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);
    }

    bufferCreateInfo.usage = VulkanEnumConverter::ConvertBufferUsage(m_desc.Descriptor, m_desc.InitialState);
    VmaAllocationInfo allocationInfo;
    vmaCreateBuffer(m_context->Vma, reinterpret_cast<VkBufferCreateInfo *>(&bufferCreateInfo), &allocationCreateInfo, reinterpret_cast<VkBuffer *>(&Instance), &m_allocation,
                    &allocationInfo);

    DescriptorInfo.buffer = Instance;
    DescriptorInfo.offset = 0;
    DescriptorInfo.range  = m_numBytes;
}

void VulkanBufferResource::MapMemory()
{
    DZ_ASSERTM(m_desc.HeapType == HeapType::CPU_GPU || m_desc.HeapType == HeapType::CPU, "Can only map to CPU visible buffer");
    DZ_ASSERTM(m_mappedMemory == nullptr, std::format("Memory already mapped {}", Name.c_str()));
    vmaMapMemory(m_context->Vma, m_allocation, &m_mappedMemory);
}

void VulkanBufferResource::CopyData(const void *data, uint32_t size)
{
    DZ_ASSERTM(m_mappedMemory != nullptr, std::format("Memory not mapped  buffer: {}", Name.c_str()));
    memcpy(m_mappedMemory, data, size);
}

void *VulkanBufferResource::ReadData()
{
    DZ_ASSERTM(m_mappedMemory != nullptr, std::format("Memory not mapped, buffer: {}", Name.c_str()));
    return m_mappedMemory;
}

void VulkanBufferResource::UnmapMemory()
{
    DZ_ASSERTM(m_mappedMemory != nullptr, std::format("Memory not mapped, buffer: {}", Name.c_str()));
    vmaUnmapMemory(m_context->Vma, m_allocation);
    m_mappedMemory = nullptr;
}
VulkanBufferResource::~VulkanBufferResource()
{
    if ( m_mappedMemory != nullptr )
    {
        LOG(WARNING) << "Memory not unmapped before lifetime of the buffer.";
        vmaUnmapMemory(m_context->Vma, m_allocation);
    }

    vmaDestroyBuffer(m_context->Vma, Instance, m_allocation);
}
