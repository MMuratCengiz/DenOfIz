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

#include <DenOfIzGraphics/Backends/Vulkan/VulkanInputLayout.h>

using namespace DenOfIz;

VulkanInputLayout::VulkanInputLayout(const InputLayoutCreateInfo& createInfo)
{
	int bindingIndex = 0;
	for (const InputGroup& inputGroup : createInfo.InputGroups)
	{
		VkVertexInputBindingDescription& bindingDescription = m_bindingDescriptions.emplace_back(VkVertexInputBindingDescription{});
		bindingDescription.binding = bindingIndex++;
		bindingDescription.inputRate = inputGroup.StepRate == StepRate::PerInstance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX;

		for (const InputLayoutElement& inputElement : inputGroup.Elements)
		{
			VkVertexInputAttributeDescription& attributeDescription = m_attributeDescriptions.emplace_back(VkVertexInputAttributeDescription{});
			attributeDescription.binding = inputElement.Binding;
			attributeDescription.location = inputElement.SemanticIndex;
			attributeDescription.format = (VkFormat) VulkanEnumConverter::ConvertImageFormat(inputElement.Format);
			attributeDescription.offset = inputElement.Offset;
		}
		bindingDescription.stride = inputGroup.Elements.size() * sizeof(float);
	}

	m_vertexInputState.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	m_vertexInputState.vertexBindingDescriptionCount = static_cast<uint32_t>(m_bindingDescriptions.size());
	m_vertexInputState.pVertexBindingDescriptions = m_bindingDescriptions.data();
	m_vertexInputState.vertexAttributeDescriptionCount = static_cast<uint32_t>(m_attributeDescriptions.size());
	m_vertexInputState.pVertexAttributeDescriptions = m_attributeDescriptions.data();
}
