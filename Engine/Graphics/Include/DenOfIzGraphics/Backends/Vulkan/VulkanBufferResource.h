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

#include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanContext.h>

namespace DenOfIz
{

    class VulkanBufferResource final : public IBufferResource, private NonCopyable
    {
        BufferDesc     m_desc{};
        VulkanContext *m_context = nullptr;
        VmaAllocation  m_allocation{ };
        VkBuffer       m_instance{ };

    public:
        void                  *MapMemory( ) override;
        void                   UnmapMemory( ) override;
        VkDescriptorBufferInfo DescriptorInfo{ };

        explicit                      VulkanBufferResource( VulkanContext *context, const BufferDesc &desc );
        ~                             VulkanBufferResource( ) override;
        void                          UpdateAllocation( const void *newData );
        [[nodiscard]] const VkBuffer &Instance( ) const
        {
            return m_instance;
        }
    };

} // namespace DenOfIz
