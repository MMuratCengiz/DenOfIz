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

#pragma once

#include "../VulkanContext.h"
#include "../../Interface/IResource.h"

namespace DenOfIz
{

class VulkanImageResource : public IImageResource
{
private:
	VulkanContext* m_context;
	ImageCreateInfo m_createInfo;

	bool m_hasSampler = false;
	SamplerCreateInfo m_samplerCreateInfo;

	vk::Image m_image;
	vk::ImageView m_imageView;
	vk::Format m_format;
	vk::Sampler m_sampler;

	VmaAllocation m_allocation;
	uint32_t m_mipLevels{};

	bool m_allocated = false;
public:
	vk::DescriptorImageInfo DescriptorInfo;

	VulkanImageResource(VulkanContext* context, ImageCreateInfo createInfo);
	~VulkanImageResource();

	inline vk::Image GetImage() const { return m_image; }
	inline vk::ImageView GetImageView() const { return m_imageView; }
	inline vk::Format GetFormat() const { return m_format; }
	inline vk::Sampler GetSampler() const { return m_sampler; }

	void AttachSampler(SamplerCreateInfo& info) override;
	void Allocate(const void* data) override;
	void Deallocate() override;
private:
	void GenerateMipMaps() const;
};

}