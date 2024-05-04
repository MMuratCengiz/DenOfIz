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

#include <DenOfIzGraphics/Backends/Vulkan/VulkanContext.h>
#include <DenOfIzCore/Utilities.h>
#include <DenOfIzCore/Logger.h>
#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanEnumConverter.h>

namespace DenOfIz
{

class VulkanPipeline
{
private:
	const std::array<vk::DynamicState, 4> dynamicStates = {
			vk::DynamicState::eViewportWithCount,
			vk::DynamicState::eDepthBias,
			vk::DynamicState::eScissorWithCount,
			vk::DynamicState::eLineWidth
	};

	VulkanContext* context;
	std::vector<vk::ShaderModule> shaderModules;
	std::vector<vk::PushConstantRange> pushConstants;
	std::vector<vk::Format> colorFormats;
	std::vector<vk::DescriptorSetLayout> layouts;

	PipelineCreateInfo createInfo;
	// Storing these here
	std::vector<vk::PipelineShaderStageCreateInfo> pipelineStageCreateInfos;
	std::vector<vk::PipelineColorBlendAttachmentState> colorBlendAttachments{};
	vk::PipelineRenderingCreateInfo renderingCreateInfo{};
	vk::PipelineTessellationStateCreateInfo tessellationStateCreateInfo{};
	vk::GraphicsPipelineCreateInfo pipelineCreateInfo{};
	vk::PipelineColorBlendStateCreateInfo colorBlending{};
	vk::PipelineRasterizationStateCreateInfo rasterizationStateCreateInfo{};
	vk::PipelineViewportStateCreateInfo viewportStateCreateInfo{};
	vk::PipelineMultisampleStateCreateInfo multisampleStateCreateInfo{};
	vk::PipelineDynamicStateCreateInfo dynamicStateCreateInfo{};
	vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
	vk::PipelineVertexInputStateCreateInfo inputStateCreateInfo{};
	vk::PipelineInputAssemblyStateCreateInfo inputAssemblyCreateInfo{};
	vk::PipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo{};

	std::vector<vk::VertexInputAttributeDescription> vertexAttributeDescriptions;
	std::vector<vk::VertexInputBindingDescription> inputBindingDescriptions;

	std::unordered_map<std::string, vk::WriteDescriptorSet> descriptorSets;
public:
	vk::Pipeline Instance;
	vk::PipelineLayout Layout;
	vk::PipelineBindPoint BindPoint;

	vk::WriteDescriptorSet GetWriteDescriptorSet(const std::string& name);
	VulkanPipeline(VulkanContext* context, const PipelineCreateInfo&);
	~VulkanPipeline();
private:
	bool AlreadyDisposed = false;
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
	vk::ShaderModule CreateShaderModule(std::vector<uint32_t> data);
};

}
