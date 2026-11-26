/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <array>
#include "DenOfIzGraphicsInternal/Backends/Interface/RayTracing/ITopLevelAS.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanBuffer.h"

namespace DenOfIz
{
    class VulkanTopLevelAS final : public ITopLevelAS
    {
        VulkanContext                                                  *m_context;
        VkAccelerationStructureKHR                                      m_accelerationStructure{ };
        VkDeviceMemory                                                  m_memory{ };
        VkBuildAccelerationStructureFlagsKHR                            m_flags;
        std::vector<VkAccelerationStructureInstanceKHR>                 m_instances;
        VkAccelerationStructureBuildRangeInfoKHR                        m_buildRangeInfo;
        std::array<const VkAccelerationStructureBuildRangeInfoKHR *, 1> m_buildRangeInfoPtr;
        std::unique_ptr<VulkanBuffer>                                   m_instanceBuffer;
        std::unique_ptr<VulkanBuffer>                                   m_buffer;
        std::unique_ptr<VulkanBuffer>                                   m_scratch;
        VkAccelerationStructureGeometryKHR                              m_buildGeometryInfo;

    public:
        VulkanTopLevelAS( VulkanContext *context, const DenOfIz_TopLevelASDesc &desc );
        void                                                                 UpdateInstanceTransforms( const DenOfIz_UpdateTransformsDesc &desc ) override;
        [[nodiscard]] VkBuildAccelerationStructureFlagsKHR                   Flags( ) const;
        [[nodiscard]] size_t                                                 NumInstances( ) const;
        [[nodiscard]] const VkAccelerationStructureKHR                      &Instance( ) const;
        [[nodiscard]] const VkAccelerationStructureGeometryKHR              *GeometryDesc( ) const;
        [[nodiscard]] const VkAccelerationStructureBuildRangeInfoKHR *const *BuildRangeInfo( ) const;
        [[nodiscard]] const VulkanBuffer                                    *InstanceBuffer( ) const;
        [[nodiscard]] VulkanBuffer                                          *GetVulkanBuffer( ) const;
        [[nodiscard]] const VulkanBuffer                                    *Scratch( ) const;
        size_t                                                               BuildNumBytes( ) const override;
        ~VulkanTopLevelAS( ) override;
    };
} // namespace DenOfIz
