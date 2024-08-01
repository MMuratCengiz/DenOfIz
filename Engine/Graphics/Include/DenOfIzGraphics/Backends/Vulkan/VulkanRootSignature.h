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
        VulkanContext    *m_context;

        std::vector<VkDescriptorSetLayout>        m_layouts;
        std::vector<VkDescriptorSetLayoutBinding> m_bindings;
        std::vector<VkPushConstantRange>          m_pushConstants;
        std::vector<VkSampler>                    m_staticSamplers;

    public:
         VulkanRootSignature( VulkanContext *context, RootSignatureDesc desc );
        ~VulkanRootSignature( ) override;

        [[nodiscard]] const std::vector<VkDescriptorSetLayout> &GetDescriptorSetLayouts( ) const
        {
            return m_layouts;
        }

    protected:
        void                                              AddResourceBindingInternal( const ResourceBindingDesc &binding ) override;
        void                                              AddRootConstantInternal( const RootConstantResourceBinding &rootConstantBinding ) override;
        [[nodiscard]] static VkDescriptorSetLayoutBinding CreateDescriptorSetLayoutBinding( const ResourceBindingDesc &binding );

    private:
        void AddStaticSampler( const StaticSamplerDesc &sampler );
    };
} // namespace DenOfIz
