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

#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanBindGroup.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/RayTracing/VulkanTopLevelAS.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanBuffer.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanEnumConverter.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanTexture.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

VulkanBindGroup::VulkanBindGroup( VulkanContext *context, const DenOfIz_BindGroupDesc &desc ) : m_desc( desc ), m_context( context )
{
    m_bindGroupLayout = dynamic_cast<VulkanBindGroupLayout *>( DENOFIZ_FROM_HANDLE( IBindGroupLayout, desc.Layout ) );

    if ( m_bindGroupLayout->RegisterSpace( ) != DENOFIZ_ROOT_CONSTANT_REGISTER_SPACE && m_bindGroupLayout->HasBindings( ) )
    {
        VkDescriptorSetLayout layout = m_bindGroupLayout->DescriptorSetLayout( );

        VkDescriptorSetAllocateInfo setAllocateInfo{ };
        setAllocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        setAllocateInfo.descriptorSetCount = 1;
        setAllocateInfo.pSetLayouts        = &layout;

        m_context->DescriptorPoolManager->AllocateDescriptorSets( setAllocateInfo, &m_descriptorSet );
    }
}

IBindGroup *VulkanBindGroup::BeginUpdate( )
{
    m_writeDescriptorSets.clear( );
    m_storage.Clear( );
    return this;
}

IBindGroup *VulkanBindGroup::Cbv( const uint32_t binding, IBuffer *resource )
{
    BindBuffer( GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER ), resource );
    return this;
}

IBindGroup *VulkanBindGroup::Cbv( const uint32_t binding, IBuffer *resource, size_t resourceOffset )
{
    const auto *vulkanResource = dynamic_cast<VulkanBuffer *>( resource );

    const DenOfIz_ResourceBindingSlot slot               = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER );
    VkWriteDescriptorSet             &writeDescriptorSet = CreateWriteDescriptor( slot );
    auto                             &bufferInfo         = m_storage.Store<VkDescriptorBufferInfo>( );
    bufferInfo.buffer                                    = vulkanResource->Instance( );
    bufferInfo.offset                                    = vulkanResource->Offset( ) + resourceOffset;
    bufferInfo.range                                     = VK_WHOLE_SIZE;
    writeDescriptorSet.pBufferInfo                       = &bufferInfo;
    return this;
}

IBindGroup *VulkanBindGroup::Srv( const uint32_t binding, IBuffer *resource )
{
    BindBuffer( GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE ), resource );
    return this;
}

IBindGroup *VulkanBindGroup::Srv( const uint32_t binding, IBuffer *resource, size_t resourceOffset )
{
    const auto *vulkanResource = dynamic_cast<VulkanBuffer *>( resource );

    const DenOfIz_ResourceBindingSlot slot               = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE );
    VkWriteDescriptorSet             &writeDescriptorSet = CreateWriteDescriptor( slot );
    auto                             &bufferInfo         = m_storage.Store<VkDescriptorBufferInfo>( );
    bufferInfo.buffer                                    = vulkanResource->Instance( );
    bufferInfo.offset                                    = vulkanResource->Offset( ) + resourceOffset;
    bufferInfo.range                                     = VK_WHOLE_SIZE;
    writeDescriptorSet.pBufferInfo                       = &bufferInfo;
    return this;
}

IBindGroup *VulkanBindGroup::Srv( const uint32_t binding, ITexture *resource )
{
    BindTexture( GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE ), resource );
    return this;
}

IBindGroup *VulkanBindGroup::SrvArray( const uint32_t binding, const DenOfIz_TextureArray &resources )
{
    const DenOfIz_ResourceBindingSlot slot = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE );

    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( slot );
    writeDescriptorSet.descriptorCount       = resources.NumElements;
    writeDescriptorSet.dstArrayElement       = 0;

    auto *imageInfos = m_storage.StoreArray<VkDescriptorImageInfo>( resources.NumElements );
    for ( uint32_t i = 0; i < resources.NumElements; ++i )
    {
        const auto *vulkanResource  = dynamic_cast<VulkanTexture *>( DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, resources.Elements[ i ] ) );
        imageInfos[ i ].imageLayout = vulkanResource->Layout( );
        imageInfos[ i ].imageView   = vulkanResource->ImageView( );
        imageInfos[ i ].sampler     = VK_NULL_HANDLE;
    }
    writeDescriptorSet.pImageInfo = imageInfos;
    return this;
}

IBindGroup *VulkanBindGroup::SrvArrayIndex( const uint32_t binding, const uint32_t arrayIndex, ITexture *resource )
{
    const auto                       *vulkanResource = dynamic_cast<VulkanTexture *>( resource );
    const DenOfIz_ResourceBindingSlot slot           = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE );

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

IBindGroup *VulkanBindGroup::Srv( const uint32_t binding, ITopLevelAS *accelerationStructure )
{
    const auto vkAccelerationStructure = dynamic_cast<VulkanTopLevelAS *>( accelerationStructure );
    auto      &writeDescriptorSet      = BindBuffer( GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE ), vkAccelerationStructure->GetVulkanBuffer( ) );

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

IBindGroup *VulkanBindGroup::Uav( const uint32_t binding, IBuffer *resource )
{
    BindBuffer( GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS ), resource );
    return this;
}

IBindGroup *VulkanBindGroup::Uav( const uint32_t binding, IBuffer *resource, size_t resourceOffset )
{
    const auto *vulkanResource = dynamic_cast<VulkanBuffer *>( resource );

    const DenOfIz_ResourceBindingSlot slot               = GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS );
    VkWriteDescriptorSet             &writeDescriptorSet = CreateWriteDescriptor( slot );
    auto                             &bufferInfo         = m_storage.Store<VkDescriptorBufferInfo>( );
    bufferInfo.buffer                                    = vulkanResource->Instance( );
    bufferInfo.offset                                    = vulkanResource->Offset( ) + resourceOffset;
    bufferInfo.range                                     = VK_WHOLE_SIZE;
    writeDescriptorSet.pBufferInfo                       = &bufferInfo;
    return this;
}

IBindGroup *VulkanBindGroup::Uav( const uint32_t binding, ITexture *resource )
{
    BindTexture( GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS ), resource );
    return this;
}

IBindGroup *VulkanBindGroup::Sampler( const uint32_t binding, ISampler *sampler )
{
    BindSampler( GetSlot( binding, DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER ), sampler );
    return this;
}

void VulkanBindGroup::EndUpdate( )
{
    vkUpdateDescriptorSets( m_context->LogicalDevice, m_writeDescriptorSets.size( ), m_writeDescriptorSets.data( ), 0, nullptr );
}

void VulkanBindGroup::BindTexture( const DenOfIz_ResourceBindingSlot &slot, ITexture *resource )
{
    const auto *vulkanResource = dynamic_cast<VulkanTexture *>( resource );

    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( slot );
    auto                 &imageInfo          = m_storage.Store<VkDescriptorImageInfo>( );
    imageInfo.imageLayout                    = vulkanResource->Layout( );
    imageInfo.imageView                      = vulkanResource->ImageView( );
    writeDescriptorSet.pImageInfo            = &imageInfo;
}

VkWriteDescriptorSet &VulkanBindGroup::BindBuffer( const DenOfIz_ResourceBindingSlot &slot, IBuffer *resource )
{
    const auto *vulkanResource = dynamic_cast<VulkanBuffer *>( resource );

    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( slot );
    auto                 &bufferInfo         = m_storage.Store<VkDescriptorBufferInfo>( );
    bufferInfo.buffer                        = vulkanResource->Instance( );
    bufferInfo.offset                        = vulkanResource->Offset( );
    bufferInfo.range                         = VK_WHOLE_SIZE;
    writeDescriptorSet.pBufferInfo           = &bufferInfo;

    return writeDescriptorSet;
}

void VulkanBindGroup::BindSampler( const DenOfIz_ResourceBindingSlot &slot, ISampler *sampler )
{
    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( slot );
    auto                 &samplerInfo        = m_storage.Store<VkDescriptorImageInfo>( );
    samplerInfo.sampler                      = dynamic_cast<VulkanSampler *>( sampler )->Instance( );
    writeDescriptorSet.pImageInfo            = &samplerInfo;
}

VkWriteDescriptorSet &VulkanBindGroup::CreateWriteDescriptor( const DenOfIz_ResourceBindingSlot &slot )
{
    const DenOfIz_BindingDesc resourceBinding    = m_bindGroupLayout->GetVkShiftedBinding( slot );
    VkWriteDescriptorSet     &writeDescriptorSet = m_writeDescriptorSets.emplace_back( );
    writeDescriptorSet.sType                     = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet                    = m_descriptorSet;
    writeDescriptorSet.dstBinding                = resourceBinding.Binding;
    writeDescriptorSet.descriptorType            = DenOfIz_VulkanEnumConverter_ConvertResourceDescriptorToDescriptorType( resourceBinding.Descriptor );
    writeDescriptorSet.descriptorCount           = resourceBinding.ArraySize;
    return writeDescriptorSet;
}

VulkanBindGroup::~VulkanBindGroup( )
{
    m_storage.Clear( );
}

bool VulkanBindGroup::HasDescriptorSet( ) const
{
    return m_descriptorSet != nullptr;
}

const VkDescriptorSet &VulkanBindGroup::GetDescriptorSet( ) const
{
    return m_descriptorSet;
}

VulkanBindGroupLayout *VulkanBindGroup::BindGroupLayout( ) const
{
    return m_bindGroupLayout;
}

uint32_t VulkanBindGroup::RegisterSpace( ) const
{
    return m_bindGroupLayout->RegisterSpace( );
}

DenOfIz_ResourceBindingSlot VulkanBindGroup::GetSlot( uint32_t binding, const DenOfIz_ResourceBindingType &type ) const
{
    return DenOfIz_ResourceBindingSlot{ .Type = type, .Binding = binding, .RegisterSpace = m_bindGroupLayout->RegisterSpace( ) };
}
