/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphicsInternal/Backends/Interface/RayTracing/ILocalRootSignature.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanContext.h"

namespace DenOfIz
{
    struct VkLayoutWithSet
    {
        VkDescriptorSetLayout Layout;
        uint32_t              Set;
    };

    // Vulkan merges the local root signatures into one in VulkanPipeline::CreateRayTracingPipeline( ), a layout created here with create = true
    // is never used.
    class VulkanLocalRootSignature final : public ILocalRootSignature
    {
        VulkanContext                 *m_context;
        DenOfIz_LocalRootSignatureDesc m_desc;
        VkDescriptorSetLayout          m_descriptorSetLayout = nullptr;

        uint32_t m_totalInlineDataBytes = 0;

        std::vector<VkLayoutWithSet>                                            m_layouts;
        std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> m_layoutBindings;

        std::vector<uint32_t> m_inlineDataOffsets;
        std::vector<uint32_t> m_inlineDataNumBytes;

    public:
        VulkanLocalRootSignature( VulkanContext *context, const DenOfIz_LocalRootSignatureDesc &desc, bool create = true );
        void Merge( const VulkanLocalRootSignature &other );
        void Create( );
        ~VulkanLocalRootSignature( ) override;

        [[nodiscard]] std::vector<VkLayoutWithSet> DescriptorSetLayouts( );
        [[nodiscard]] VkDescriptorSetLayout       *DescriptorSetLayout( );
        [[nodiscard]] uint32_t                     LocalDataNumBytes( ) const;
        [[nodiscard]] uint32_t                     InlineDataNumBytes( ) const;
        [[nodiscard]] uint32_t                     CbvOffset( uint32_t cbvIndex ) const;
        [[nodiscard]] uint32_t                     CbvNumBytes( uint32_t cbvIndex ) const;
    };
} // namespace DenOfIz
