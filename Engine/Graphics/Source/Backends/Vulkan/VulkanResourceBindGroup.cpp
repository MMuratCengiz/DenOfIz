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

VulkanResourceBindGroup::VulkanResourceBindGroup( VulkanContext *context, const ResourceBindGroupDesc &desc ) : IResourceBindGroup( desc ), m_context( context )
{
    m_rootSignature = dynamic_cast<VulkanRootSignature *>( m_desc.RootSignature );

    const auto &layout = m_rootSignature->GetDescriptorSetLayout( m_desc.RegisterSpace );

    if ( layout == VK_NULL_HANDLE )
    {
        LOG( ERROR ) << "Descriptor set layout not found for register space: " << m_desc.RegisterSpace;
    }

    VkDescriptorSetAllocateInfo setAllocateInfo{ };
    setAllocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    setAllocateInfo.descriptorSetCount = 1;
    setAllocateInfo.pSetLayouts        = &layout;

    m_context->DescriptorPoolManager->AllocateDescriptorSets( setAllocateInfo, &m_descriptorSet );
}

void VulkanResourceBindGroup::Update( const UpdateDesc &desc )
{
    m_writeDescriptorSets.clear( );
    IResourceBindGroup::Update( desc );

    vkUpdateDescriptorSets( m_context->LogicalDevice, m_writeDescriptorSets.size( ), m_writeDescriptorSets.data( ), 0, NULL );
}

void VulkanResourceBindGroup::BindTexture( const std::string &name, ITextureResource *resource )
{
    const auto *vulkanResource = dynamic_cast<VulkanTextureResource *>( resource );

    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( resource->Name );
    auto                 &imageInfo          = m_storage.Store<VkDescriptorImageInfo>( );
    imageInfo.imageLayout                    = vulkanResource->Layout( );
    imageInfo.imageView                      = vulkanResource->ImageView( );
    writeDescriptorSet.pImageInfo            = &imageInfo;
}

void VulkanResourceBindGroup::BindBuffer( const std::string &name, IBufferResource *resource )
{
    const auto *vulkanResource = dynamic_cast<VulkanBufferResource *>( resource );

    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( resource->Name );
    auto                 &bufferInfo         = m_storage.Store<VkDescriptorBufferInfo>( );
    bufferInfo.buffer                        = vulkanResource->Instance( );
    bufferInfo.offset                        = vulkanResource->Offset( );
    bufferInfo.range                         = vulkanResource->NumBytes( );
    writeDescriptorSet.pBufferInfo           = &bufferInfo;
}

void VulkanResourceBindGroup::BindSampler( const std::string &name, ISampler *sampler )
{
    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( sampler->Name );
    auto                 &samplerInfo        = m_storage.Store<VkDescriptorImageInfo>( );
    samplerInfo.sampler                      = dynamic_cast<VulkanSampler *>( sampler )->Instance( );
    writeDescriptorSet.pImageInfo            = &samplerInfo;
}

VkWriteDescriptorSet &VulkanResourceBindGroup::CreateWriteDescriptor( const std::string &name )
{
    const ResourceBindingDesc resourceBinding    = m_rootSignature->GetResourceBinding( name );
    VkWriteDescriptorSet     &writeDescriptorSet = m_writeDescriptorSets.emplace_back( );
    writeDescriptorSet.sType                     = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet                    = m_descriptorSet;
    writeDescriptorSet.dstBinding                = resourceBinding.Binding;
    writeDescriptorSet.descriptorType            = VulkanEnumConverter::ConvertResourceDescriptorToDescriptorType( resourceBinding.Descriptor );
    writeDescriptorSet.descriptorCount           = resourceBinding.ArraySize;
    return writeDescriptorSet;
}

VulkanResourceBindGroup::~VulkanResourceBindGroup( )
{
    m_storage.Clear( );
    m_context->DescriptorPoolManager->FreeDescriptorSets( 1, &m_descriptorSet );
}
