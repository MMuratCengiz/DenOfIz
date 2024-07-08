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

#include <DenOfIzGraphics/Backends/Vulkan/VulkanDescriptorTable.h>
#include "DenOfIzGraphics/Backends/Vulkan/VulkanBufferResource.h"
#include "DenOfIzGraphics/Backends/Vulkan/VulkanTextureResource.h"

using namespace DenOfIz;

VulkanDescriptorTable::VulkanDescriptorTable(VulkanContext *context, DescriptorTableDesc desc) : m_context(context), m_desc(std::move(desc))
{
    m_rootSignature = static_cast<VulkanRootSignature *>(m_desc.RootSignature);
}

void VulkanDescriptorTable::BindImage(ITextureResource *resource)
{
    VulkanTextureResource *vulkanResource = static_cast<VulkanTextureResource *>(resource);

    vk::WriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor(resource->Name);
    writeDescriptorSet.pImageInfo              = &vulkanResource->DescriptorInfo;
}

void VulkanDescriptorTable::BindBuffer(IBufferResource *resource)
{
    VulkanBufferResource *vulkanResource = static_cast<VulkanBufferResource *>(resource);

    vk::WriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor(resource->Name);
    writeDescriptorSet.pBufferInfo             = &vulkanResource->DescriptorInfo;
}

vk::WriteDescriptorSet &VulkanDescriptorTable::CreateWriteDescriptor(std::string &name)
{
    ResourceBinding resourceBinding = m_rootSignature->GetResourceBinding(name);

    vk::WriteDescriptorSet &writeDescriptorSet = m_writeDescriptorSets.emplace_back();
    writeDescriptorSet.dstSet                  = nullptr;
    writeDescriptorSet.dstBinding              = resourceBinding.Binding;
    writeDescriptorSet.descriptorType          = VulkanEnumConverter::ConvertResourceDescriptorToDescriptorType(resourceBinding.Descriptor);
    writeDescriptorSet.descriptorCount         = resourceBinding.ArraySize;
    return writeDescriptorSet;
}
