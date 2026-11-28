/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanTexture.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanEnumConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/StringUtilities.h"

using namespace DenOfIz;

VulkanTexture::VulkanTexture( VulkanContext *context, const DenOfIz_TextureDesc &desc ) :
    m_context( context ), m_desc( desc ), m_debugName( StringViewToStdString( desc.DebugName ) )
{
    if ( !m_debugName.empty( ) )
    {
        m_desc.DebugName = DenOfIz_StringView( m_debugName.c_str( ), static_cast<uint32_t>( m_debugName.size( ) ) );
    }
    else
    {
        m_desc.DebugName = { };
    }

    m_width  = desc.Width;
    m_height = desc.Height;
    m_depth  = desc.Depth;
    m_format = desc.Format;

    VkImageType imageType = VK_IMAGE_TYPE_2D;

    if ( m_desc.Depth > 1 )
    {
        imageType = VK_IMAGE_TYPE_3D;
    }
    else if ( m_desc.Height == 0 )
    {
        imageType = VK_IMAGE_TYPE_1D;
    }

    VkImageCreateInfo imageCreateInfo{ };
    imageCreateInfo.sType         = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.format        = DenOfIz_VulkanEnumConverter_ConvertImageFormat( desc.Format );
    imageCreateInfo.imageType     = imageType;
    imageCreateInfo.extent.width  = std::max( 1u, m_desc.Width );
    imageCreateInfo.extent.height = std::max( 1u, m_desc.Height );
    imageCreateInfo.extent.depth  = std::max( 1u, m_desc.Depth );
    imageCreateInfo.tiling        = VK_IMAGE_TILING_OPTIMAL;
    imageCreateInfo.usage         = DenOfIz_VulkanEnumConverter_ConvertTextureUsageToVk( desc.Usage, desc.Format );
    imageCreateInfo.sharingMode   = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.samples       = DenOfIz_VulkanEnumConverter_ConvertSampleCount( desc.MSAASampleCount );
    imageCreateInfo.mipLevels     = desc.MipLevels;
    imageCreateInfo.arrayLayers   = desc.ArraySize;
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    if ( context->SharedQueueFamilies.size( ) > 1 )
    {
        imageCreateInfo.sharingMode           = VK_SHARING_MODE_CONCURRENT;
        imageCreateInfo.queueFamilyIndexCount = static_cast<uint32_t>( context->SharedQueueFamilies.size( ) );
        imageCreateInfo.pQueueFamilyIndices   = context->SharedQueueFamilies.data( );
    }

    if ( m_desc.Usage & DENOFIZ_TEXTURE_USAGE_CUBE_BIT )
    {
        imageCreateInfo.flags |= VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }

    VmaAllocationCreateInfo allocationCreateInfo{ };
    switch ( desc.HeapType )
    {
    case DENOFIZ_HEAP_TYPE_GPU:
        allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    case DENOFIZ_HEAP_TYPE_CPU:
        allocationCreateInfo.requiredFlags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        break;
    case DENOFIZ_HEAP_TYPE_GPU_CPU:
    case DENOFIZ_HEAP_TYPE_CPU_GPU:
        allocationCreateInfo.requiredFlags  = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        allocationCreateInfo.preferredFlags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        break;
    }
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    if ( const VkResult result = vmaCreateImage( context->Vma, &imageCreateInfo, &allocationCreateInfo, &m_image, &m_allocation, nullptr ); result != VK_SUCCESS )
    {
        spdlog::error( "[Vulkan] Failed to create a {}x{} texture: {}", m_width, m_height, static_cast<int>( result ) );
        m_image      = VK_NULL_HANDLE;
        m_allocation = VK_NULL_HANDLE;
        return;
    }
    CreateImageView( );

    TransitionToInitialLayout( );
}

void VulkanTexture::CreateImageView( )
{
    VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
    if ( m_desc.Depth > 1 )
    {
        viewType = VK_IMAGE_VIEW_TYPE_3D;
    }
    else if ( m_desc.Height == 0 )
    {
        viewType = VK_IMAGE_VIEW_TYPE_1D;
    }

    if ( m_desc.Usage & DENOFIZ_TEXTURE_USAGE_CUBE_BIT )
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
    viewCreateInfo.format                          = DenOfIz_VulkanEnumConverter_ConvertImageFormat( m_desc.Format );
    viewCreateInfo.components.r                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.components.g                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.components.b                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.components.a                    = VK_COMPONENT_SWIZZLE_IDENTITY;
    viewCreateInfo.subresourceRange.aspectMask     = DenOfIz_VulkanEnumConverter_GetImageAspectFromFormat( m_desc.Format );
    viewCreateInfo.subresourceRange.baseMipLevel   = 0;
    viewCreateInfo.subresourceRange.levelCount     = 1;
    viewCreateInfo.subresourceRange.baseArrayLayer = 0;
    viewCreateInfo.subresourceRange.layerCount     = m_desc.ArraySize;

    m_aspect = DenOfIz_VulkanEnumConverter_GetImageAspectFromFormat( m_desc.Format );

    m_imageViews.resize( m_desc.MipLevels );
    for ( uint32_t j = 0; j < m_desc.MipLevels; ++j )
    {
        viewCreateInfo.subresourceRange.baseMipLevel = j;
        VK_CHECK_RESULT( vkCreateImageView( m_context->LogicalDevice, &viewCreateInfo, nullptr, &m_imageViews[ j ] ) );
    }

    viewCreateInfo.subresourceRange.baseMipLevel = 0;
    viewCreateInfo.subresourceRange.levelCount   = m_desc.MipLevels;
    VK_CHECK_RESULT( vkCreateImageView( m_context->LogicalDevice, &viewCreateInfo, nullptr, &m_srvImageView ) );
}

VulkanTexture::VulkanTexture( VkImage const &image, VkImageView const &imageView, const VkFormat format, const VkImageAspectFlags imageAspect, const DenOfIz_TextureDesc &desc ) :
    m_desc( desc ), m_debugName( StringViewToStdString( desc.DebugName ) ), m_image( image ), m_imageViews( { imageView } ), m_srvImageView( imageView ), m_vkFormat( format ),
    m_aspect( imageAspect ), m_layout( VK_IMAGE_LAYOUT_UNDEFINED )
{
    if ( !m_debugName.empty( ) )
    {
        m_desc.DebugName = DenOfIz_StringView( m_debugName.c_str( ), static_cast<uint32_t>( m_debugName.size( ) ) );
    }
    else
    {
        m_desc.DebugName = { };
    }

    m_width      = desc.Width;
    m_height     = desc.Height;
    m_depth      = desc.Depth;
    m_format     = desc.Format;
    m_isExternal = true;
}

void VulkanTexture::UpdateExternal( const VkImage &image, const VkImageView &imageView, const DenOfIz_TextureDesc &desc )
{
    m_image      = image;
    m_imageViews = { imageView };
    m_srvImageView = imageView;
    m_layout     = VK_IMAGE_LAYOUT_UNDEFINED;
    m_width      = desc.Width;
    m_height     = desc.Height;
    m_desc.Width  = desc.Width;
    m_desc.Height = desc.Height;
}

void VulkanTexture::TransitionToInitialLayout( ) const
{
    const VkImageLayout initialLayout = VK_IMAGE_LAYOUT_GENERAL;
    if ( m_context->IsDeviceLost )
    {
        return;
    }

    VkCommandBufferAllocateInfo bufferAllocateInfo{ };
    bufferAllocateInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    bufferAllocateInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    bufferAllocateInfo.commandPool        = m_context->GraphicsQueueCommandPool;
    bufferAllocateInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer = VK_NULL_HANDLE;
    VkResult        result        = vkAllocateCommandBuffers( m_context->LogicalDevice, &bufferAllocateInfo, &commandBuffer );
    if ( result != VK_SUCCESS || commandBuffer == VK_NULL_HANDLE )
    {
        spdlog::error( "[VulkanTexture] Failed to allocate command buffer for initial layout transition" );
        return;
    }

    VkCommandBufferBeginInfo beginInfo{ };
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

    result = vkBeginCommandBuffer( commandBuffer, &beginInfo );
    if ( result != VK_SUCCESS )
    {
        spdlog::error( "[VulkanTexture] Failed to begin command buffer for initial layout transition" );
        vkFreeCommandBuffers( m_context->LogicalDevice, m_context->GraphicsQueueCommandPool, 1, &commandBuffer );
        return;
    }

    VkImageMemoryBarrier barrier{ };
    barrier.sType                           = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout                       = VK_IMAGE_LAYOUT_UNDEFINED;
    barrier.newLayout                       = initialLayout;
    barrier.srcQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex             = VK_QUEUE_FAMILY_IGNORED;
    barrier.image                           = m_image;
    barrier.subresourceRange.aspectMask     = m_aspect;
    barrier.subresourceRange.baseMipLevel   = 0;
    barrier.subresourceRange.levelCount     = m_desc.MipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount     = m_desc.ArraySize;
    barrier.srcAccessMask                   = 0;
    barrier.dstAccessMask                   = VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;

    vkCmdPipelineBarrier( commandBuffer, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier );

    result = vkEndCommandBuffer( commandBuffer );
    if ( result != VK_SUCCESS )
    {
        spdlog::error( "[VulkanTexture] Failed to end command buffer for initial layout transition" );
        vkFreeCommandBuffers( m_context->LogicalDevice, m_context->GraphicsQueueCommandPool, 1, &commandBuffer );
        return;
    }

    VkSubmitInfo submitInfo{ };
    submitInfo.sType              = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers    = &commandBuffer;

    result = vkQueueSubmit( m_context->TextureSetupQueue, 1, &submitInfo, nullptr );
    if ( m_context->CheckDeviceLost( result, "vkQueueSubmit" ) )
    {
        vkFreeCommandBuffers( m_context->LogicalDevice, m_context->GraphicsQueueCommandPool, 1, &commandBuffer );
        return;
    }
    if ( result != VK_SUCCESS )
    {
        spdlog::error( "[VulkanTexture] Failed to submit initial layout transition" );
        vkFreeCommandBuffers( m_context->LogicalDevice, m_context->GraphicsQueueCommandPool, 1, &commandBuffer );
        return;
    }

    result = vkQueueWaitIdle( m_context->TextureSetupQueue );
    if ( !m_context->CheckDeviceLost( result, "vkQueueWaitIdle" ) && result != VK_SUCCESS )
    {
        spdlog::error( "[VulkanTexture] Failed to wait for initial layout transition" );
    }

    vkFreeCommandBuffers( m_context->LogicalDevice, m_context->GraphicsQueueCommandPool, 1, &commandBuffer );

    NotifyLayoutChange( initialLayout );
}

VulkanTexture::~VulkanTexture( )
{
    if ( !m_isExternal )
    {
        vmaDestroyImage( m_context->Vma, m_image, m_allocation );

        for ( const auto &imageView : m_imageViews )
        {
            vkDestroyImageView( m_context->LogicalDevice, imageView, nullptr );
        }
        vkDestroyImageView( m_context->LogicalDevice, m_srvImageView, nullptr );
    }
}

DenOfIz_Format VulkanTexture::GetFormat( ) const
{
    return m_format;
}

uint32_t VulkanTexture::GetMipLevels( ) const
{
    return m_desc.MipLevels;
}

uint32_t VulkanTexture::GetDepth( ) const
{
    return m_depth;
}

uint32_t VulkanTexture::GetHeight( ) const
{
    return m_height;
}

uint32_t VulkanTexture::GetWidth( ) const
{
    return m_width;
}

VkImageAspectFlags VulkanTexture::Aspect( ) const
{
    return m_aspect;
}

VkImageLayout VulkanTexture::Layout( ) const
{
    return m_layout;
}

VkImageView VulkanTexture::ImageView( uint32_t mipLevel ) const
{
    return m_imageViews[ mipLevel ];
}

VkImageView VulkanTexture::SrvImageView( ) const
{
    return m_srvImageView;
}

VkImage VulkanTexture::Image( ) const
{
    return m_image;
}

void VulkanTexture::NotifyLayoutChange( const VkImageLayout newLayout ) const
{
    m_layout = newLayout;
}

VulkanSampler::VulkanSampler( VulkanContext *context, const DenOfIz_SamplerDesc &desc ) :
    m_context( context ), m_desc( desc ), m_debugName( StringViewToStdString( desc.DebugName ) )
{
    VkSamplerCreateInfo samplerCreateInfo{ };
    samplerCreateInfo.sType                   = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter               = DenOfIz_VulkanEnumConverter_ConvertFilter( desc.MagFilter );
    samplerCreateInfo.minFilter               = DenOfIz_VulkanEnumConverter_ConvertFilter( desc.MinFilter );
    samplerCreateInfo.addressModeU            = DenOfIz_VulkanEnumConverter_ConvertAddressMode( desc.AddressModeU );
    samplerCreateInfo.addressModeV            = DenOfIz_VulkanEnumConverter_ConvertAddressMode( desc.AddressModeV );
    samplerCreateInfo.addressModeW            = DenOfIz_VulkanEnumConverter_ConvertAddressMode( desc.AddressModeW );
    samplerCreateInfo.anisotropyEnable        = desc.MaxAnisotropy > 1.0f ? VK_TRUE : VK_FALSE;
    samplerCreateInfo.maxAnisotropy           = desc.MaxAnisotropy;
    samplerCreateInfo.borderColor             = VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    samplerCreateInfo.compareEnable           = desc.CompareOp == DENOFIZ_COMPARE_OP_NEVER ? VK_FALSE : VK_TRUE;
    samplerCreateInfo.compareOp               = DenOfIz_VulkanEnumConverter_ConvertCompareOp( desc.CompareOp );
    samplerCreateInfo.mipmapMode              = DenOfIz_VulkanEnumConverter_ConvertMipmapMode( desc.MipmapMode );
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
