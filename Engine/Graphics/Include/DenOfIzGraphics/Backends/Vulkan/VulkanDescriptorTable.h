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

#include "VulkanContext.h"
#include "VulkanRootSignature.h"
#include <DenOfIzGraphics/Backends/Interface/IDescriptorTable.h>

namespace DenOfIz
{

class VulkanDescriptorTable : public IDescriptorTable
{
private:
	VulkanContext* m_context;
	VulkanRootSignature* m_rootSignature;
	DescriptorTableCreateInfo m_createInfo;

	std::vector<vk::DescriptorSet> m_descriptorSets;
	std::vector<vk::WriteDescriptorSet> m_writeDescriptorSets;
public:
	VulkanDescriptorTable(VulkanContext* context, DescriptorTableCreateInfo createInfo);

	void BindImage(IImageResource* resource) override;
	void BindBuffer(IBufferResource* resource) override;

	const std::vector<vk::WriteDescriptorSet>& GetWriteDescriptorSets() const { return m_writeDescriptorSets; }

	vk::WriteDescriptorSet& CreateWriteDescriptor(std::string& name);
};

}