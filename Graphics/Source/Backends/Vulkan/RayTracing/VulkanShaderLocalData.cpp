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

#include "DenOfIzGraphicsInternal/Backends/Vulkan/RayTracing/VulkanShaderLocalData.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/RayTracing/VulkanLocalRootSignature.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanBufferResource.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanTextureResource.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

VulkanShaderLocalData::VulkanShaderLocalData( VulkanContext *context, const ShaderLocalDataDesc &desc ) :
    m_context( context ), m_desc( desc ), m_layout( dynamic_cast<VulkanLocalRootSignature *>( desc.Layout ) )
{
    m_inlineData.resize( m_layout->InlineDataNumBytes( ) );
    // It is possible that the layout only contains push constants
    if ( m_layout->DescriptorSetLayout( ) != nullptr )
    {
        VkDescriptorSetAllocateInfo setAllocateInfo{ };
        setAllocateInfo.sType              = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        setAllocateInfo.descriptorSetCount = 1;
        setAllocateInfo.pSetLayouts        = m_layout->DescriptorSetLayout( );

        m_context->DescriptorPoolManager->AllocateDescriptorSets( setAllocateInfo, &m_descriptorSet );
    }
}

VulkanShaderLocalData::~VulkanShaderLocalData( )
{
    if ( m_descriptorSet )
    {
        m_context->DescriptorPoolManager->FreeDescriptorSets( 1, &m_descriptorSet );
    }
    m_storage.Clear( );
}

void VulkanShaderLocalData::Begin( )
{
    m_writeDescriptorSets.clear( );
}

void VulkanShaderLocalData::Cbv( const uint32_t binding, IBufferResource *bufferResource )
{
    auto          *vulkanBuffer = dynamic_cast<VulkanBufferResource *>( bufferResource );
    const uint32_t offset       = m_layout->CbvOffset( binding );
    const uint32_t size         = m_layout->CbvNumBytes( binding );

    const void *srcData = vulkanBuffer->MapMemory( );
    memcpy( m_inlineData.data( ) + offset, srcData, size );
    vulkanBuffer->UnmapMemory( );
}

void VulkanShaderLocalData::Cbv( const uint32_t binding, const InteropArray<Byte> &data )
{
    const uint32_t offset   = m_layout->CbvOffset( binding );
    auto           numBytes = m_layout->CbvNumBytes( binding );
    if ( data.NumElements( ) > numBytes )
    {
        spdlog::error( "Data larger than expected: [ {} vs {} ] for binding: {} This could lead to data corruption. Binding skipped.", data.NumElements( ), numBytes, binding );
        return;
    }
    memcpy( m_inlineData.data( ) + offset, data.Data( ), data.NumElements( ) );
}

void VulkanShaderLocalData::Srv( const uint32_t binding, const IBufferResource *bufferResource )
{
    const auto *vulkanBuffer = dynamic_cast<const VulkanBufferResource *>( bufferResource );

    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( binding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER );

    auto &bufferInfo               = m_storage.Store<VkDescriptorBufferInfo>( );
    bufferInfo.buffer              = vulkanBuffer->Instance( );
    bufferInfo.offset              = vulkanBuffer->Offset( );
    bufferInfo.range               = vulkanBuffer->NumBytes( );
    writeDescriptorSet.pBufferInfo = &bufferInfo;
}

void VulkanShaderLocalData::Srv( const uint32_t binding, const ITextureResource *textureResource )
{
    const auto *vulkanTexture = dynamic_cast<const VulkanTextureResource *>( textureResource );

    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( binding, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE );

    auto &imageInfo               = m_storage.Store<VkDescriptorImageInfo>( );
    imageInfo.imageLayout         = vulkanTexture->Layout( );
    imageInfo.imageView           = vulkanTexture->ImageView( );
    writeDescriptorSet.pImageInfo = &imageInfo;
}

void VulkanShaderLocalData::Uav( const uint32_t binding, const IBufferResource *bufferResource )
{
    const auto *vulkanBuffer = dynamic_cast<const VulkanBufferResource *>( bufferResource );

    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( binding, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER );

    auto &bufferInfo               = m_storage.Store<VkDescriptorBufferInfo>( );
    bufferInfo.buffer              = vulkanBuffer->Instance( );
    bufferInfo.offset              = vulkanBuffer->Offset( );
    bufferInfo.range               = vulkanBuffer->NumBytes( );
    writeDescriptorSet.pBufferInfo = &bufferInfo;
}

void VulkanShaderLocalData::Uav( const uint32_t binding, const ITextureResource *textureResource )
{
    const auto *vulkanTexture = dynamic_cast<const VulkanTextureResource *>( textureResource );

    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( binding, VK_DESCRIPTOR_TYPE_STORAGE_IMAGE );

    auto &imageInfo               = m_storage.Store<VkDescriptorImageInfo>( );
    imageInfo.imageLayout         = vulkanTexture->Layout( );
    imageInfo.imageView           = vulkanTexture->ImageView( );
    writeDescriptorSet.pImageInfo = &imageInfo;
}

void VulkanShaderLocalData::Sampler( uint32_t binding, const ISampler *sampler )
{
    const auto *vulkanSampler = dynamic_cast<const VulkanSampler *>( sampler );

    VkWriteDescriptorSet &writeDescriptorSet = CreateWriteDescriptor( binding, VK_DESCRIPTOR_TYPE_SAMPLER );

    auto &samplerInfo             = m_storage.Store<VkDescriptorImageInfo>( );
    samplerInfo.sampler           = vulkanSampler->Instance( );
    writeDescriptorSet.pImageInfo = &samplerInfo;
}

void VulkanShaderLocalData::End( )
{
    if ( !m_writeDescriptorSets.empty( ) )
    {
        vkUpdateDescriptorSets( m_context->LogicalDevice, m_writeDescriptorSets.size( ), m_writeDescriptorSets.data( ), 0, nullptr );
    }
}

const VkDescriptorSet *VulkanShaderLocalData::DescriptorSet( ) const
{
    return &m_descriptorSet;
}

uint32_t VulkanShaderLocalData::DataNumBytes( ) const
{
    return m_inlineData.size( );
}

const Byte *VulkanShaderLocalData::Data( ) const
{
    return m_inlineData.data( );
}

VkWriteDescriptorSet &VulkanShaderLocalData::CreateWriteDescriptor( uint32_t binding, VkDescriptorType type )
{
    VkWriteDescriptorSet &writeDescriptorSet = m_writeDescriptorSets.emplace_back( );
    writeDescriptorSet.sType                 = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    writeDescriptorSet.dstSet                = m_descriptorSet;
    writeDescriptorSet.dstBinding            = binding;
    writeDescriptorSet.descriptorType        = type;
    writeDescriptorSet.descriptorCount       = 1;
    return writeDescriptorSet;
}
