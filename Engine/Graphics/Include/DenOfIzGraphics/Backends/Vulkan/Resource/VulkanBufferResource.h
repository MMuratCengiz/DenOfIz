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
#include <DenOfIzGraphics/Backends/Interface/IResource.h>

namespace DenOfIz
{

    class VulkanBufferResource final : public IBufferResource, boost::noncopyable
    {
        BufferCreateInfo m_CreateInfo;
        VulkanContext *m_Context;

        VmaAllocation m_Allocation;
        bool m_AlreadyDisposed = false;
        bool m_AlreadyAllocated = false;

    public:
        vk::Buffer Instance;
        vk::DescriptorBufferInfo DescriptorInfo;

        explicit VulkanBufferResource( VulkanContext *context, const BufferCreateInfo &createInfo );

        void Allocate( const void *newData ) override;
        void Deallocate() override;
        virtual ~VulkanBufferResource();
        void UpdateAllocation( const void *newData );

    };

}

#endif
