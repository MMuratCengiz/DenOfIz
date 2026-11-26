/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphicsInternal/Backends/Common/ShaderBindingTableDebugData.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/RayTracing/IShaderBindingTable.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanBuffer.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanPipeline.h"

namespace DenOfIz
{
    class VulkanShaderLocalData;

    class VulkanShaderBindingTable final : public IShaderBindingTable
    {
        VulkanContext                 *m_context;
        VulkanPipeline                *m_pipeline;
        DenOfIz_ShaderBindingTableDesc m_desc;
        size_t                         m_numBufferBytes{ };
        void                          *m_mappedMemory{ };
        std::unique_ptr<VulkanBuffer>  m_stagingBuffer;
        std::unique_ptr<VulkanBuffer>  m_buffer;

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

        ShaderBindingTableDebugData m_debugData;

    public:
        VulkanShaderBindingTable( VulkanContext *context, const DenOfIz_ShaderBindingTableDesc &desc );
        void                        Resize( const DenOfIz_SBTSizeDesc & ) override;
        void                        BindRayGenerationShader( const DenOfIz_RayGenerationBindingDesc &desc ) override;
        void                        BindHitGroup( const DenOfIz_HitGroupBindingDesc &desc ) override;
        void                        BindMissShader( const DenOfIz_MissBindingDesc &desc ) override;
        void                        Build( ) override;
        [[nodiscard]] VulkanBuffer *GetVulkanBuffer( ) const;

        [[nodiscard]] const VkStridedDeviceAddressRegionKHR *RayGenerationShaderRange( ) const;
        [[nodiscard]] const VkStridedDeviceAddressRegionKHR *MissShaderRange( ) const;
        [[nodiscard]] const VkStridedDeviceAddressRegionKHR *HitGroupShaderRange( ) const;
        [[nodiscard]] const VkStridedDeviceAddressRegionKHR *CallableShaderRange( ) const;

    private:
        [[nodiscard]] uint32_t AlignRecord( uint32_t size ) const;
        void                   EncodeData( void *entry, VulkanShaderLocalData *data ) const;
    };
} // namespace DenOfIz
