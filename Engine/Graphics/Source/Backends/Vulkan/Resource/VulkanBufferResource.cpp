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
#ifdef BUILD_VK

#include "DenOfIzGraphics/Backends/Vulkan/VulkanUtilities.h"
#include <DenOfIzGraphics/Backends/Vulkan/Resource/VulkanBufferResource.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanEnumConverter.h>

using namespace DenOfIz;

VulkanBufferResource::VulkanBufferResource( VulkanContext *context, const BufferCreateInfo &createInfo ) :
    m_createInfo( createInfo ), m_context( context ), m_allocation( nullptr )
{
}

void VulkanBufferResource::Allocate( const void *newData )
{
    Data = newData;
    if ( m_alreadyAllocated )
    {
        UpdateAllocation( newData );
        return;
    }

    m_alreadyDisposed = false;
    m_alreadyAllocated = true;

    std::pair<vk::Buffer, VmaAllocation> stagingBuffer;
    Size = m_createInfo.MemoryCreateInfo.Size;

    if ( m_createInfo.UseStaging )
    {
        VulkanUtilities::InitStagingBuffer( m_context, stagingBuffer.first, stagingBuffer.second, newData, Size );
    }

    vk::BufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.usage = VulkanEnumConverter::ConvertBufferUsage( m_createInfo.MemoryCreateInfo.Usage );
    bufferCreateInfo.size = Size;
    bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;

    VmaAllocationCreateInfo allocationCreateInfo{};

    allocationCreateInfo.usage = VulkanEnumConverter::ConvertMemoryLocation( m_createInfo.MemoryCreateInfo.Location );

    // Todo more flexibility, or a clearer interface here:
    if ( m_createInfo.MemoryCreateInfo.Location == MemoryLocation::CPU_GPU )
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
    vmaCreateBuffer( m_context->Vma, reinterpret_cast<VkBufferCreateInfo *>(&bufferCreateInfo), &allocationCreateInfo, reinterpret_cast<VkBuffer *>(&Instance), &m_allocation, &allocationInfo );

    if ( m_createInfo.UseStaging )
    {
        VulkanUtilities::CopyBuffer( m_context, stagingBuffer.first, Instance, Size );
        vmaDestroyBuffer( m_context->Vma, stagingBuffer.first, stagingBuffer.second );
    }
    else if ( mappedMemory != nullptr && m_createInfo.KeepMemoryMapped )
    {
        memcpy( mappedMemory, newData, Size );
    }
    else
    {
        vmaMapMemory( this->m_context->Vma, m_allocation, &mappedMemory );
        memcpy( mappedMemory, newData, Size );

        if ( !m_createInfo.KeepMemoryMapped )
        {
            vmaUnmapMemory( m_context->Vma, m_allocation );
        }
    }

    DescriptorInfo.buffer = Instance;
    DescriptorInfo.offset = 0;
    DescriptorInfo.range = Size;
}

void VulkanBufferResource::UpdateAllocation( const void *newData )
{
    if ( m_createInfo.UseStaging )
    {
        std::pair<vk::Buffer, VmaAllocation> stagingBuffer;
        VulkanUtilities::InitStagingBuffer( m_context, stagingBuffer.first, stagingBuffer.second, newData, Size );
        VulkanUtilities::CopyBuffer( m_context, stagingBuffer.first, Instance, Size );
        vmaDestroyBuffer( m_context->Vma, stagingBuffer.first, stagingBuffer.second );
    }
    else if ( m_createInfo.KeepMemoryMapped )
    {
        memcpy( mappedMemory, newData, Size );
    }
    else
    {
        vmaMapMemory( m_context->Vma, m_allocation, &mappedMemory );
        memcpy( mappedMemory, newData, Size );
        vmaUnmapMemory( m_context->Vma, m_allocation );
    }
}

void VulkanBufferResource::Deallocate()
{
    m_alreadyAllocated = false;
    RETURN_IF( m_alreadyDisposed );
    m_alreadyDisposed = true;

    if ( m_createInfo.KeepMemoryMapped )
    {
        vmaUnmapMemory( m_context->Vma, m_allocation );
    }

    vmaDestroyBuffer( m_context->Vma, Instance, m_allocation );
}

VulkanBufferResource::~VulkanBufferResource()
{
    VulkanBufferResource::Deallocate();
}

#endif
