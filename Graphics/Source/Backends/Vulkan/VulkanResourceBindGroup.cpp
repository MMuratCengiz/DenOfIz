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

#include "DenOfIzGraphicsInternal/Backends/Vulkan/RayTracing/VulkanTopLevelAS.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanBufferResource.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanResourceBindGroup.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanTextureResource.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

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

void VulkanResourceBindGroup::SetRootConstantsData( uint32_t binding, const InteropArray<Byte> &data )
{
    const VkPushConstantRange pushConstantRange = m_rootSignature->PushConstantRange( binding );
    if ( data.NumElements( ) != pushConstantRange.size )
    {
        LOG( ERROR ) << "Root constant size mismatch. Expected: " << pushConstantRange.size << ", Got: " << data.NumElements( );
        return;
    }
    SetRootConstants( binding, (void *)data.Data( ) );
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

IResourceBindGroup *VulkanResourceBindGroup::BeginUpdate( )
{
    m_writeDescriptorSets.clear( );
    m_storage.Clear( );
    return this;
}

IResourceBindGroup *VulkanResourceBindGroup::Cbv( const uint32_t binding, IBufferResource *resource )
{
    BindBuffer( GetSlot( binding, ResourceBindingType::ConstantBuffer ), resource );
    return this;
}

IResourceBindGroup *VulkanResourceBindGroup::Cbv( const BindBufferDesc &desc )
{
    const auto *vulkanResource = dynamic_cast<VulkanBufferResource *>( desc.Resource );

    const ResourceBindingSlot slot               = GetSlot( desc.Binding, ResourceBindingType::ConstantBuffer );
    VkWriteDescriptorSet     &writeDescriptorSet = CreateWriteDescriptor( slot );
    auto                     &bufferInfo         = m_storage.Store<VkDescriptorBufferInfo>( );
    bufferInfo.buffer                            = vulkanResource->Instance( );
    bufferInfo.offset                            = vulkanResource->Offset( ) + desc.ResourceOffset;
    bufferInfo.range                             = VK_WHOLE_SIZE - desc.ResourceOffset;
    writeDescriptorSet.pBufferInfo               = &bufferInfo;
    return this;
}

IResourceBindGroup *VulkanResourceBindGroup::Srv( const uint32_t binding, IBufferResource *resource )
{
    BindBuffer( GetSlot( binding, ResourceBindingType::ShaderResource ), resource );
    return this;
}

IResourceBindGroup *VulkanResourceBindGroup::Srv( const BindBufferDesc &desc )
{
    const auto *vulkanResource = dynamic_cast<VulkanBufferResource *>( desc.Resource );

    const ResourceBindingSlot slot               = GetSlot( desc.Binding, ResourceBindingType::ShaderResource );
    VkWriteDescriptorSet     &writeDescriptorSet = CreateWriteDescriptor( slot );
    auto                     &bufferInfo         = m_storage.Store<VkDescriptorBufferInfo>( );
    bufferInfo.buffer                            = vulkanResource->Instance( );
    bufferInfo.offset                            = vulkanResource->Offset( ) + desc.ResourceOffset;
    bufferInfo.range                             = VK_WHOLE_SIZE - desc.ResourceOffset;
    writeDescriptorSet.pBufferInfo               = &bufferInfo;
    return this;
}

IResourceBindGroup *VulkanResourceBindGroup::Srv( const uint32_t binding, ITextureResource *resource )
{
    BindTexture( GetSlot( binding, ResourceBindingType::ShaderResource ), resource );
    return this;
}

IResourceBindGroup *VulkanResourceBindGroup::SrvArray( const uint32_t binding, const InteropArray<ITextureResource *> &resources )
{
    const ResourceBindingSlot slot = GetSlot( binding, ResourceBindingType::ShaderResource );

    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( slot );
    writeDescriptorSet.descriptorCount       = resources.NumElements( );
    writeDescriptorSet.dstArrayElement       = 0;

    auto *imageInfos = m_storage.StoreArray<VkDescriptorImageInfo>( resources.NumElements( ) );
    for ( uint32_t i = 0; i < resources.NumElements( ); ++i )
    {
        const auto *vulkanResource  = dynamic_cast<VulkanTextureResource *>( resources.GetElement( i ) );
        imageInfos[ i ].imageLayout = vulkanResource->Layout( );
        imageInfos[ i ].imageView   = vulkanResource->ImageView( );
        imageInfos[ i ].sampler     = VK_NULL_HANDLE;
    }
    writeDescriptorSet.pImageInfo = imageInfos;
    return this;
}

IResourceBindGroup *VulkanResourceBindGroup::SrvArrayIndex( const uint32_t binding, const uint32_t arrayIndex, ITextureResource *resource )
{
    const auto               *vulkanResource = dynamic_cast<VulkanTextureResource *>( resource );
    const ResourceBindingSlot slot           = GetSlot( binding, ResourceBindingType::ShaderResource );

    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( slot );
    writeDescriptorSet.dstArrayElement       = arrayIndex;
    writeDescriptorSet.descriptorCount       = 1;

    auto &imageInfo       = m_storage.Store<VkDescriptorImageInfo>( );
    imageInfo.imageLayout = vulkanResource->Layout( );
    imageInfo.imageView   = vulkanResource->ImageView( );
    imageInfo.sampler     = VK_NULL_HANDLE;

    writeDescriptorSet.pImageInfo = &imageInfo;
    return this;
}

IResourceBindGroup *VulkanResourceBindGroup::Srv( const uint32_t binding, ITopLevelAS *accelerationStructure )
{
    const auto vkAccelerationStructure = dynamic_cast<VulkanTopLevelAS *>( accelerationStructure );
    auto      &writeDescriptorSet      = BindBuffer( GetSlot( binding, ResourceBindingType::ShaderResource ), vkAccelerationStructure->VulkanBuffer( ) );

    if ( writeDescriptorSet.descriptorType == VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR )
    {
        auto &accelerationStructureInfo                      = m_storage.Store<VkWriteDescriptorSetAccelerationStructureKHR>( );
        accelerationStructureInfo.sType                      = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
        accelerationStructureInfo.pNext                      = nullptr;
        accelerationStructureInfo.accelerationStructureCount = 1;
        accelerationStructureInfo.pAccelerationStructures    = &vkAccelerationStructure->Instance( );
        writeDescriptorSet.pNext                             = &accelerationStructureInfo;
    }

    return this;
}

IResourceBindGroup *VulkanResourceBindGroup::Uav( const uint32_t binding, IBufferResource *resource )
{
    BindBuffer( GetSlot( binding, ResourceBindingType::UnorderedAccess ), resource );
    return this;
}

IResourceBindGroup *VulkanResourceBindGroup::Uav( const BindBufferDesc &desc )
{
    const auto *vulkanResource = dynamic_cast<VulkanBufferResource *>( desc.Resource );

    const ResourceBindingSlot slot               = GetSlot( desc.Binding, ResourceBindingType::UnorderedAccess );
    VkWriteDescriptorSet     &writeDescriptorSet = CreateWriteDescriptor( slot );
    auto                     &bufferInfo         = m_storage.Store<VkDescriptorBufferInfo>( );
    bufferInfo.buffer                            = vulkanResource->Instance( );
    bufferInfo.offset                            = vulkanResource->Offset( ) + desc.ResourceOffset;
    bufferInfo.range                             = VK_WHOLE_SIZE - desc.ResourceOffset;
    writeDescriptorSet.pBufferInfo               = &bufferInfo;
    return this;
}

IResourceBindGroup *VulkanResourceBindGroup::Uav( const uint32_t binding, ITextureResource *resource )
{
    BindTexture( GetSlot( binding, ResourceBindingType::UnorderedAccess ), resource );
    return this;
}

IResourceBindGroup *VulkanResourceBindGroup::Sampler( const uint32_t binding, ISampler *sampler )
{
    BindSampler( GetSlot( binding, ResourceBindingType::Sampler ), sampler );
    return this;
}

void VulkanResourceBindGroup::EndUpdate( )
{
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

VkWriteDescriptorSet &VulkanResourceBindGroup::BindBuffer( const ResourceBindingSlot &slot, IBufferResource *resource )
{
    const auto *vulkanResource = dynamic_cast<VulkanBufferResource *>( resource );

    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( slot );
    auto                 &bufferInfo         = m_storage.Store<VkDescriptorBufferInfo>( );
    bufferInfo.buffer                        = vulkanResource->Instance( );
    bufferInfo.offset                        = vulkanResource->Offset( );
    bufferInfo.range                         = VK_WHOLE_SIZE;
    writeDescriptorSet.pBufferInfo           = &bufferInfo;

    return writeDescriptorSet;
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

ResourceBindingSlot VulkanResourceBindGroup::GetSlot( uint32_t binding, const ResourceBindingType &type ) const
{
    return ResourceBindingSlot{ .Type = type, .Binding = binding, .RegisterSpace = m_desc.RegisterSpace };
}
