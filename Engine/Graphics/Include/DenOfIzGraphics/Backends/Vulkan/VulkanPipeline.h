/*
Blazar Engine - 3D Game Engine
Copyright (c) 2020-2021 Muhammed Murat Cengiz

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

#include <DenOfIzCore/Utilities.h>
#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanContext.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanEnumConverter.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanInputLayout.h>

namespace DenOfIz
{

    class VulkanPipeline final : public IPipeline
    {
        static const std::array<VkDynamicState, 4> g_dynamicStates;

        VulkanContext *m_context;

        std::vector<VkShaderModule>        m_shaderModules;
        std::vector<VkDescriptorSetLayout> m_layouts;

        PipelineDesc m_desc;

        std::unordered_map<std::string, VkWriteDescriptorSet> m_descriptorSets;

        VkPipeline          m_instance{ };
        VkPipelineLayout    m_layout{ };
        VkPipelineBindPoint m_bindPoint{ };

    public:
        [[nodiscard]] VkPipeline Instance( ) const
        {
            return m_instance;
        }
        [[nodiscard]] VkPipelineLayout Layout( ) const
        {
            return m_layout;
        }
        [[nodiscard]] VkPipelineBindPoint BindPoint( ) const
        {
            return m_bindPoint;
        }
        [[nodiscard]] VkWriteDescriptorSet GetWriteDescriptorSet( const std::string &name );
        VulkanPipeline( VulkanContext *context, const PipelineDesc & );
        ~VulkanPipeline( ) override;

    private:
        void                                                       CreateGraphicsPipeline( );
        void                                                       CreateComputePipeline( );
        [[nodiscard]] std::vector<VkPipelineShaderStageCreateInfo> ConfigurePipelineStages( );
        [[nodiscard]] VkPipelineRenderingCreateInfo                ConfigureRenderingInfo( std::vector<VkFormat> &colorAttachmentsStore ) const;
        [[nodiscard]] VkPipelineTessellationStateCreateInfo        ConfigureTessellation( ) const;
        [[nodiscard]] VkPipelineInputAssemblyStateCreateInfo       ConfigureInputAssembly( ) const;
        [[nodiscard]] VkPipelineVertexInputStateCreateInfo         ConfigureVertexInputState( ) const;
        [[nodiscard]] VkPipelineShaderStageCreateInfo              ConfigureShaderStage( const ShaderDesc &shaderDesc ) const;
        [[nodiscard]] VkPipelineLayoutCreateInfo                   ConfigurePipelineLayout( ) const;
        [[nodiscard]] VkPipelineColorBlendStateCreateInfo          ConfigureColorBlend( std::vector<VkPipelineColorBlendAttachmentState> &colorBlendAttachments ) const;
        [[nodiscard]] VkPipelineRasterizationStateCreateInfo       ConfigureRasterization( ) const;
        [[nodiscard]] VkPipelineViewportStateCreateInfo            ConfigureViewport( ) const;
        [[nodiscard]] VkPipelineMultisampleStateCreateInfo         ConfigureMultisampling( ) const;
        void                                                       CreatePipelineLayout( );
        [[nodiscard]] VkPipelineDepthStencilStateCreateInfo        CreateDepthAttachmentImages( ) const;
        VkShaderModule                                             CreateShaderModule( IDxcBlob *blob ) const;
    };

} // namespace DenOfIz
