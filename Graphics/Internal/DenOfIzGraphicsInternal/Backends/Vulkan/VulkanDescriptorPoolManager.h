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

#include <DenOfIzGraphics/Utilities/Common.h>
#include "VulkanInclude.h"
#include <memory>
#include <vector>

namespace DenOfIz
{
    /// TODO This class is extremely crude at this moment, rotating pools and correctly managing them is not yet implemented.
    class VulkanDescriptorPoolManager final
    {
        class VulkanDescriptorPool final
        {
            friend class VulkanDescriptorPoolManager;
            VkDevice         m_device;
            VkDescriptorPool m_pool{};
            uint32_t         m_numSets;
            uint32_t         m_setsAllocated = 0;

        public:
            VulkanDescriptorPool( const VkDevice& device, uint32_t numSets );
            ~VulkanDescriptorPool( );
        };

        VkDevice                                           m_device;
        std::vector<std::unique_ptr<VulkanDescriptorPool>> m_pools;
        std::unique_ptr<VulkanDescriptorPool>              m_currentPool;
        const uint32_t                                     m_maxSets = 100;

    public:
        explicit VulkanDescriptorPoolManager( const VkDevice& device );
        ~VulkanDescriptorPoolManager( ) = default;

        /// <param name="allocateInfo">Pool parameter is filled in automatically </param>
        void AllocateDescriptorSets( const VkDescriptorSetAllocateInfo &allocateInfo, VkDescriptorSet *sets );
        void FreeDescriptorSets( uint32_t count, const VkDescriptorSet *sets );
    };
} // namespace DenOfIz
