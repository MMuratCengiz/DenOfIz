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

#include <DenOfIzGraphics/Backends/Vulkan/VulkanResourceBindGroup.h>
#include "DenOfIzGraphics/Backends/Vulkan/VulkanBufferResource.h"
#include "DenOfIzGraphics/Backends/Vulkan/VulkanTextureResource.h"

using namespace DenOfIz;

VulkanResourceBindGroup::VulkanResourceBindGroup( VulkanContext *context, ResourceBindGroupDesc desc ) : IResourceBindGroup( desc ), m_context( context )
{
    m_rootSignature = dynamic_cast<VulkanRootSignature *>( m_desc.RootSignature );
}

void VulkanResourceBindGroup::Update( const UpdateDesc desc )
{
    m_writeDescriptorSets.clear( );
    IResourceBindGroup::Update( desc );
}

void VulkanResourceBindGroup::BindTexture( const std::string &name, ITextureResource *resource )
{
    auto *vulkanResource = dynamic_cast<VulkanTextureResource *>( resource );

    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( resource->Name );
    VkDescriptorImageInfo imageInfo{ };
    imageInfo.imageLayout         = vulkanResource->Layout( );
    imageInfo.imageView           = vulkanResource->ImageView( );
    writeDescriptorSet.pImageInfo = &imageInfo;
}

void VulkanResourceBindGroup::BindBuffer( const std::string &name, IBufferResource *resource )
{
    const auto *vulkanResource = dynamic_cast<VulkanBufferResource *>( resource );

    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( resource->Name );
    writeDescriptorSet.pBufferInfo           = &vulkanResource->DescriptorInfo;
}

void VulkanResourceBindGroup::BindSampler( const std::string &name, ISampler *sampler )
{
    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( sampler->Name );
}

VkWriteDescriptorSet &VulkanResourceBindGroup::CreateWriteDescriptor( std::string &name )
{
    ResourceBindingDesc resourceBinding = m_rootSignature->GetResourceBinding( name );

    VkWriteDescriptorSet &writeDescriptorSet = m_writeDescriptorSets.emplace_back( );
    writeDescriptorSet.dstSet                = nullptr;
    writeDescriptorSet.dstBinding            = resourceBinding.Binding;
    writeDescriptorSet.descriptorType        = VulkanEnumConverter::ConvertResourceDescriptorToDescriptorType( resourceBinding.Descriptor );
    writeDescriptorSet.descriptorCount       = resourceBinding.ArraySize;
    return writeDescriptorSet;
}
