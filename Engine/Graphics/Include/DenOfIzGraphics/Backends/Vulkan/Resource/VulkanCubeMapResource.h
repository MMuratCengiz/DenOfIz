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
#include "VulkanSamplerResource.h"

namespace DenOfIz
{

    class VulkanCubeMapResource final : ICubeMapResource, boost::noncopyable
    {
        VulkanContext *m_Context;
        CubeMapCreateInfo m_CreateInfo;

        vk::Sampler m_Sampler{};
        vk::ImageView m_ImageView{};
        vk::Image m_Image;
        VmaAllocation m_Allocation;

    public:
        explicit VulkanCubeMapResource( VulkanContext *context, const CubeMapCreateInfo &createInfo );

        void Allocate( std::vector<const void *> data ) override;
        void Deallocate() override;
    };

}

#endif
