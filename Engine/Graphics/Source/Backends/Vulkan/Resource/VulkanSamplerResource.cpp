/*
DenOfIz - Game Engine/Game
Copyright (c) 2020-2021 Muhammed Murat Cengiz

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

#ifdef BUILD_VK

#include <DenOfIzGraphics/Backends/Vulkan/Resource/VulkanSamplerResource.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanEnumConverter.h>

using namespace DenOfIz;

// Struct VulkanImage; --Start
void VulkanImage::Dispose( const VulkanContext *context ) const
{
    vmaDestroyImage( context->Vma, Instance, Allocation );
    context->LogicalDevice.destroyImageView( ImageView );
    context->LogicalDevice.destroySampler( Sampler );
}

void VulkanImage::Create( VulkanContext *context, const VulkanImageCreateInfo &createInfo )
{
    vk::ImageCreateInfo imageCreateInfo{};

    imageCreateInfo.imageType = vk::ImageType::e2D;
    imageCreateInfo.extent.width = createInfo.Width == 0 ? context->SurfaceExtent.width : createInfo.Width;
    imageCreateInfo.extent.height = createInfo.Height == 0 ? context->SurfaceExtent.height : createInfo.Height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.format = createInfo.Format;
    imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
    imageCreateInfo.usage = createInfo.Usage;
    imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
    imageCreateInfo.samples = createInfo.SampleCount;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    vmaCreateImage( context->Vma, reinterpret_cast<VkImageCreateInfo *>(&imageCreateInfo), &allocationCreateInfo, reinterpret_cast<VkImage *>(&Instance), &Allocation, nullptr );

    vk::ImageViewCreateInfo imageViewCreateInfo{};
    imageViewCreateInfo.image = Instance;
    imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
    imageViewCreateInfo.format = createInfo.Format;
    imageViewCreateInfo.components.r = vk::ComponentSwizzle::eIdentity;
    imageViewCreateInfo.components.g = vk::ComponentSwizzle::eIdentity;
    imageViewCreateInfo.components.b = vk::ComponentSwizzle::eIdentity;
    imageViewCreateInfo.components.a = vk::ComponentSwizzle::eIdentity;
    imageViewCreateInfo.subresourceRange.aspectMask = createInfo.Aspect;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;

    ImageView = context->LogicalDevice.createImageView( imageViewCreateInfo );

    if ( createInfo.Usage & vk::ImageUsageFlagBits::eSampled )
    {
        vk::SamplerCreateInfo samplerCreateInfo{};

        samplerCreateInfo.magFilter = vk::Filter::eNearest;
        samplerCreateInfo.minFilter = vk::Filter::eNearest;
        samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
        samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
        samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
        samplerCreateInfo.maxAnisotropy = 1.0f;
        samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueWhite;
        samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
        samplerCreateInfo.mipLodBias = 0.0f;
        samplerCreateInfo.minLod = 0.0f;
        samplerCreateInfo.maxLod = 1.0f;

        Sampler = context->LogicalDevice.createSampler( samplerCreateInfo );

    }
}

// Struct VulkanImage; --End

VulkanSamplerResource::VulkanSamplerResource( VulkanContext *context, const SamplerCreateInfo &createInfo ) :
    m_CreateInfo( createInfo ), m_Context( context )
{
}

void VulkanSamplerResource::Allocate( const void *newImage )
{
    bool isEmptyImage = newImage == nullptr;
    data = newImage;

    m_MipLevels = static_cast<uint32_t>(std::floor( std::log2( std::max( m_CreateInfo.Width, m_CreateInfo.Height ) ) )) + 1;

    if ( isEmptyImage )
    {
        m_MipLevels = 1;
    }

    const vk::Format format = VulkanEnumConverter::ConvertImageFormat( m_CreateInfo.Format );

    vk::ImageCreateInfo imageCreateInfo{};

    imageCreateInfo.imageType = vk::ImageType::e2D;
    imageCreateInfo.extent.width = m_CreateInfo.Width;
    imageCreateInfo.extent.height = m_CreateInfo.Height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = m_MipLevels;
    imageCreateInfo.arrayLayers = 1;
    imageCreateInfo.format = format;
    imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
    imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageCreateInfo.usage = vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
    imageCreateInfo.samples = vk::SampleCountFlagBits::e1;

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    vmaCreateImage( m_Context->Vma, reinterpret_cast<VkImageCreateInfo *>(&imageCreateInfo), &allocationCreateInfo, reinterpret_cast<VkImage *>(&(m_Image.Instance)), &(m_Image.Allocation),
                    nullptr );

    vk::ImageViewCreateInfo imageViewCreateInfo{};

    imageViewCreateInfo.image = m_Image.Instance;
    imageViewCreateInfo.viewType = vk::ImageViewType::e2D;
    imageViewCreateInfo.format = format;
    imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = m_MipLevels;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 1;

    m_Image.ImageView = m_Context->LogicalDevice.createImageView( imageViewCreateInfo );
    m_Image.Sampler = m_Context->LogicalDevice.createSampler( GetSamplerCreateInfo() );

    ReturnIf( isEmptyImage );

    vk::Buffer stagingBuffer;
    VmaAllocation stagingAllocation;

    VulkanUtilities::InitStagingBuffer( m_Context, stagingBuffer, stagingAllocation, newImage, m_CreateInfo.Width * m_CreateInfo.Height * 4 );
    VulkanUtilities::RunOneTimeCommand( m_Context, [&]( const vk::CommandBuffer &commandBuffer )
    {
        vk::ImageMemoryBarrier memoryBarrier{};

        memoryBarrier.oldLayout = vk::ImageLayout::eUndefined;
        memoryBarrier.newLayout = vk::ImageLayout::eTransferDstOptimal;
        memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        memoryBarrier.image = m_Image.Instance;
        memoryBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        memoryBarrier.subresourceRange.baseMipLevel = 0;
        memoryBarrier.subresourceRange.levelCount = m_MipLevels;
        memoryBarrier.subresourceRange.baseArrayLayer = 0;
        memoryBarrier.subresourceRange.layerCount = 1;
        memoryBarrier.srcAccessMask = {};
        memoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

        commandBuffer.pipelineBarrier( vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, 0, nullptr, 0, nullptr, 1, &memoryBarrier );

        vk::BufferImageCopy bufferImageCopy{};

        bufferImageCopy.bufferOffset = 0;
        bufferImageCopy.bufferRowLength = 0;
        bufferImageCopy.bufferImageHeight = 0;
        bufferImageCopy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
        bufferImageCopy.imageSubresource.mipLevel = 0;
        bufferImageCopy.imageSubresource.baseArrayLayer = 0;
        bufferImageCopy.imageSubresource.layerCount = 1;
        bufferImageCopy.imageOffset = vk::Offset3D{ 0, 0, 0 };
        bufferImageCopy.imageExtent = vk::Extent3D{ m_CreateInfo.Width, m_CreateInfo.Height, 1 };

        commandBuffer.copyBufferToImage( stagingBuffer, m_Image.Instance, vk::ImageLayout::eTransferDstOptimal, 1, &bufferImageCopy );
    } );

    vmaDestroyBuffer( m_Context->Vma, stagingBuffer, stagingAllocation );

    vk::FormatProperties properties = m_Context->PhysicalDevice.getFormatProperties( format );

    if ( (properties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear) != vk::FormatFeatureFlagBits::eSampledImageFilterLinear )
    {
        throw std::runtime_error( "Unsupported device, VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT must be  supported" );
    }

    GenerateMipMaps();

    DescriptorInfo.imageLayout = vk::ImageLayout::eColorAttachmentOptimal;
    DescriptorInfo.imageView = m_Image.ImageView;
    DescriptorInfo.sampler = m_Image.Sampler;
}

void VulkanSamplerResource::GenerateMipMaps() const
{
    int32_t mipWidth = m_CreateInfo.Width, mipHeight = m_CreateInfo.Height;

    VulkanUtilities::RunOneTimeCommand( m_Context, [&]( vk::CommandBuffer &commandBuffer )
    {
        /*
         * Continuously copy current image( in mip level=index ) to the next, ie.
         * Iteration 1: i - 1 = 0, 512x512 is copied to i as 512/2
         * Iteration 2: i - 1 = 0, 512/2 is copied to i as 512/2/2
         *
         * In such form every mip level of the image is filled with an image with half the size of the previous image
         */
        for ( uint32_t index = 1; index < m_MipLevels; ++index )
        {
            vk::ImageMemoryBarrier memoryBarrier{};

            memoryBarrier.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            memoryBarrier.newLayout = vk::ImageLayout::eTransferSrcOptimal;
            memoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            memoryBarrier.image = m_Image.Instance;
            memoryBarrier.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            memoryBarrier.subresourceRange.baseMipLevel = index - 1;
            memoryBarrier.subresourceRange.levelCount = 1;
            memoryBarrier.subresourceRange.baseArrayLayer = 0;
            memoryBarrier.subresourceRange.layerCount = 1;
            memoryBarrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
            memoryBarrier.dstAccessMask = vk::AccessFlagBits::eTransferRead;

            commandBuffer.pipelineBarrier( vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eTransfer, {}, 0, nullptr, 0, nullptr, 1, &memoryBarrier );

            vk::ImageBlit imageBlit{};

            imageBlit.srcOffsets[ 0 ] = vk::Offset3D{ 0, 0, 0 };
            imageBlit.dstOffsets[ 0 ] = vk::Offset3D{ 0, 0, 0 };
            imageBlit.srcOffsets[ 1 ] = vk::Offset3D{ mipWidth, mipHeight, 1 };
            imageBlit.dstOffsets[ 1 ] = vk::Offset3D{ mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };

            imageBlit.srcSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            imageBlit.srcSubresource.mipLevel = index - 1;
            imageBlit.srcSubresource.baseArrayLayer = 0;
            imageBlit.srcSubresource.layerCount = 1;
            imageBlit.dstSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            imageBlit.dstSubresource.mipLevel = index;
            imageBlit.dstSubresource.baseArrayLayer = 0;
            imageBlit.dstSubresource.layerCount = 1;

            commandBuffer.blitImage( m_Image.Instance, vk::ImageLayout::eTransferSrcOptimal, m_Image.Instance, vk::ImageLayout::eTransferDstOptimal, 1, &imageBlit,
                                     vk::Filter::eLinear );

            /* After the blit transition the image to the form that will be used by the shader */
            vk::ImageMemoryBarrier toShaderFormat{};

            toShaderFormat.oldLayout = vk::ImageLayout::eTransferSrcOptimal;
            toShaderFormat.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            toShaderFormat.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            toShaderFormat.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            toShaderFormat.image = m_Image.Instance;
            toShaderFormat.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            toShaderFormat.subresourceRange.baseMipLevel = index - 1;
            toShaderFormat.subresourceRange.levelCount = 1;
            toShaderFormat.subresourceRange.baseArrayLayer = 0;
            toShaderFormat.subresourceRange.layerCount = 1;
            toShaderFormat.srcAccessMask = vk::AccessFlagBits::eTransferRead;
            toShaderFormat.dstAccessMask = vk::AccessFlagBits::eShaderRead;

            commandBuffer.pipelineBarrier( vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, 0, nullptr, 0, nullptr, 1, &toShaderFormat );

            if ( mipWidth > 1 )
            {
                mipWidth /= 2;
            }
            if ( mipHeight > 1 )
            {
                mipHeight /= 2;
            }
        }

        vk::ImageMemoryBarrier finalFormat{};

        finalFormat.oldLayout = vk::ImageLayout::eTransferDstOptimal;
        finalFormat.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
        finalFormat.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        finalFormat.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        finalFormat.image = m_Image.Instance;
        finalFormat.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
        finalFormat.subresourceRange.baseMipLevel = m_MipLevels - 1;
        finalFormat.subresourceRange.levelCount = 1;
        finalFormat.subresourceRange.baseArrayLayer = 0;
        finalFormat.subresourceRange.layerCount = 1;
        finalFormat.srcAccessMask = vk::AccessFlagBits::eTransferRead;
        finalFormat.dstAccessMask = vk::AccessFlagBits::eShaderRead;

        commandBuffer.pipelineBarrier( vk::PipelineStageFlagBits::eTransfer, vk::PipelineStageFlagBits::eFragmentShader, {}, 0, nullptr, 0, nullptr, 1, &finalFormat );
    } );
}

vk::SamplerCreateInfo VulkanSamplerResource::GetSamplerCreateInfo() const
{
    vk::SamplerCreateInfo samplerCreateInfo{};

    samplerCreateInfo.magFilter = VulkanEnumConverter::ConvertFilter( m_CreateInfo.MagFilter );
    samplerCreateInfo.minFilter = VulkanEnumConverter::ConvertFilter( m_CreateInfo.MinFilter );
    samplerCreateInfo.addressModeU = VulkanEnumConverter::ConvertAddressMode( m_CreateInfo.AddressModeU );
    samplerCreateInfo.addressModeV = VulkanEnumConverter::ConvertAddressMode( m_CreateInfo.AddressModeV );
    samplerCreateInfo.addressModeW = VulkanEnumConverter::ConvertAddressMode( m_CreateInfo.AddressModeW );
    samplerCreateInfo.anisotropyEnable = m_CreateInfo.AnisotropyEnable;
    samplerCreateInfo.maxAnisotropy = m_CreateInfo.MaxAnisotropy;
    samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueBlack;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    samplerCreateInfo.compareEnable = m_CreateInfo.CompareEnable;
    samplerCreateInfo.compareOp = VulkanEnumConverter::ConvertCompareOp( m_CreateInfo.CompareOp );
    samplerCreateInfo.mipmapMode = VulkanEnumConverter::ConvertMipmapMode( m_CreateInfo.MipmapMode );
    samplerCreateInfo.mipLodBias = m_CreateInfo.MipLodBias;
    samplerCreateInfo.minLod = m_CreateInfo.MinLod;
    samplerCreateInfo.maxLod = m_MipLevels;

    return samplerCreateInfo;
}

void VulkanSamplerResource::Deallocate()
{
    m_Image.Dispose( m_Context );
}

#endif
