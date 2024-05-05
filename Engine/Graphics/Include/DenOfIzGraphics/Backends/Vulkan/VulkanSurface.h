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

#include "VulkanContext.h"
#include "VulkanUtilities.h"

namespace DenOfIz
{

    class VulkanSurface
    {
        VulkanContext *m_Context;

    public:
        explicit VulkanSurface( VulkanContext *context );

        ~VulkanSurface();

    private:
        void CreateSurface() const;
        void CreateSwapChain( const vk::SurfaceCapabilitiesKHR &surfaceCapabilities ) const;
        void CreateImageView( vk::ImageView &imageView, const vk::Image &image, const vk::Format &format, const vk::ImageAspectFlags &aspectFlags ) const;
        void ChooseExtent2D( const vk::SurfaceCapabilitiesKHR &capabilities ) const;
        void CreateSwapChainImages( vk::Format format ) const;
        void Dispose() const;
    };

}

#endif
