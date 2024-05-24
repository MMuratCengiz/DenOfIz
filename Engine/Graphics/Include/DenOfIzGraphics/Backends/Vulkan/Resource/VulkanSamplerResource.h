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

#pragma once
#ifdef BUILD_VK

#include <DenOfIzGraphics/Backends/Vulkan/VulkanContext.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanUtilities.h>

namespace DenOfIz
{

struct VulkanImageCreateInfo
{
	vk::Format Format{};
	vk::ImageUsageFlags Usage{};
	vk::ImageAspectFlags Aspect{};
	vk::SampleCountFlagBits SampleCount{};
	uint32_t Width = 0; // 0 means as wide as the render window
	uint32_t Height = 0; // 0 means as tall as the render window
};

struct VulkanImage
{
	vk::Sampler Sampler{};
	vk::ImageView ImageView{};
	vk::Image Instance{};
	VmaAllocation Allocation{};

	void Create(VulkanContext* context, const VulkanImageCreateInfo& createInfo);
	void Dispose(const VulkanContext* context) const;
};

class VulkanSamplerResource final : public IImageResource, boost::noncopyable
{
	SamplerCreateInfo m_createInfo;
	VulkanContext* m_context;
	VulkanImage m_image{};
	uint32_t m_mipLevels{};

public:
	vk::DescriptorImageInfo DescriptorInfo;

	explicit VulkanSamplerResource(VulkanContext* context, const SamplerCreateInfo& createInfo);

	void Allocate(const void* newImage) override;
	void Deallocate() override;

private:
	vk::SamplerCreateInfo GetSamplerCreateInfo() const;
	void GenerateMipMaps() const;
};

}

#endif
