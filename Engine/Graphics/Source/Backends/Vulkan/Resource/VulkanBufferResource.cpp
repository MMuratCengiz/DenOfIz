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

#include <DenOfIzGraphics/Backends/Vulkan/Resource/VulkanBufferResource.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanEnumConverter.h>

using namespace DenOfIz;

VulkanBufferResource::VulkanBufferResource(VulkanContext* context, const BufferCreateInfo& createInfo)
		:context(context), createInfo(createInfo)
{
}

void VulkanBufferResource::Allocate(const void* newData)
{
	Data = newData;
	if (alreadyAllocated)
	{
		UpdateAllocation(newData);
		return;
	}

	alreadyDisposed = false;
	alreadyAllocated = true;

	std::pair<vk::Buffer, VmaAllocation> stagingBuffer;
	Size = createInfo.MemoryCreateInfo.Size;

	if (createInfo.UseStaging)
	{
		VulkanUtilities::InitStagingBuffer(context, stagingBuffer.first, stagingBuffer.second, newData, Size);
	}

	vk::BufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.usage = VulkanEnumConverter::ConvertBufferUsage(createInfo.MemoryCreateInfo.Usage);
	bufferCreateInfo.size = Size;
	bufferCreateInfo.sharingMode = vk::SharingMode::eExclusive;

	VmaAllocationCreateInfo allocationCreateInfo{};

	allocationCreateInfo.usage = VulkanEnumConverter::ConvertMemoryLocation(createInfo.MemoryCreateInfo.Location);

	// Todo more flexibility, or a clearer interface here:
	if (createInfo.MemoryCreateInfo.Location == MemoryLocation::CPU_GPU)
	{
		auto gpuVisible = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eDeviceLocal;
		allocationCreateInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(vk::MemoryPropertyFlagBits::eHostVisible);
		allocationCreateInfo.preferredFlags = static_cast<VkMemoryPropertyFlags>(gpuVisible);
	}
	else
	{
		bufferCreateInfo.usage = bufferCreateInfo.usage | vk::BufferUsageFlagBits::eTransferDst;
		allocationCreateInfo.requiredFlags = static_cast<VkMemoryPropertyFlags>(vk::MemoryPropertyFlagBits::eDeviceLocal);
	}

	VmaAllocationInfo allocationInfo;
	vmaCreateBuffer(context->Vma, (VkBufferCreateInfo*)&bufferCreateInfo, &allocationCreateInfo, (VkBuffer*)&Instance, &allocation, &allocationInfo);

	if (createInfo.UseStaging)
	{
		VulkanUtilities::CopyBuffer(context, stagingBuffer.first, Instance, Size);
		vmaDestroyBuffer(context->Vma, stagingBuffer.first, stagingBuffer.second);
	}
	else if (mappedMemory != nullptr && createInfo.KeepMemoryMapped)
	{
		memcpy(mappedMemory, newData, Size);
	}
	else
	{
		vmaMapMemory(this->context->Vma, allocation, &mappedMemory);
		memcpy(mappedMemory, newData, Size);

		if (!createInfo.KeepMemoryMapped)
		{
			vmaUnmapMemory(context->Vma, allocation);
		}
	}

	DescriptorInfo.buffer = Instance;
	DescriptorInfo.offset = 0;
	DescriptorInfo.range = Size;
}

void VulkanBufferResource::UpdateAllocation(const void* newData)
{
	if (createInfo.UseStaging)
	{
		std::pair<vk::Buffer, VmaAllocation> stagingBuffer;
		VulkanUtilities::InitStagingBuffer(context, stagingBuffer.first, stagingBuffer.second, newData, Size);
		VulkanUtilities::CopyBuffer(context, stagingBuffer.first, Instance, Size);
		vmaDestroyBuffer(context->Vma, stagingBuffer.first, stagingBuffer.second);
	}
	else if (createInfo.KeepMemoryMapped)
	{
		memcpy(mappedMemory, newData, Size);
	}
	else
	{
		vmaMapMemory(context->Vma, allocation, &mappedMemory);
		memcpy(mappedMemory, newData, Size);
		vmaUnmapMemory(context->Vma, allocation);
	}
}

void VulkanBufferResource::Deallocate()
{
	alreadyAllocated = false;
	FUNCTION_BREAK(alreadyDisposed);
	alreadyDisposed = true;

	if (createInfo.KeepMemoryMapped)
	{
		vmaUnmapMemory(context->Vma, allocation);
	}

	vmaDestroyBuffer(context->Vma, Instance, allocation);
}

VulkanBufferResource::~VulkanBufferResource()
{
	VulkanBufferResource::Deallocate();
}