/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/IRootSignature.h>
#include "VulkanBindGroupLayout.h"

namespace DenOfIz
{
    class VulkanRootSignature final : public IRootSignature
    {
        DenOfIz_RootSignatureDesc m_desc;
        VulkanContext            *m_context = nullptr;

        std::vector<VulkanBindGroupLayout *> m_bindGroupLayouts;
        std::vector<VkDescriptorSetLayout>   m_layouts;
        std::vector<VkPushConstantRange>     m_pushConstants;
        VkPipelineLayout                     m_pipelineLayout{ };
        VkDescriptorSetLayout                m_emptyLayout{ };

    public:
        VulkanRootSignature( VulkanContext *context, DenOfIz_RootSignatureDesc desc );
        ~VulkanRootSignature( ) override;

        [[nodiscard]] uint32_t                                    NumRootConstants( ) const;
        [[nodiscard]] std::vector<VkPushConstantRange>            PushConstantRanges( ) const;
        [[nodiscard]] VkPushConstantRange                         PushConstantRange( const uint32_t binding ) const;
        [[nodiscard]] std::vector<VkDescriptorSetLayout>          DescriptorSetLayouts( ) const;
        [[nodiscard]] const VkDescriptorSetLayout                &DescriptorSetLayout( const uint32_t registerSpace ) const;
        [[nodiscard]] VkPipelineLayout                            PipelineLayout( ) const;
        [[nodiscard]] VkDescriptorSetLayout                       EmptyLayout( ) const;
        [[nodiscard]] const std::vector<VulkanBindGroupLayout *> &BindGroupLayouts( ) const;
        [[nodiscard]] VulkanBindGroupLayout                      *GetBindGroupLayout( uint32_t registerSpace ) const;

    private:
        void AddRootConstant( const DenOfIz_RootConstantBindingDesc &rootConstantBinding );
    };
} // namespace DenOfIz
