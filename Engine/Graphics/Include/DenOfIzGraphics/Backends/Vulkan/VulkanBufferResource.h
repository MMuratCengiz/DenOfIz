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

#include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanContext.h>

namespace DenOfIz
{

    class VulkanBufferResource final : public IBufferResource, private NonCopyable
    {
        BufferDesc     m_desc;
        VulkanContext *m_context;
        VmaAllocation  m_allocation;
        vk::Buffer     Instance;

    public:
        void                     MapMemory() override;
        void                     CopyData(const void *data, uint32_t size) override;
        void *                   ReadData() override;
        void                     UnmapMemory() override;
        vk::DescriptorBufferInfo DescriptorInfo;

        explicit VulkanBufferResource(VulkanContext *context, const BufferDesc &desc);
        ~VulkanBufferResource() override;
        void                     UpdateAllocation(const void *newData);
        inline const vk::Buffer &GetBuffer() const
        {
            return Instance;
        }
    };

} // namespace DenOfIz

#endif
