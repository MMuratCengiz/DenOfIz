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
#include <DenOfIzGraphics/Backends/Vulkan/VulkanPipeline.h>
#include <ranges>
#include "DenOfIzGraphics/Backends/Vulkan/VulkanRootSignature.h"

using namespace DenOfIz;

VulkanPipeline::VulkanPipeline(VulkanContext* context, const PipelineCreateInfo& createInfo)
		:
		m_context(context), m_createInfo(createInfo), BindPoint(VulkanEnumConverter::ConvertPipelineBindPoint(createInfo.BindPoint))
{
	m_pipelineCreateInfo.pDepthStencilState = nullptr;

	ConfigureVertexInput();
	ConfigureViewport();
	ConfigureRasterization();
	ConfigureMultisampling();
	ConfigureColorBlend();
	ConfigureDynamicState();
	CreatePipelineLayout();
	CreateDepthAttachmentImages();
	CreateRenderPass();
	CreatePipeline();
}

void VulkanPipeline::ConfigureVertexInput()
{
	bool hasTessellationShaders = false;

	for (const auto& [Stage, Data] : m_createInfo.ShaderProgram.Shaders)
	{
		vk::PipelineShaderStageCreateInfo& shaderStageCreateInfo = m_pipelineStageCreateInfos.emplace_back();

		const vk::ShaderStageFlagBits stage = VulkanEnumConverter::ConvertShaderStage(Stage);
		vk::ShaderModule& shaderModule = m_shaderModules.emplace_back(this->CreateShaderModule(Data));

		shaderStageCreateInfo.stage = stage;
		shaderStageCreateInfo.module = shaderModule;
		shaderStageCreateInfo.pName = "main";
		shaderStageCreateInfo.pNext = nullptr;

		hasTessellationShaders = hasTessellationShaders || stage == vk::ShaderStageFlagBits::eTessellationEvaluation;
		hasTessellationShaders = hasTessellationShaders || stage == vk::ShaderStageFlagBits::eTessellationControl;
	}

	auto& program = m_createInfo.ShaderProgram;
	auto& vertexInputs = program.VertexInputs();

	uint32_t offsetIter = 0;
	for (const VertexInput& vertexInput : vertexInputs)
	{
		vk::VertexInputAttributeDescription& desc = m_vertexAttributeDescriptions.emplace_back();

		if (m_createInfo.InterleavedMode)
		{
			desc.binding = 0;
		}
		else
		{
			vk::VertexInputBindingDescription& bindingDesc = m_inputBindingDescriptions.emplace_back();
			bindingDesc.binding = m_inputBindingDescriptions.size() - 1;
			bindingDesc.inputRate = vk::VertexInputRate::eVertex; // TODO investigate later for instanced rendering
			bindingDesc.stride = 0;

			desc.binding = bindingDesc.binding;
		}

		desc.location = vertexInput.Location;
		desc.format = VulkanEnumConverter::ConvertImageFormat(vertexInput.Format);
		desc.offset = vertexInput.Offset;
		offsetIter += vertexInput.Size;
	}

	if (m_createInfo.InterleavedMode)
	{
		vk::VertexInputBindingDescription& bindingDesc = m_inputBindingDescriptions.emplace_back();
		bindingDesc.binding = 0;
		bindingDesc.inputRate = vk::VertexInputRate::eVertex; // TODO investigate later for instanced rendering
		bindingDesc.stride = offsetIter;
	}

	m_inputStateCreateInfo.vertexBindingDescriptionCount = m_inputBindingDescriptions.size();
	m_inputStateCreateInfo.pVertexBindingDescriptions = m_inputBindingDescriptions.data();
	m_inputStateCreateInfo.vertexAttributeDescriptionCount = m_vertexAttributeDescriptions.size();
	m_inputStateCreateInfo.pVertexAttributeDescriptions = m_vertexAttributeDescriptions.data();

	m_inputAssemblyCreateInfo.topology = VulkanEnumConverter::ConvertPrimitiveTopology(m_createInfo.PrimitiveTopology);
	m_inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	m_pipelineCreateInfo.stageCount = static_cast<uint32_t>(m_pipelineStageCreateInfos.size());
	m_pipelineCreateInfo.pStages = m_pipelineStageCreateInfos.data();

	// Todo read patch control points from either pipelineRequest or from GLSLShaderSet
	m_tessellationStateCreateInfo.patchControlPoints = 3;

	if (hasTessellationShaders)
	{
		m_inputAssemblyCreateInfo.topology = vk::PrimitiveTopology::ePatchList;
		m_pipelineCreateInfo.pTessellationState = &m_tessellationStateCreateInfo;
	}

	m_pipelineCreateInfo.pVertexInputState = &m_inputStateCreateInfo;
	m_pipelineCreateInfo.pInputAssemblyState = &m_inputAssemblyCreateInfo;
}

void VulkanPipeline::ConfigureMultisampling()
{
	m_multisampleStateCreateInfo.sampleShadingEnable = VK_TRUE;

	switch (m_createInfo.MSAASampleCount)
	{
	case MSAASampleCount::_0:
		m_multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
	case MSAASampleCount::_1:
		m_multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
		break;
	case MSAASampleCount::_2:
		m_multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e2;
		break;
	case MSAASampleCount::_4:
		m_multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e4;
		break;
	case MSAASampleCount::_8:
		m_multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e8;
		break;
	case MSAASampleCount::_16:
		m_multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e16;
		break;
	case MSAASampleCount::_32:
		m_multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e32;
		break;
	case MSAASampleCount::_64:
		m_multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e64;
		break;
	}

	m_multisampleStateCreateInfo.minSampleShading = 1.0f;
	m_multisampleStateCreateInfo.pSampleMask = nullptr;
	m_multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
	m_multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;
	m_multisampleStateCreateInfo.sampleShadingEnable = VK_TRUE;
	m_multisampleStateCreateInfo.minSampleShading = .2f;

	m_pipelineCreateInfo.pMultisampleState = &m_multisampleStateCreateInfo;
}

void VulkanPipeline::ConfigureViewport()
{
	// Todo test, these are dynamic states
	m_viewportStateCreateInfo.viewportCount = 0;
	m_viewportStateCreateInfo.pViewports = nullptr;
	m_viewportStateCreateInfo.scissorCount = 0;
	m_viewportStateCreateInfo.pScissors = nullptr;
	m_pipelineCreateInfo.pViewportState = &m_viewportStateCreateInfo;
}

void VulkanPipeline::ConfigureRasterization()
{
	m_rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	m_rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	m_rasterizationStateCreateInfo.polygonMode = vk::PolygonMode::eFill;
	m_rasterizationStateCreateInfo.lineWidth = 1.0f;

	switch (m_createInfo.CullMode)
	{
	case CullMode::FrontAndBackFace:
		m_rasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eFrontAndBack;
		break;
	case CullMode::BackFace:
		m_rasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eBack;
		break;
	case CullMode::FrontFace:
		m_rasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eFront;
		break;
	case CullMode::None:
		m_rasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eNone;
		break;
	}

	m_rasterizationStateCreateInfo.frontFace = vk::FrontFace::eCounterClockwise;
	m_rasterizationStateCreateInfo.depthBiasEnable = false; // Todo, test if works with dynamic state
	m_rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
	m_rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
	m_rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

	m_pipelineCreateInfo.pRasterizationState = &m_rasterizationStateCreateInfo;
}

void VulkanPipeline::ConfigureColorBlend()
{
	const uint32_t attachmentCount = m_createInfo.BlendModes.size();
	m_colorBlendAttachments.resize(attachmentCount);

	for (int i = 0; i < attachmentCount; ++i)
	{
		m_colorBlendAttachments[i].colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB |
				vk::ColorComponentFlagBits::eA;

		if (m_createInfo.BlendModes[i] == BlendMode::AlphaBlend)
		{
			m_colorBlendAttachments[i].blendEnable = true;
			m_colorBlendAttachments[i].srcColorBlendFactor = vk::BlendFactor::eOne;
			m_colorBlendAttachments[i].dstColorBlendFactor = vk::BlendFactor::eOne;

			m_colorBlendAttachments[i].srcAlphaBlendFactor = vk::BlendFactor::eSrcAlpha;
			m_colorBlendAttachments[i].dstAlphaBlendFactor = vk::BlendFactor::eDstAlpha;

			m_colorBlendAttachments[i].colorBlendOp = vk::BlendOp::eAdd;
			m_colorBlendAttachments[i].alphaBlendOp = vk::BlendOp::eAdd;
		}
	}

	// This overwrites the above
	m_colorBlending.logicOpEnable = false;
	m_colorBlending.logicOp = vk::LogicOp::eCopy;
	m_colorBlending.attachmentCount = attachmentCount;
	m_colorBlending.pAttachments = m_colorBlendAttachments.data();
	m_colorBlending.blendConstants[0] = 0.0f;
	m_colorBlending.blendConstants[1] = 0.0f;
	m_colorBlending.blendConstants[2] = 0.0f;
	m_colorBlending.blendConstants[3] = 0.0f;
	m_pipelineCreateInfo.pColorBlendState = &m_colorBlending;
}

void VulkanPipeline::ConfigureDynamicState()
{
	m_dynamicStateCreateInfo.dynamicStateCount = m_dynamicStates.size();
	m_dynamicStateCreateInfo.pDynamicStates = m_dynamicStates.data();
	m_pipelineCreateInfo.pDynamicState = &m_dynamicStateCreateInfo;
}

void VulkanPipeline::CreatePipelineLayout()
{
	VulkanRootSignature* vulkanRootSignature = dynamic_cast<VulkanRootSignature*>(m_createInfo.RootSignature);
	m_pipelineLayoutCreateInfo.setSetLayouts(vulkanRootSignature->GetDescriptorSetLayouts());

	for (const PushConstant& pushConstant : m_createInfo.ShaderProgram.PushConstants())
	{
		vk::PushConstantRange& pushConstantRange = m_pushConstants.emplace_back();
		pushConstantRange.stageFlags = VulkanEnumConverter::ConvertShaderStage(pushConstant.Stage);
		pushConstantRange.offset = pushConstant.Offset;
		pushConstantRange.size = pushConstant.Size;
	}

	m_pipelineLayoutCreateInfo.pushConstantRangeCount = m_pushConstants.size();
	m_pipelineLayoutCreateInfo.pPushConstantRanges = m_pushConstants.data();

	Layout = m_context->LogicalDevice.createPipelineLayout(m_pipelineLayoutCreateInfo);
	m_pipelineCreateInfo.layout = Layout;
}

void VulkanPipeline::CreateRenderPass()
{
	m_pipelineCreateInfo.renderPass = nullptr;
	m_pipelineCreateInfo.subpass = 0;
	m_pipelineCreateInfo.basePipelineHandle = nullptr;
	m_pipelineCreateInfo.basePipelineIndex = -1;
}

void VulkanPipeline::CreateDepthAttachmentImages()
{
	m_depthStencilStateCreateInfo.depthTestEnable = m_createInfo.DepthTest.Enable;
	m_depthStencilStateCreateInfo.depthWriteEnable = m_createInfo.DepthTest.Write;
	m_depthStencilStateCreateInfo.depthCompareOp = VulkanEnumConverter::ConvertCompareOp(m_createInfo.DepthTest.CompareOp);
	m_depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	m_depthStencilStateCreateInfo.minDepthBounds = 0.0f;
	m_depthStencilStateCreateInfo.maxDepthBounds = 1.0f;

	m_depthStencilStateCreateInfo.stencilTestEnable = m_createInfo.StencilTest.Enable;

	m_depthStencilStateCreateInfo.front = vk::StencilOpState{};
	m_depthStencilStateCreateInfo.back = vk::StencilOpState{};

	auto initStencilState = [=, this](vk::StencilOpState& vkState, const StencilFace& state)
	{
		vkState.compareOp = VulkanEnumConverter::ConvertCompareOp(state.CompareOp);
		vkState.compareMask = m_createInfo.StencilTest.ReadMask;
		vkState.writeMask = m_createInfo.StencilTest.WriteMask;
		vkState.reference = 0;
		vkState.failOp = VulkanEnumConverter::ConvertStencilOp(state.FailOp);
		vkState.passOp = VulkanEnumConverter::ConvertStencilOp(state.PassOp);
		vkState.depthFailOp = VulkanEnumConverter::ConvertStencilOp(state.DepthFailOp);
	};

	if (m_createInfo.StencilTest.Enable)
	{
		initStencilState(m_depthStencilStateCreateInfo.front, m_createInfo.StencilTest.FrontFace);
		initStencilState(m_depthStencilStateCreateInfo.back, m_createInfo.StencilTest.BackFace);
	}

	m_pipelineCreateInfo.pDepthStencilState = &m_depthStencilStateCreateInfo;
}

vk::ShaderModule VulkanPipeline::CreateShaderModule(const std::vector<uint32_t>& data) const
{
	vk::ShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.codeSize = data.size() * sizeof(uint32_t);
	shaderModuleCreateInfo.pCode = data.data();

	return m_context->LogicalDevice.createShaderModule(shaderModuleCreateInfo);
}

void VulkanPipeline::CreatePipeline()
{
	for (auto attachment : m_createInfo.Rendering.ColorAttachmentFormats)
	{
		m_colorFormats.push_back(VulkanEnumConverter::ConvertImageFormat(attachment));
	}

	m_renderingCreateInfo = vk::PipelineRenderingCreateInfo()
			.setViewMask(m_createInfo.Rendering.ViewMask)
			.setColorAttachmentFormats(m_colorFormats)
			.setDepthAttachmentFormat(VulkanEnumConverter::ConvertImageFormat(m_createInfo.Rendering.DepthAttachmentFormat))
			.setStencilAttachmentFormat(VulkanEnumConverter::ConvertImageFormat(m_createInfo.Rendering.StencilAttachmentFormat));

	m_pipelineCreateInfo.pNext = &m_renderingCreateInfo;
	Instance = m_context->LogicalDevice.createGraphicsPipeline(nullptr, m_pipelineCreateInfo).value;
}

vk::WriteDescriptorSet VulkanPipeline::GetWriteDescriptorSet(const std::string& name)
{
	if (!m_descriptorSets.contains(name))
	{
		LOG(Verbosity::Critical, "VulkanPipeline", "Invalid descriptor set, about to crash!");
	}

	return m_descriptorSets[name];
}

VulkanPipeline::~VulkanPipeline()
{
	for (const auto& module : m_shaderModules)
	{
		m_context->LogicalDevice.destroyShaderModule(module);
	}

	for (const auto& layout : m_layouts)
	{
		m_context->LogicalDevice.destroyDescriptorSetLayout(layout);
	}

	m_context->LogicalDevice.destroyPipeline(Instance);
	m_context->LogicalDevice.destroyPipelineLayout(Layout);
}
