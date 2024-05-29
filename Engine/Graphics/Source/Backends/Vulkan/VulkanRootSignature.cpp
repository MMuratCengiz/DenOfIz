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

#include <DenOfIzGraphics/Backends/Vulkan/VulkanRootSignature.h>

using namespace DenOfIz;

VulkanRootSignature::VulkanRootSignature(VulkanContext* context, RootSignatureCreateInfo createInfo) : m_context(context), m_createInfo(std::move(createInfo))
{
	m_layouts.resize(1);
}

void VulkanRootSignature::AddResourceBindingInternal(const ResourceBinding& binding)
{
	assertm(!m_created, "Root signature is already created. Changing the root signature after creation could cause undefined behavior.");
	m_resourceBindingMap[binding.Name] = binding;

	vk::DescriptorSetLayoutBinding layoutBinding{};

	layoutBinding.binding = binding.Binding;
	layoutBinding.descriptorType = VulkanEnumConverter::ConvertBindingTypeToDescriptorType(binding.Type);
	layoutBinding.descriptorCount = binding.ArraySize;

	for (auto stage : binding.Stages)
	{
		layoutBinding.stageFlags |= VulkanEnumConverter::ConvertShaderStage(stage);
	}

	m_bindings.push_back(std::move(layoutBinding));
}

void VulkanRootSignature::AddRootConstantInternal(const RootConstantBinding& rootConstantBinding)
{
	m_rootConstantMap[rootConstantBinding.Name] = rootConstantBinding;

	vk::PushConstantRange pushConstantRange{};

	pushConstantRange.offset = rootConstantBinding.Order;
	pushConstantRange.size = rootConstantBinding.Size;

	for (auto stage : rootConstantBinding.Stages)
	{
		pushConstantRange.stageFlags |= VulkanEnumConverter::ConvertShaderStage(stage);
	}

	m_pushConstants.push_back(std::move(pushConstantRange));
}

void VulkanRootSignature::CreateInternal()
{
	vk::DescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.setBindings(m_bindings);
	layoutInfo.flags = vk::DescriptorSetLayoutCreateFlagBits::ePushDescriptorKHR;
	m_layouts[0] = m_context->LogicalDevice.createDescriptorSetLayout(layoutInfo);
}

VulkanRootSignature::~VulkanRootSignature()
{
	for (auto layout : m_layouts)
	{
		m_context->LogicalDevice.destroyDescriptorSetLayout(layout);
	}
}
