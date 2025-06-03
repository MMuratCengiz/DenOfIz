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

#include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
#include "VulkanEnumConverter.h"

namespace DenOfIz
{
    class VulkanRootSignature final : public IRootSignature
    {
        RootSignatureDesc m_desc;
        VulkanContext    *m_context = nullptr;

        std::vector<VkDescriptorSetLayout>                m_layouts;
        std::vector<VkDescriptorSetLayoutBinding>         m_bindings;
        std::vector<VkPushConstantRange>                  m_pushConstants;
        std::vector<VkSampler>                            m_staticSamplers;
        std::unordered_map<uint32_t, ResourceBindingDesc> m_resourceBindingMap;
        // Stores the layout bindings for each set
        std::unordered_map<uint32_t, std::vector<VkDescriptorSetLayoutBinding>> m_layoutBindings;
        // Weirdly enough, seems to make the most sense here
        VkPipelineLayout      m_pipelineLayout{ };
        VkDescriptorSetLayout m_emptyLayout{ };

    public:
        VulkanRootSignature( VulkanContext *context, RootSignatureDesc desc );
        ~VulkanRootSignature( ) override;

        [[nodiscard]] ResourceBindingDesc              GetVkShiftedBinding( const ResourceBindingSlot &slot ) const;
        [[nodiscard]] uint32_t                         NumRootConstants( ) const;
        [[nodiscard]] std::vector<VkPushConstantRange> PushConstantRanges( ) const;
        [[nodiscard]] VkPushConstantRange              PushConstantRange( const uint32_t binding ) const;
        //! @return VK_NULL_HANDLE if register space is between 0 and max register space. </returns>
        //! @throw std::runtime_error if register space is larger than the max set. </throws>
        [[nodiscard]] std::vector<VkDescriptorSetLayout> DescriptorSetLayouts( ) const;
        [[nodiscard]] const VkDescriptorSetLayout       &DescriptorSetLayout( const uint32_t registerSpace ) const;
        [[nodiscard]] VkPipelineLayout                   PipelineLayout( ) const;
        [[nodiscard]] VkDescriptorSetLayout              EmptyLayout( ) const;
    private:
        void                                       AddResourceBinding( const ResourceBindingDesc &binding );
        void                                       AddRootConstant( const RootConstantResourceBindingDesc &rootConstantBinding );
        void                                       AddStaticSampler( const StaticSamplerDesc &sampler );
        void                                       AddBindlessResource( const BindlessResourceDesc &bindlessResource );
        [[nodiscard]] VkDescriptorSetLayoutBinding CreateDescriptorSetLayoutBinding( const ResourceBindingDesc &binding );
    };
} // namespace DenOfIz
