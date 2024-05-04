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

using namespace DenOfIz;

VulkanPipeline::VulkanPipeline(VulkanContext* context, const PipelineCreateInfo& createInfo)
		:context(context), createInfo(createInfo), BindPoint(VulkanEnumConverter::ConvertPipelineBindPoint(createInfo.BindPoint))
{
	pipelineCreateInfo.pDepthStencilState = nullptr;

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

	for (const CompiledShader& shader : createInfo.SpvProgram.Shaders)
	{
		vk::PipelineShaderStageCreateInfo& shaderStageCreateInfo = pipelineStageCreateInfos.emplace_back(vk::PipelineShaderStageCreateInfo{});

		vk::ShaderStageFlagBits stage = VulkanEnumConverter::ConvertShaderStage(shader.Stage);
		vk::ShaderModule& shaderModule = shaderModules.emplace_back(this->CreateShaderModule(shader.Data));

		shaderStageCreateInfo.stage = stage;
		shaderStageCreateInfo.module = shaderModule;
		shaderStageCreateInfo.pName = "main";
		shaderStageCreateInfo.pNext = nullptr;

		hasTessellationShaders = hasTessellationShaders || stage == vk::ShaderStageFlagBits::eTessellationEvaluation;
		hasTessellationShaders = hasTessellationShaders || stage == vk::ShaderStageFlagBits::eTessellationControl;
	}

	auto& program = createInfo.SpvProgram;
	auto& vertexInputs = program.VertexInputs();

	uint32_t offsetIter = 0;
	for (const VertexInput& vertexInput : vertexInputs)
	{
		vk::VertexInputAttributeDescription& desc = vertexAttributeDescriptions.emplace_back(vk::VertexInputAttributeDescription{});

		if (createInfo.InterleavedMode)
		{
			desc.binding = 0;
		}
		else
		{
			vk::VertexInputBindingDescription& bindingDesc = inputBindingDescriptions.emplace_back(vk::VertexInputBindingDescription{});
			bindingDesc.binding = inputBindingDescriptions.size() - 1;
			bindingDesc.inputRate = vk::VertexInputRate::eVertex; // TODO investigate later for instanced rendering
			bindingDesc.stride = 0;

			desc.binding = bindingDesc.binding;
		}

		desc.location = vertexInput.Location;
		desc.format = VulkanEnumConverter::ConvertImageFormat(vertexInput.Format);
		desc.offset = vertexInput.Offset;
		offsetIter += vertexInput.Size;
	}

	if (createInfo.InterleavedMode)
	{
		vk::VertexInputBindingDescription& bindingDesc = inputBindingDescriptions.emplace_back(vk::VertexInputBindingDescription{});
		bindingDesc.binding = 0;
		bindingDesc.inputRate = vk::VertexInputRate::eVertex; // TODO investigate later for instanced rendering
		bindingDesc.stride = offsetIter;
	}

	inputStateCreateInfo.vertexBindingDescriptionCount = inputBindingDescriptions.size();
	inputStateCreateInfo.pVertexBindingDescriptions = inputBindingDescriptions.data();
	inputStateCreateInfo.vertexAttributeDescriptionCount = vertexAttributeDescriptions.size();
	inputStateCreateInfo.pVertexAttributeDescriptions = vertexAttributeDescriptions.data();

	inputAssemblyCreateInfo.topology = vk::PrimitiveTopology::eTriangleList;
	inputAssemblyCreateInfo.primitiveRestartEnable = VK_FALSE;

	pipelineCreateInfo.stageCount = static_cast<uint32_t>(pipelineStageCreateInfos.size());
	pipelineCreateInfo.pStages = pipelineStageCreateInfos.data();

	// Todo read patch control points from either pipelineRequest or from GLSLShaderSet
	tessellationStateCreateInfo.patchControlPoints = 3;

	if (hasTessellationShaders)
	{
		inputAssemblyCreateInfo.topology = vk::PrimitiveTopology::ePatchList;
		pipelineCreateInfo.pTessellationState = &tessellationStateCreateInfo;
	}

	pipelineCreateInfo.pVertexInputState = &inputStateCreateInfo;
	pipelineCreateInfo.pInputAssemblyState = &inputAssemblyCreateInfo;
}

void VulkanPipeline::ConfigureMultisampling()
{
	multisampleStateCreateInfo.sampleShadingEnable = VK_TRUE;

	switch (createInfo.MSAASampleCount)
	{
	case MSAASampleCount::_0:
		multisampleStateCreateInfo.sampleShadingEnable = VK_FALSE;
	case MSAASampleCount::_1:
		multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e1;
		break;
	case MSAASampleCount::_2:
		multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e2;
		break;
	case MSAASampleCount::_4:
		multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e4;
		break;
	case MSAASampleCount::_8:
		multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e8;
		break;
	case MSAASampleCount::_16:
		multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e16;
		break;
	case MSAASampleCount::_32:
		multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e32;
		break;
	case MSAASampleCount::_64:
		multisampleStateCreateInfo.rasterizationSamples = vk::SampleCountFlagBits::e64;
		break;
	}

	multisampleStateCreateInfo.minSampleShading = 1.0f;
	multisampleStateCreateInfo.pSampleMask = nullptr;
	multisampleStateCreateInfo.alphaToCoverageEnable = VK_FALSE;
	multisampleStateCreateInfo.alphaToOneEnable = VK_FALSE;
	multisampleStateCreateInfo.sampleShadingEnable = VK_TRUE;
	multisampleStateCreateInfo.minSampleShading = .2f;

	pipelineCreateInfo.pMultisampleState = &multisampleStateCreateInfo;
}

void VulkanPipeline::ConfigureViewport()
{
	// Todo test, these are dynamic states
	viewportStateCreateInfo.viewportCount = 0;
	viewportStateCreateInfo.pViewports = nullptr;
	viewportStateCreateInfo.scissorCount = 0;
	viewportStateCreateInfo.pScissors = nullptr;
	pipelineCreateInfo.pViewportState = &viewportStateCreateInfo;
}

void VulkanPipeline::ConfigureRasterization()
{
	rasterizationStateCreateInfo.depthClampEnable = VK_FALSE;
	rasterizationStateCreateInfo.rasterizerDiscardEnable = VK_FALSE;
	rasterizationStateCreateInfo.polygonMode = vk::PolygonMode::eFill;
	rasterizationStateCreateInfo.lineWidth = 1.0f;

	switch (createInfo.CullMode)
	{
	case CullMode::FrontAndBackFace:
		rasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eFrontAndBack;
		break;
	case CullMode::BackFace:
		rasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eBack;
		break;
	case CullMode::FrontFace:
		rasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eFront;
		break;
	case CullMode::None:
		rasterizationStateCreateInfo.cullMode = vk::CullModeFlagBits::eNone;
		break;
	}

	rasterizationStateCreateInfo.frontFace = vk::FrontFace::eCounterClockwise;
	rasterizationStateCreateInfo.depthBiasEnable = false; // Todo, test if works with dynamic state
	rasterizationStateCreateInfo.depthBiasConstantFactor = 0.0f;
	rasterizationStateCreateInfo.depthBiasClamp = 0.0f;
	rasterizationStateCreateInfo.depthBiasSlopeFactor = 0.0f;

	pipelineCreateInfo.pRasterizationState = &rasterizationStateCreateInfo;
}

void VulkanPipeline::ConfigureColorBlend()
{
	uint32_t attachmentCount = createInfo.BlendModes.size();
	colorBlendAttachments.resize(attachmentCount);

	for (int i = 0; i < attachmentCount; ++i)
	{
		colorBlendAttachments[i].colorWriteMask =
				vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA;

		colorBlendAttachments[i].blendEnable = false;

		if (createInfo.BlendModes[i] == BlendMode::AlphaBlend)
		{
			colorBlendAttachments[i].blendEnable = true;
			colorBlendAttachments[i].srcColorBlendFactor = vk::BlendFactor::eOne;
			colorBlendAttachments[i].dstColorBlendFactor = vk::BlendFactor::eOne;

			colorBlendAttachments[i].srcAlphaBlendFactor = vk::BlendFactor::eSrcAlpha;
			colorBlendAttachments[i].dstAlphaBlendFactor = vk::BlendFactor::eDstAlpha;

			colorBlendAttachments[i].colorBlendOp = vk::BlendOp::eAdd;
			colorBlendAttachments[i].alphaBlendOp = vk::BlendOp::eAdd;
		}
	}

	// This overwrites the above
	colorBlending.logicOpEnable = false;
	colorBlending.logicOp = vk::LogicOp::eCopy;
	colorBlending.attachmentCount = attachmentCount;
	colorBlending.pAttachments = colorBlendAttachments.data();
	colorBlending.blendConstants[0] = 0.0f;
	colorBlending.blendConstants[1] = 0.0f;
	colorBlending.blendConstants[2] = 0.0f;
	colorBlending.blendConstants[3] = 0.0f;
	pipelineCreateInfo.pColorBlendState = &colorBlending;
}

void VulkanPipeline::ConfigureDynamicState()
{
	dynamicStateCreateInfo.dynamicStateCount = dynamicStates.size();
	dynamicStateCreateInfo.pDynamicStates = dynamicStates.data();
	pipelineCreateInfo.pDynamicState = &dynamicStateCreateInfo;
}

void VulkanPipeline::CreatePipelineLayout()
{
	// Layout binding per set!
	std::unordered_map<uint32_t, std::vector<vk::DescriptorSetLayoutBinding>> bindings;

	for (const ShaderUniformInput& input : createInfo.SpvProgram.UniformInputs())
	{
		vk::DescriptorSetLayoutBinding& binding = bindings[input.BoundDescriptorSet].emplace_back(vk::DescriptorSetLayoutBinding{});

		binding.binding = input.Binding;

		switch (input.Type)
		{
		case UniformType::Struct:
			binding.descriptorType = vk::DescriptorType::eUniformBuffer;
			break;
		case UniformType::Sampler:
			binding.descriptorType = vk::DescriptorType::eCombinedImageSampler;
			break;
		}
		binding.descriptorCount = input.ArraySize;
		binding.stageFlags = VulkanEnumConverter::ConvertShaderStage(input.Stage);

		vk::WriteDescriptorSet& writeDescriptorSet = descriptorSets[input.Name];
		writeDescriptorSet.descriptorType = binding.descriptorType;
		writeDescriptorSet.dstBinding = input.Binding;
	}

	for (auto setBindings: bindings)
	{
		vk::DescriptorSetLayoutCreateInfo layoutCreateInfo{};
		layoutCreateInfo.bindingCount = setBindings.second.size();
		layoutCreateInfo.pBindings = setBindings.second.data();
		layoutCreateInfo.flags = vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR;

		layouts.emplace_back(context->LogicalDevice.createDescriptorSetLayout(layoutCreateInfo));
	}


	pipelineLayoutCreateInfo.setLayoutCount = layouts.size();
	pipelineLayoutCreateInfo.pSetLayouts = layouts.data();

	for (const PushConstant& pushConstant : createInfo.SpvProgram.PushConstants())
	{
		vk::PushConstantRange& pushConstantRange = pushConstants.emplace_back(vk::PushConstantRange{});
		pushConstantRange.stageFlags = VulkanEnumConverter::ConvertShaderStage(pushConstant.Stage);
		pushConstantRange.offset = pushConstant.Offset;
		pushConstantRange.size = pushConstant.Size;
	}

	pipelineLayoutCreateInfo.pushConstantRangeCount = pushConstants.size();
	pipelineLayoutCreateInfo.pPushConstantRanges = pushConstants.data();

	Layout = context->LogicalDevice.createPipelineLayout(pipelineLayoutCreateInfo);
	pipelineCreateInfo.layout = Layout;
}

void VulkanPipeline::CreateRenderPass()
{
	pipelineCreateInfo.renderPass = nullptr;
	pipelineCreateInfo.subpass = 0;
	pipelineCreateInfo.basePipelineHandle = nullptr;
	pipelineCreateInfo.basePipelineIndex = -1;
}

void VulkanPipeline::CreateDepthAttachmentImages()
{
	depthStencilStateCreateInfo.depthTestEnable = createInfo.EnableDepthTest;
	depthStencilStateCreateInfo.depthWriteEnable = createInfo.EnableDepthTest;
	depthStencilStateCreateInfo.depthCompareOp = VulkanEnumConverter::ConvertCompareOp(createInfo.DepthCompareOp);
	depthStencilStateCreateInfo.depthBoundsTestEnable = VK_FALSE;
	depthStencilStateCreateInfo.minDepthBounds = 0.0f;
	depthStencilStateCreateInfo.maxDepthBounds = 1.0f;

	bool enableStencilTest = createInfo.StencilTestStateFront.enabled || createInfo.StencilTestStateBack.enabled;

	depthStencilStateCreateInfo.stencilTestEnable = enableStencilTest;

	depthStencilStateCreateInfo.front = vk::StencilOpState{};
	depthStencilStateCreateInfo.back = vk::StencilOpState{};

	auto initStencilState = [=](vk::StencilOpState& vkState, const StencilTestState& state)
	{
		FUNCTION_BREAK(!state.enabled);
		vkState.compareOp = VulkanEnumConverter::ConvertCompareOp(state.compareOp);
		vkState.compareMask = state.compareMask;
		vkState.writeMask = state.writeMask;
		vkState.reference = state.ref;
		vkState.failOp = VulkanEnumConverter::ConvertStencilOp(state.failOp);
		vkState.passOp = VulkanEnumConverter::ConvertStencilOp(state.passOp);
		vkState.depthFailOp = VulkanEnumConverter::ConvertStencilOp(state.depthFailOp);
	};

	initStencilState(depthStencilStateCreateInfo.front, createInfo.StencilTestStateFront);
	initStencilState(depthStencilStateCreateInfo.back, createInfo.StencilTestStateFront);

	pipelineCreateInfo.pDepthStencilState = &depthStencilStateCreateInfo;
}

vk::ShaderModule VulkanPipeline::CreateShaderModule(std::vector<uint32_t> data)
{
	vk::ShaderModuleCreateInfo shaderModuleCreateInfo{};
	shaderModuleCreateInfo.codeSize = data.size() * sizeof(uint32_t);
	shaderModuleCreateInfo.pCode = data.data();

	return context->LogicalDevice.createShaderModule(shaderModuleCreateInfo);
}

void VulkanPipeline::CreatePipeline()
{
	colorFormats.resize(createInfo.Rendering.ColorAttachmentFormats.size());
	for (auto attachment : createInfo.Rendering.ColorAttachmentFormats)
	{
		colorFormats.push_back(VulkanEnumConverter::ConvertImageFormat(attachment));
	}

	renderingCreateInfo = vk::PipelineRenderingCreateInfo(
			createInfo.Rendering.ViewMask,
			colorFormats,
			VulkanEnumConverter::ConvertImageFormat(createInfo.Rendering.DepthAttachmentFormat),
			VulkanEnumConverter::ConvertImageFormat(createInfo.Rendering.StencilAttachmentFormat)
	);

	pipelineCreateInfo.pNext = &renderingCreateInfo;
	Instance = context->LogicalDevice.createGraphicsPipeline(nullptr, pipelineCreateInfo).value;
}

vk::WriteDescriptorSet VulkanPipeline::GetWriteDescriptorSet(const std::string& name)
{
	if (! descriptorSets.contains(name))
	{
		LOG(Verbosity::Critical, "VulkanPipeline", "Invalid descriptor set, about to crash!");
	}

	return descriptorSets[name];
}

VulkanPipeline::~VulkanPipeline()
{
	for (auto& module : shaderModules)
	{
		context->LogicalDevice.destroyShaderModule(module);
	}

	for (auto& layout : layouts)
	{
		context->LogicalDevice.destroyDescriptorSetLayout(layout);
	}

	context->LogicalDevice.destroyPipeline(Instance);
	context->LogicalDevice.destroyPipelineLayout(Layout);
}