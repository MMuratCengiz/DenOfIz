/*
DenOfIz Engine - 3D Game Engine
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
#include <DenOfIzGraphics/Backends/Vulkan/VulkanUtilities.h>

using namespace DenOfIz;

void VulkanUtilities::InitStagingBuffer( const VulkanContext *context, VkBuffer &buffer, VmaAllocation &allocation, const void *data, const uint64_t &size )
{
    VkBufferCreateInfo stagingBufferCreateInfo{ };
    stagingBufferCreateInfo.sType       = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    stagingBufferCreateInfo.usage       = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    stagingBufferCreateInfo.size        = size;
    stagingBufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    VmaAllocationCreateInfo stagingAllocationInfo{ };
    stagingAllocationInfo.usage          = VMA_MEMORY_USAGE_CPU_TO_GPU;
    stagingAllocationInfo.requiredFlags  = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    stagingAllocationInfo.preferredFlags = VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

    vmaCreateBuffer( context->Vma, &stagingBufferCreateInfo, &stagingAllocationInfo, &buffer, &allocation, nullptr );

    void *deviceMemory;
    vmaMapMemory( context->Vma, allocation, &deviceMemory );
    memcpy( deviceMemory, data, size );
    vmaUnmapMemory( context->Vma, allocation );
}

void VulkanUtilities::RunOneTimeCommand( const VulkanContext *context, const std::function<void( VkCommandBuffer & )> &run )
{
}

void VulkanUtilities::CopyBuffer( const VulkanContext *context, const VkBuffer &from, const VkBuffer &to, const uint32_t size )
{
    RunOneTimeCommand( context,
                       [ & ]( const VkCommandBuffer &commandBuffer )
                       {
                           VkBufferCopy bufferCopy{ };
                           bufferCopy.size = size;
                           vkCmdCopyBuffer( commandBuffer, from, to, 1, &bufferCopy );
                       } );
}
