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
#include "DenOfIzGraphics/Backends/Common/ShaderReflection.h"

namespace DenOfIz
{

    class VulkanPipeline : public IPipeline
    {
        bool                                  m_alreadyDisposed = false;
        const std::array<vk::DynamicState, 4> m_dynamicStates   = { vk::DynamicState::eViewportWithCount, vk::DynamicState::eDepthBias, vk::DynamicState::eScissorWithCount,
                                                                    vk::DynamicState::eLineWidth };

        VulkanContext   *m_context;
        ShaderReflection m_programReflection;

        std::vector<vk::ShaderModule>        m_shaderModules;
        std::vector<vk::PushConstantRange>   m_pushConstants;
        std::vector<vk::Format>              m_colorFormats;
        std::vector<vk::DescriptorSetLayout> m_layouts;

        PipelineDesc m_desc;
        // Storing these here
        std::vector<vk::PipelineShaderStageCreateInfo>     m_pipelineStageCreateInfos;
        std::vector<vk::PipelineColorBlendAttachmentState> m_colorBlendAttachments{};
        vk::PipelineRenderingCreateInfo                    m_renderingCreateInfo{};
        vk::PipelineTessellationStateCreateInfo            m_tessellationStateCreateInfo{};
        vk::GraphicsPipelineCreateInfo                     m_pipelineCreateInfo{};
        vk::PipelineColorBlendStateCreateInfo              m_colorBlending{};
        vk::PipelineRasterizationStateCreateInfo           m_rasterizationStateCreateInfo{};
        vk::PipelineViewportStateCreateInfo                m_viewportStateCreateInfo{};
        vk::PipelineMultisampleStateCreateInfo             m_multisampleStateCreateInfo{};
        vk::PipelineDynamicStateCreateInfo                 m_dynamicStateCreateInfo{};
        vk::PipelineLayoutCreateInfo                       m_pipelineLayoutCreateInfo{};
        vk::PipelineVertexInputStateCreateInfo             m_inputStateCreateInfo{};
        vk::PipelineInputAssemblyStateCreateInfo           m_inputAssemblyCreateInfo{};
        vk::PipelineDepthStencilStateCreateInfo            m_depthStencilStateCreateInfo{};

        std::vector<vk::VertexInputAttributeDescription> m_vertexAttributeDescriptions;
        std::vector<vk::VertexInputBindingDescription>   m_inputBindingDescriptions;

        std::unordered_map<std::string, vk::WriteDescriptorSet> m_descriptorSets;

    public:
        vk::Pipeline          Instance;
        vk::PipelineLayout    Layout;
        vk::PipelineBindPoint BindPoint;

        vk::WriteDescriptorSet GetWriteDescriptorSet(const std::string &name);
        VulkanPipeline(VulkanContext *context, const PipelineDesc &);
        ~VulkanPipeline() override;

    private:
        void             ConfigureVertexInput();
        void             ConfigureColorBlend();
        void             ConfigureRasterization();
        void             ConfigureViewport();
        void             ConfigureMultisampling();
        void             ConfigureDynamicState();
        void             CreatePipelineLayout();
        void             CreateRenderPass();
        void             CreateDepthAttachmentImages();
        void             CreatePipeline();
        vk::ShaderModule CreateShaderModule(IDxcBlob* blob) const;
    };

} // namespace DenOfIz
