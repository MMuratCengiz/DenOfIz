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

#include "DenOfIzGraphics/Backends/Interface/RayTracing/IShaderBindingTable.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanBufferResource.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanPipeline.h"

namespace DenOfIz
{
    class VulkanShaderBindingTable final : public IShaderBindingTable
    {
        VulkanContext                        *m_context;
        VulkanPipeline                       *m_pipeline;
        ShaderBindingTableDesc                m_desc;
        size_t                                m_numBufferBytes{ };
        void                                 *m_mappedMemory{ };
        std::unique_ptr<VulkanBufferResource> m_stagingBuffer;
        std::unique_ptr<VulkanBufferResource> m_buffer;

        VkStridedDeviceAddressRegionKHR m_rayGenerationShaderRange{ };
        VkStridedDeviceAddressRegionKHR m_missShaderRange{ };
        VkStridedDeviceAddressRegionKHR m_hitGroupShaderRange{ };
        VkStridedDeviceAddressRegionKHR m_callableShaderRange{ };

        uint32_t m_missGroupOffset = 0;
        uint32_t m_hitGroupOffset  = 0;

        uint32_t m_shaderGroupHandleSize;
        uint32_t m_rayGenNumBytes    = 0;
        uint32_t m_hitGroupNumBytes  = 0;
        uint32_t m_missGroupNumBytes = 0;

        ShaderBindingTableDebugData        m_debugData;
        std::vector<ShaderRecordDebugData> m_rayGenerationShaderDebugData;
        std::vector<ShaderRecordDebugData> m_missShaderDebugData;
        std::vector<ShaderRecordDebugData> m_hitGroupDebugData;

    public:
        VulkanShaderBindingTable( VulkanContext *context, const ShaderBindingTableDesc &desc );
        void                                Resize( const SBTSizeDesc & ) override;
        void                                BindRayGenerationShader( const RayGenerationBindingDesc &desc ) override;
        void                                BindHitGroup( const HitGroupBindingDesc &desc ) override;
        void                                BindMissShader( const MissBindingDesc &desc ) override;
        void                                Build( ) override;
        [[nodiscard]] VulkanBufferResource *VulkanBuffer( ) const;

        [[nodiscard]] const VkStridedDeviceAddressRegionKHR *RayGenerationShaderRange( ) const;
        [[nodiscard]] const VkStridedDeviceAddressRegionKHR *MissShaderRange( ) const;
        [[nodiscard]] const VkStridedDeviceAddressRegionKHR *HitGroupShaderRange( ) const;
        [[nodiscard]] const VkStridedDeviceAddressRegionKHR *CallableShaderRange( ) const;

    private:
        [[nodiscard]] uint32_t AlignRecord( uint32_t size ) const;
        void                   EncodeData( void *entry, IShaderLocalData *iData ) const;
    };
} // namespace DenOfIz
