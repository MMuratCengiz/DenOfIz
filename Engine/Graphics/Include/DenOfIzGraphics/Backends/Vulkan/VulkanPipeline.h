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
#ifdef BUILD_VK

#include <DenOfIzGraphics/Backends/Vulkan/VulkanContext.h>
#include <DenOfIzCore/Utilities.h>
#include <DenOfIzCore/Logger.h>
#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanEnumConverter.h>

namespace DenOfIz
{

    class VulkanPipeline
    {
        bool m_AlreadyDisposed = false;
        const std::array<vk::DynamicState, 4> m_DynamicStates = { vk::DynamicState::eViewportWithCount, vk::DynamicState::eDepthBias, vk::DynamicState::eScissorWithCount,
                                                                vk::DynamicState::eLineWidth };

        VulkanContext *m_Context;
        std::vector<vk::ShaderModule> m_ShaderModules;
        std::vector<vk::PushConstantRange> m_PushConstants;
        std::vector<vk::Format> m_ColorFormats;
        std::vector<vk::DescriptorSetLayout> m_Layouts;

        PipelineCreateInfo m_CreateInfo;
        // Storing these here
        std::vector<vk::PipelineShaderStageCreateInfo> m_PipelineStageCreateInfos;
        std::vector<vk::PipelineColorBlendAttachmentState> m_ColorBlendAttachments{};
        vk::PipelineRenderingCreateInfo m_RenderingCreateInfo{};
        vk::PipelineTessellationStateCreateInfo m_TessellationStateCreateInfo{};
        vk::GraphicsPipelineCreateInfo m_PipelineCreateInfo{};
        vk::PipelineColorBlendStateCreateInfo m_ColorBlending{};
        vk::PipelineRasterizationStateCreateInfo m_RasterizationStateCreateInfo{};
        vk::PipelineViewportStateCreateInfo m_ViewportStateCreateInfo{};
        vk::PipelineMultisampleStateCreateInfo m_MultisampleStateCreateInfo{};
        vk::PipelineDynamicStateCreateInfo m_DynamicStateCreateInfo{};
        vk::PipelineLayoutCreateInfo m_PipelineLayoutCreateInfo{};
        vk::PipelineVertexInputStateCreateInfo m_InputStateCreateInfo{};
        vk::PipelineInputAssemblyStateCreateInfo m_InputAssemblyCreateInfo{};
        vk::PipelineDepthStencilStateCreateInfo m_DepthStencilStateCreateInfo{};

        std::vector<vk::VertexInputAttributeDescription> m_VertexAttributeDescriptions;
        std::vector<vk::VertexInputBindingDescription> m_InputBindingDescriptions;

        std::unordered_map<std::string, vk::WriteDescriptorSet> m_DescriptorSets;

    public:
        vk::Pipeline Instance;
        vk::PipelineLayout Layout;
        vk::PipelineBindPoint BindPoint;

        vk::WriteDescriptorSet GetWriteDescriptorSet( const std::string &name );
        VulkanPipeline( VulkanContext *context, const PipelineCreateInfo & );
        ~VulkanPipeline();

    private:
        void ConfigureVertexInput();
        void ConfigureColorBlend();
        void ConfigureRasterization();
        void ConfigureViewport();
        void ConfigureMultisampling();
        void ConfigureDynamicState();
        void CreatePipelineLayout();
        void CreateRenderPass();
        void CreateDepthAttachmentImages();
        void CreatePipeline();
        vk::ShaderModule CreateShaderModule( const std::vector<uint32_t> &data ) const;
    };

}

#endif
