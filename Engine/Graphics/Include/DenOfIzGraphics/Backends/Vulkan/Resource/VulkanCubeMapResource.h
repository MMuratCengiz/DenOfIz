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

namespace DenOfIz
{

struct CubeMapCreateInfo
{
	std::vector<SamplerCreateInfo> Samplers;
};

class ICubeMapResource : public IResource
{

protected:
	uint32_t size;
	const void* data;

public:
	virtual void Allocate(std::vector<const void*> data) = 0;
	virtual void Deallocate() = 0;

	const ResourceType Type() override
	{
		return ResourceType::CubeMap;
	};
};

class VulkanCubeMapResource final : ICubeMapResource, boost::noncopyable
{
	VulkanContext* m_context;
	CubeMapCreateInfo m_createInfo;

	vk::Sampler m_sampler{};
	vk::ImageView m_imageView{};
	vk::Image m_image;
	VmaAllocation m_allocation;

public:
	explicit VulkanCubeMapResource(VulkanContext* context, const CubeMapCreateInfo& createInfo);

	void Allocate(std::vector<const void*> data) override;
	void Deallocate() override;
};

}

#endif
