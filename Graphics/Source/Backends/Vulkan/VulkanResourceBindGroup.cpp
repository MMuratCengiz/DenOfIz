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

VulkanResourceBindGroup::VulkanResourceBindGroup( VulkanContext *context, const ResourceBindGroupDesc &desc ) : m_desc( desc ), m_context( context )
{
    m_rootSignature = dynamic_cast<VulkanRootSignature *>( m_desc.RootSignature );

    if ( m_desc.RegisterSpace != DZConfiguration::Instance( ).RootConstantRegisterSpace )
    {
        const auto &layout = m_rootSignature->DescriptorSetLayout( m_desc.RegisterSpace );

        VkDescriptorSetAllocateInfo setAllocateInfo{ };
        setAllocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        setAllocateInfo.descriptorSetCount = 1;
        setAllocateInfo.pSetLayouts        = &layout;

        m_context->DescriptorPoolManager->AllocateDescriptorSets( setAllocateInfo, &m_descriptorSet );
    }

    m_rootConstants.resize( m_rootSignature->NumRootConstants( ) );
}

void VulkanResourceBindGroup::SetRootConstants( const uint32_t binding, void *data )
{
    const VkPushConstantRange  pushConstantRange   = m_rootSignature->PushConstantRange( binding );
    VulkanRootConstantBinding &rootConstantBinding = m_rootConstants[ binding ];
    rootConstantBinding.PipelineLayout             = m_rootSignature->PipelineLayout( );
    rootConstantBinding.ShaderStage                = pushConstantRange.stageFlags;
    rootConstantBinding.Binding                    = binding;
    rootConstantBinding.Offset                     = pushConstantRange.offset;
    rootConstantBinding.Size                       = pushConstantRange.size;
    rootConstantBinding.Data                       = data;
}

void VulkanResourceBindGroup::Update( const UpdateDesc &desc )
{
    m_writeDescriptorSets.clear( );
    for ( auto item : desc.Buffers )
    {
        BindBuffer( item.Slot, item.Resource );
    }
    for ( auto item : desc.Textures )
    {
        BindTexture( item.Slot, item.Resource );
    }
    for ( auto item : desc.Samplers )
    {
        BindSampler( item.Slot, item.Resource );
    }

    vkUpdateDescriptorSets( m_context->LogicalDevice, m_writeDescriptorSets.size( ), m_writeDescriptorSets.data( ), 0, nullptr );
}

void VulkanResourceBindGroup::BindTexture( const ResourceBindingSlot &slot, ITextureResource *resource )
{
    const auto *vulkanResource = dynamic_cast<VulkanTextureResource *>( resource );

    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( slot );
    auto                 &imageInfo          = m_storage.Store<VkDescriptorImageInfo>( );
    imageInfo.imageLayout                    = vulkanResource->Layout( );
    imageInfo.imageView                      = vulkanResource->ImageView( );
    writeDescriptorSet.pImageInfo            = &imageInfo;
}

void VulkanResourceBindGroup::BindBuffer( const ResourceBindingSlot &slot, IBufferResource *resource )
{
    const auto *vulkanResource = dynamic_cast<VulkanBufferResource *>( resource );

    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( slot );
    auto                 &bufferInfo         = m_storage.Store<VkDescriptorBufferInfo>( );
    bufferInfo.buffer                        = vulkanResource->Instance( );
    bufferInfo.offset                        = vulkanResource->Offset( );
    bufferInfo.range                         = vulkanResource->NumBytes( );
    writeDescriptorSet.pBufferInfo           = &bufferInfo;
}

void VulkanResourceBindGroup::BindSampler( const ResourceBindingSlot &slot, ISampler *sampler )
{
    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( slot );
    auto                 &samplerInfo        = m_storage.Store<VkDescriptorImageInfo>( );
    samplerInfo.sampler                      = dynamic_cast<VulkanSampler *>( sampler )->Instance( );
    writeDescriptorSet.pImageInfo            = &samplerInfo;
}

VkWriteDescriptorSet &VulkanResourceBindGroup::CreateWriteDescriptor( const ResourceBindingSlot &slot )
{
    const ResourceBindingDesc resourceBinding    = m_rootSignature->GetVkShiftedBinding( slot );
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

const std::vector<VulkanRootConstantBinding> &VulkanResourceBindGroup::RootConstants( ) const
{
    return m_rootConstants;
}

bool VulkanResourceBindGroup::HasDescriptorSet( ) const
{
    return m_descriptorSet != nullptr;
}

const VkDescriptorSet &VulkanResourceBindGroup::GetDescriptorSet( ) const
{
    return m_descriptorSet;
}

VulkanRootSignature *VulkanResourceBindGroup::RootSignature( ) const
{
    return m_rootSignature;
}

uint32_t VulkanResourceBindGroup::RegisterSpace( ) const
{
    return m_desc.RegisterSpace;
}
