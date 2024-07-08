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

void VulkanUtilities::InitStagingBuffer(const VulkanContext *context, vk::Buffer &buffer, VmaAllocation &allocation, const void *data, const uint64_t &size)
{
    vk::BufferCreateInfo stagingBufferCreateInfo{};

    stagingBufferCreateInfo.usage       = vk::BufferUsageFlagBits::eTransferSrc;
    stagingBufferCreateInfo.size        = size;
    stagingBufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;

    VmaAllocationCreateInfo stagingAllocationInfo{};
    stagingAllocationInfo.usage          = VMA_MEMORY_USAGE_CPU_TO_GPU;
    stagingAllocationInfo.requiredFlags  = static_cast<VkMemoryPropertyFlagBits>(vk::MemoryPropertyFlagBits::eHostVisible);
    stagingAllocationInfo.preferredFlags = static_cast<VkMemoryPropertyFlagBits>(vk::MemoryPropertyFlagBits::eHostCoherent);

    vmaCreateBuffer(context->Vma, reinterpret_cast<VkBufferCreateInfo *>(&stagingBufferCreateInfo), &stagingAllocationInfo, reinterpret_cast<VkBuffer *>(&buffer), &allocation,
                    nullptr);

    void *deviceMemory;
    vmaMapMemory(context->Vma, allocation, &deviceMemory);
    memcpy(deviceMemory, data, size);
    vmaUnmapMemory(context->Vma, allocation);
}

void VulkanUtilities::RunOneTimeCommand(const VulkanContext *context, const std::function<void(vk::CommandBuffer &)> &run)
{
    vk::CommandBufferAllocateInfo bufferAllocateInfo{};
    bufferAllocateInfo.level              = vk::CommandBufferLevel::ePrimary;
    bufferAllocateInfo.commandPool        = context->GraphicsQueueCommandPool;
    bufferAllocateInfo.commandBufferCount = 1;

    vk::CommandBuffer buffer = context->LogicalDevice.allocateCommandBuffers(bufferAllocateInfo)[ 0 ];

    vk::CommandBufferBeginInfo beginInfo{};
    beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

    vk::Result result = buffer.begin(&beginInfo);
    VK_CHECK_RESULT(result);

    run(buffer);

    buffer.end();

    vk::SubmitInfo submitInfo{};
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &buffer;

    const vk::Queue queue = context->Queues.at(QueueType::Graphics);
    result                = queue.submit(1, &submitInfo, nullptr);
    VK_CHECK_RESULT(result);
}

void VulkanUtilities::CopyBuffer(const VulkanContext *context, const vk::Buffer &from, const vk::Buffer &to, const uint32_t size)
{
    RunOneTimeCommand(context,
                      [ & ](const vk::CommandBuffer &commandBuffer)
                      {
                          vk::BufferCopy bufferCopy{};
                          bufferCopy.size = size;
                          commandBuffer.copyBuffer(from, to, 1, &bufferCopy);
                      });
}
