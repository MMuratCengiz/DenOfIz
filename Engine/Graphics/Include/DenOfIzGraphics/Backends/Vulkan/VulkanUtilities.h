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

#include "VulkanContext.h"
#include "../Interface/IResource.h"

namespace DenOfIz
{

class VulkanUtilities
{
private:
	VulkanUtilities() = default;
public:
	static void InitStagingBuffer(VulkanContext* context, vk::Buffer& buffer, VmaAllocation& allocation, const void* data, const uint64_t& size);
	static void RunOneTimeCommand(VulkanContext* context, std::function<void(vk::CommandBuffer&)> run);
	static void CopyBuffer(VulkanContext* context, vk::Buffer& from, vk::Buffer& to, uint32_t size);
};

}