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

#include <DenOfIzGraphics/Backends/Interface/RayTracing/ITopLevelAS.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanBufferResource.h>

namespace DenOfIz
{
    class VulkanTopLevelAS : public ITopLevelAS
    {
    private:
        VulkanContext                                  *m_context;
        VkAccelerationStructureKHR                      m_accelerationStructure{};
        VkDeviceMemory                                  m_memory{};
        VkBuildAccelerationStructureFlagsKHR            m_flags;
        std::vector<VkAccelerationStructureInstanceKHR> m_instances;
        std::unique_ptr<VulkanBufferResource>           m_instanceBuffer;
        std::unique_ptr<VulkanBufferResource>           m_buffer;
        std::unique_ptr<VulkanBufferResource>           m_scratch;

    public:
                                                           VulkanTopLevelAS( VulkanContext *context, const TopLevelASDesc &desc );
        [[nodiscard]] VkBuildAccelerationStructureFlagsKHR Flags( ) const;
        [[nodiscard]] size_t                               NumInstances( ) const;
        [[nodiscard]] const VulkanBufferResource          *InstanceBuffer( ) const;
        [[nodiscard]] VulkanBufferResource                *VulkanBuffer( ) const;
        [[nodiscard]] IBufferResource                     *Buffer( ) const override;
        [[nodiscard]] const VulkanBufferResource          *Scratch( ) const;
        ~                                                  VulkanTopLevelAS( ) override;
    };
} // namespace DenOfIz
