/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphicsInternal/Backends/Interface/RayTracing/IShaderLocalData.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/RayTracing/VulkanLocalRootSignature.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanContext.h"
#include "DenOfIzGraphicsInternal/Utilities/Storage.h"

namespace DenOfIz
{
    class VulkanShaderLocalData final : public IShaderLocalData
    {
        VulkanContext                    *m_context{ };
        DenOfIz_ShaderLocalDataDesc       m_desc;
        VulkanLocalRootSignature         *m_layout;
        std::vector<Byte>                 m_data;
        Storage                           m_storage{ };
        std::vector<VkWriteDescriptorSet> m_writeDescriptorSets{ };
        std::vector<uint8_t>              m_inlineData;
        VkDescriptorSet                   m_descriptorSet = nullptr;

    public:
        VulkanShaderLocalData( VulkanContext *context, const DenOfIz_ShaderLocalDataDesc &desc );
        ~VulkanShaderLocalData( ) override;

        void Begin( ) override;
        void Cbv( const uint32_t binding, IBuffer *bufferResource ) override;
        void Cbv( const uint32_t binding, const DenOfIz_ByteArrayView &data ) override;
        void Srv( const uint32_t binding, const IBuffer *bufferResource ) override;
        void Srv( const uint32_t binding, const ITexture *textureResource ) override;
        void Uav( const uint32_t binding, const IBuffer *bufferResource ) override;
        void Uav( const uint32_t binding, const ITexture *textureResource ) override;
        void Sampler( uint32_t binding, const ISampler *sampler ) override;
        void End( ) override;

        [[nodiscard]] const VkDescriptorSet *DescriptorSet( ) const;
        [[nodiscard]] uint32_t               DataNumBytes( ) const;
        [[nodiscard]] const Byte            *Data( ) const;

    private:
        VkWriteDescriptorSet &CreateWriteDescriptor( uint32_t binding, VkDescriptorType type );
    };
} // namespace DenOfIz
