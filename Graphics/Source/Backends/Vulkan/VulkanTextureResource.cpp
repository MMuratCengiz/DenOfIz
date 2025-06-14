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

#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanTextureResource.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

VulkanTextureResource::VulkanTextureResource( VulkanContext *context, const TextureDesc &desc ) : m_context( context ), m_desc( desc )
{
    m_width        = desc.Width;
    m_height       = desc.Height;
    m_depth        = desc.Depth;
    m_format       = desc.Format;
    m_initialState = desc.InitialUsage;

    VkImageType imageType = VK_IMAGE_TYPE_1D;

    if ( m_desc.Depth > 1 )
    {
        imageType = VK_IMAGE_TYPE_3D;
    }
    else if ( m_desc.Height > 1 )
    {
        imageType = VK_IMAGE_TYPE_2D;
    }

    uint32_t usage = m_desc.Usages;
    usage |= m_desc.InitialUsage;

    VkImageCreateInfo imageCreateInfo{ };
    imageCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.format        = VulkanEnumConverter::ConvertImageFormat( desc.Format );
    imageCreateInfo.imageType     = imageType;
    imageCreateInfo.extent.width  = std::max( 1u, m_desc.Width );
    imageCreateInfo.extent.height = std::max( 1u, m_desc.Height );
    imageCreateInfo.extent.depth  = std::max( 1u, m_desc.Depth );
    imageCreateInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage         = VulkanEnumConverter::ConvertTextureUsage( desc.Descriptor, usage );
    imageCreateInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.samples       = VulkanEnumConverter::ConvertSampleCount( desc.MSAASampleCount );
    imageCreateInfo.mipLevels     = desc.MipLevels;
    imageCreateInfo.arrayLayers   = desc.ArraySize;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

    VmaAllocationCreateInfo allocationCreateInfo{ };
    switch ( desc.HeapType )
    {
    case HeapType::GPU:
        allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    case HeapType::CPU:
        allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        break;
    case HeapType::GPU_CPU:
    case HeapType::CPU_GPU:
        allocationCreateInfo.requiredFlags  = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        allocationCreateInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    }
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    VK_CHECK_RESULT( vmaCreateImage( context->Vma, &imageCreateInfo, &allocationCreateInfo, &m_image, &m_allocation, nullptr ) );
    CreateImageView( );

    // This is not super efficient, but vulkan is the only api that doesn't support initial layouts. So this is a simple adaptation.
    // Performance implications can be considered in the future after benchmarking.
    TransitionToInitialLayout( );
}

void VulkanTextureResource::CreateImageView( )
{
    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_1D;
    if ( m_desc.Depth > 1 )
    {
        viewType = VK_IMAGE_VIEW_TYPE_3D;
    }
    else if ( m_desc.Height > 1 )
    {
        viewType = VK_IMAGE_VIEW_TYPE_2D;
    }

    if ( m_desc.Descriptor &ResourceDescriptor::TextureCube )
    {
        viewType = VK_IMAGE_VIEW_TYPE_CUBE;
    }
    if ( m_desc.ArraySize > 1 )
    {
        if ( viewType == VK_IMAGE_VIEW_TYPE_1D )
        {
            viewType = VK_IMAGE_VIEW_TYPE_1D_ARRAY;
        }
        else if ( viewType == VK_IMAGE_VIEW_TYPE_2D )
        {
            viewType = VK_IMAGE_VIEW_TYPE_2D_ARRAY;
        }
        else if ( viewType == VK_IMAGE_VIEW_TYPE_CUBE )
        {
            viewType = VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        }
        else
        {
            spdlog::warn( "Unsupported array size for image type" );
        }
    }
    VkImageViewCreateInfo viewCreateInfo{ };
    viewCreateInfo.sType                           = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewCreateInfo.image                           = m_image;
    viewCreateInfo.viewType                        = viewType;
    viewCreateInfo.format                          = VulkanEnumConverter::ConvertImageFormat( m_desc.Format );
    viewCreateInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.subresourceRange.aspectMask     = VulkanEnumConverter::ConvertImageAspect( m_desc.Aspect );
    viewCreateInfo.subresourceRange.baseMipLevel   = 0;
    viewCreateInfo.subresourceRange.levelCount     = 1; // desc.MipLevels; Mip levels are created individually
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount     = m_desc.ArraySize;

    m_aspect = VulkanEnumConverter::ConvertImageAspect( m_desc.Aspect );

    /* TODO. Support array layers:
    for ( uint32_t i = 0; i < m_desc.ArraySize; ++i )
    {
        viewCreateInfo.subresourceRange.baseArrayLayer = i;
    }
    */

    m_imageViews.resize( m_desc.MipLevels );
    for ( uint32_t j = 0; j < m_desc.MipLevels; ++j )
    {
        viewCreateInfo.subresourceRange.baseMipLevel = j;
        VK_CHECK_RESULT( vkCreateImageView( m_context->LogicalDevice, &viewCreateInfo, nullptr, &m_imageViews[ j ] ) );
    }
}

VulkanTextureResource::VulkanTextureResource( VkImage const &image, VkImageView const &imageView, const VkFormat format, const VkImageAspectFlags imageAspect,
                                              const TextureDesc &desc ) :
    m_desc( desc ), m_image( image ), m_imageViews( { imageView } ), m_vkFormat( format ), m_aspect( imageAspect )
{
    m_width        = desc.Width;
    m_height       = desc.Height;
    m_depth        = desc.Depth;
    m_format       = desc.Format;
    m_initialState = desc.InitialUsage;
    m_isExternal   = true;
}

// Todo transition all mip levels
void VulkanTextureResource::TransitionToInitialLayout( ) const
{
    const VkImageLayout initialLayout = VulkanEnumConverter::ConvertTextureDescriptorToLayout( m_desc.InitialUsage );
    if ( initialLayout == VK_IMAGE_LAYOUT_UNDEFINED )
    {
        return;
    }

    VkCommandBufferAllocateInfo bufferAllocateInfo{ };
    bufferAllocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    bufferAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    bufferAllocateInfo.commandPool        = m_context->GraphicsQueueCommandPool;
    bufferAllocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers( m_context->LogicalDevice, &bufferAllocateInfo, &commandBuffer );

    VkCommandBufferBeginInfo beginInfo{ };
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    VK_CHECK_RESULT( vkBeginCommandBuffer( commandBuffer, &beginInfo ) );

    VkImageMemoryBarrier barrier{ };
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout                       = initialLayout;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = m_image;
    barrier.subresourceRange.aspectMask     = m_aspect;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = m_desc.ArraySize;
    barrier.srcAccessMask                   = 0;
    barrier.dstAccessMask                   = VK_ACCESS_TRANSFER_WRITE_BIT;

    for ( uint32_t j = 0; j < m_desc.MipLevels; ++j )
    {
        barrier.subresourceRange.baseMipLevel = j;
        vkCmdPipelineBarrier( commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier );
    }

    VK_CHECK_RESULT( vkEndCommandBuffer( commandBuffer ) );

    VkSubmitInfo submitInfo{ };
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;

    VK_CHECK_RESULT( vkQueueSubmit( m_context->Queues.at( VulkanQueueType::Graphics ), 1, &submitInfo, nullptr ) );

    NotifyLayoutChange( initialLayout );
}

VulkanTextureResource::~VulkanTextureResource( )
{
    if ( !m_isExternal )
    {
        vmaDestroyImage( m_context->Vma, m_image, m_allocation );

        for ( const auto &imageView : m_imageViews )
        {
            vkDestroyImageView( m_context->LogicalDevice, imageView, nullptr );
        }
    }
}

Format VulkanTextureResource::GetFormat( ) const
{
    return m_format;
}

uint32_t VulkanTextureResource::GetDepth( ) const
{
    return m_depth;
}

uint32_t VulkanTextureResource::GetHeight( ) const
{
    return m_height;
}

uint32_t VulkanTextureResource::GetWidth( ) const
{
    return m_width;
}

uint32_t VulkanTextureResource::InitialState( ) const
{
    return m_initialState;
}

VkImageAspectFlags VulkanTextureResource::Aspect( ) const
{
    return m_aspect;
}

VkImageLayout VulkanTextureResource::Layout( ) const
{
    return m_layout;
}

VkImageView VulkanTextureResource::ImageView( uint32_t mipLevel ) const
{
    return m_imageViews[ mipLevel ];
}

VkImage VulkanTextureResource::Image( ) const
{
    return m_image;
}

void VulkanTextureResource::NotifyLayoutChange( const VkImageLayout newLayout ) const
{
    m_layout = newLayout;
}

VulkanSampler::VulkanSampler( VulkanContext *context, const SamplerDesc &desc ) : m_context( context ), m_desc( desc )
{
    VkSamplerCreateInfo samplerCreateInfo{ };
    samplerCreateInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter               = VulkanEnumConverter::ConvertFilter( desc.MagFilter );
    samplerCreateInfo.minFilter               = VulkanEnumConverter::ConvertFilter( desc.MinFilter );
    samplerCreateInfo.addressModeU            = VulkanEnumConverter::ConvertAddressMode( desc.AddressModeU );
    samplerCreateInfo.addressModeV            = VulkanEnumConverter::ConvertAddressMode( desc.AddressModeV );
    samplerCreateInfo.addressModeW            = VulkanEnumConverter::ConvertAddressMode( desc.AddressModeW );
    samplerCreateInfo.anisotropyEnable        = desc.MaxAnisotropy > 1.0f ? VK_TRUE : VK_FALSE;
    samplerCreateInfo.maxAnisotropy           = desc.MaxAnisotropy;
    samplerCreateInfo.borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    samplerCreateInfo.compareEnable           = desc.CompareOp == CompareOp::Never ? VK_FALSE : VK_TRUE;
    samplerCreateInfo.compareOp               = VulkanEnumConverter::ConvertCompareOp( desc.CompareOp );
    samplerCreateInfo.mipmapMode              = VulkanEnumConverter::ConvertMipmapMode( desc.MipmapMode );
    samplerCreateInfo.mipLodBias              = desc.MipLodBias;
    samplerCreateInfo.minLod                  = desc.MinLod;
    samplerCreateInfo.maxLod                  = desc.MaxLod;

    VK_CHECK_RESULT( vkCreateSampler( m_context->LogicalDevice, &samplerCreateInfo, nullptr, &m_sampler ) );
}

VulkanSampler::~VulkanSampler( )
{
    vkDestroySampler( m_context->LogicalDevice, m_sampler, nullptr );
}

VkSampler VulkanSampler::Instance( ) const
{
    return m_sampler;
}
