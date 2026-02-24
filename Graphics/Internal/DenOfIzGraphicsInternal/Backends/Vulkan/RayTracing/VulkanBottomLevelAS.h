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

#include "DenOfIzGraphicsInternal/Backends/Interface/RayTracing/IBottomLevelAS.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanBuffer.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanContext.h"

namespace DenOfIz
{
    class VulkanBottomLevelAS final : public IBottomLevelAS
    {
        VulkanContext                                                *m_context;
        VkAccelerationStructureKHR                                    m_accelerationStructure;
        DenOfIz_BottomLevelASDesc                                     m_desc;
        std::vector<VkAccelerationStructureGeometryKHR>               m_geometryDescs;
        std::vector<VkAccelerationStructureBuildRangeInfoKHR>         m_buildRangeInfos;    // More convenient for memory management
        std::vector<const VkAccelerationStructureBuildRangeInfoKHR *> m_buildRangeInfoPtrs; // Required for vkCmdBuildAccelerationStructuresKHR
        std::unique_ptr<VulkanBuffer>                                 m_asBuffer;
        std::unique_ptr<VulkanBuffer>                                 m_scratchBuffer;
        VkBuildAccelerationStructureFlagsKHR                          m_flags;

    public:
        VulkanBottomLevelAS( VulkanContext *context, const DenOfIz_BottomLevelASDesc &desc );
        ~VulkanBottomLevelAS( ) override;

        [[nodiscard]] const VkAccelerationStructureKHR                      &Instance( ) const;
        [[nodiscard]] const std::vector<VkAccelerationStructureGeometryKHR> &GeometryDescs( ) const;
        [[nodiscard]] const VkAccelerationStructureBuildRangeInfoKHR *const *BuildRangeInfos( ) const;
        [[nodiscard]] const VkBuildAccelerationStructureFlagsKHR            &Flags( ) const;
        [[nodiscard]] const VulkanBuffer                                    *ScratchBuffer( ) const;
        uint64_t                                                             DeviceAddress( ) const;
        size_t                                                               BuildNumBytes( ) const override;

    private:
        void InitializeTriangles( const DenOfIz_ASGeometryTriangleDesc &triangle, VkAccelerationStructureGeometryKHR &vkGeometry ) const;
        void InitializeAABBs( const DenOfIz_ASGeometryAABBDesc &aabb, VkAccelerationStructureGeometryKHR &vkGeometry ) const;
    };
} // namespace DenOfIz
