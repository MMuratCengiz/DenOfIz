/*
Blazar Engine - 3D Game Engine
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

#include <DenOfIzGraphics/Backends/Vulkan/Resource/VulkanCubeMapResource.h>

using namespace DenOfIz;

VulkanCubeMapResource::VulkanCubeMapResource( VulkanContext *context, const CubeMapCreateInfo &createInfo ) :
    m_Context( context ), m_CreateInfo( createInfo ), m_Allocation( nullptr )
{
}

void VulkanCubeMapResource::Allocate( std::vector<const void *> data )
{
    assert( !m_CreateInfo.Samplers.empty() );
    assert( m_CreateInfo.Samplers.size() != data.size() );

    int width = m_CreateInfo.Samplers[ 0 ].Width;
    int height = m_CreateInfo.Samplers[ 0 ].Height;

    std::vector<std::pair<vk::Buffer, VmaAllocation>> stagingBuffers( m_CreateInfo.Samplers.size() );

    int mipStagingBufferIndex = 0;

    uint32_t index = 0;
    for ( const auto &img : m_CreateInfo.Samplers )
    {
        auto &[buffer, allocation] = stagingBuffers[ mipStagingBufferIndex++ ];

        VulkanUtilities::InitStagingBuffer( m_Context, buffer, allocation, data[ index++ ], img.Width * img.Height * 4 );
    }

    vk::ImageCreateInfo imageCreateInfo{};

    imageCreateInfo.imageType = vk::ImageType::e2D;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 6;
    imageCreateInfo.format = vk::Format::eR8G8B8A8Srgb;
    imageCreateInfo.tiling = vk::ImageTiling::eOptimal;
    imageCreateInfo.initialLayout = vk::ImageLayout::eUndefined;
    imageCreateInfo.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
    imageCreateInfo.sharingMode = vk::SharingMode::eExclusive;
    imageCreateInfo.samples = vk::SampleCountFlagBits::e1;
    imageCreateInfo.flags = vk::ImageCreateFlagBits::eCubeCompatible;

    VmaAllocationCreateInfo allocationCreateInfo{};
    allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

    vmaCreateImage( m_Context->Vma, (VkImageCreateInfo *)&imageCreateInfo, &allocationCreateInfo, (VkImage *)&m_Image, &m_Allocation, nullptr );

    vk::ImageViewCreateInfo imageViewCreateInfo{};

    imageViewCreateInfo.image = m_Image;
    imageViewCreateInfo.viewType = vk::ImageViewType::eCube;
    imageViewCreateInfo.format = vk::Format::eR8G8B8A8Srgb;
    imageViewCreateInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
    imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
    imageViewCreateInfo.subresourceRange.levelCount = 1;
    imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
    imageViewCreateInfo.subresourceRange.layerCount = 6;

    m_ImageView = m_Context->LogicalDevice.createImageView( imageViewCreateInfo );

    vk::SamplerCreateInfo samplerCreateInfo{};

    samplerCreateInfo.magFilter = vk::Filter::eLinear;
    samplerCreateInfo.minFilter = vk::Filter::eLinear;
    samplerCreateInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
    samplerCreateInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
    samplerCreateInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
    samplerCreateInfo.anisotropyEnable = VK_TRUE;
    samplerCreateInfo.maxAnisotropy = 16.0f;
    samplerCreateInfo.borderColor = vk::BorderColor::eFloatOpaqueBlack;
    samplerCreateInfo.unnormalizedCoordinates = VK_FALSE;
    samplerCreateInfo.compareEnable = VK_FALSE;
    samplerCreateInfo.compareOp = vk::CompareOp::eAlways;
    samplerCreateInfo.mipmapMode = vk::SamplerMipmapMode::eLinear;
    samplerCreateInfo.mipLodBias = 0.0f;
    samplerCreateInfo.minLod = 1.0f;
    samplerCreateInfo.maxLod = 1.0f;

    m_Sampler = m_Context->LogicalDevice.createSampler( samplerCreateInfo );

    int arrayLayer = 0;
    for ( auto &[buffer, allocation] : stagingBuffers )
    {
        VulkanUtilities::RunOneTimeCommand( m_Context, [&]( const vk::CommandBuffer &commandBuffer )
        {
            vk::ImageMemoryBarrier toTransferOptimal{};

            toTransferOptimal.oldLayout = vk::ImageLayout::eUndefined;
            toTransferOptimal.newLayout = vk::ImageLayout::eTransferDstOptimal;
            toTransferOptimal.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            toTransferOptimal.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            toTransferOptimal.image = m_Image;
            toTransferOptimal.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            toTransferOptimal.subresourceRange.baseMipLevel = 0;
            toTransferOptimal.subresourceRange.levelCount = 1;
            toTransferOptimal.subresourceRange.baseArrayLayer = arrayLayer;
            toTransferOptimal.subresourceRange.layerCount = 1;
            toTransferOptimal.srcAccessMask = {};
            toTransferOptimal.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

            commandBuffer.pipelineBarrier( vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, 0, nullptr, 0, nullptr, 1, &toTransferOptimal );

            vk::BufferImageCopy bufferImageCopy{};

            bufferImageCopy.bufferOffset = 0;
            bufferImageCopy.bufferRowLength = 0;
            bufferImageCopy.bufferImageHeight = 0;
            bufferImageCopy.imageSubresource.aspectMask = vk::ImageAspectFlagBits::eColor;
            bufferImageCopy.imageSubresource.mipLevel = 0;
            bufferImageCopy.imageSubresource.baseArrayLayer = arrayLayer;
            bufferImageCopy.imageSubresource.layerCount = 1;
            bufferImageCopy.imageOffset = vk::Offset3D{ 0, 0, 0 };
            bufferImageCopy.imageExtent = vk::Extent3D{ static_cast<uint32_t>(width), static_cast<uint32_t>(height), 1 };

            commandBuffer.copyBufferToImage( buffer, m_Image, vk::ImageLayout::eTransferDstOptimal, 1, &bufferImageCopy );

            vk::ImageMemoryBarrier toShaderOptimal{};

            toShaderOptimal.oldLayout = vk::ImageLayout::eTransferDstOptimal;
            toShaderOptimal.newLayout = vk::ImageLayout::eShaderReadOnlyOptimal;
            toShaderOptimal.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            toShaderOptimal.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
            toShaderOptimal.image = m_Image;
            toShaderOptimal.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
            toShaderOptimal.subresourceRange.baseMipLevel = 0;
            toShaderOptimal.subresourceRange.levelCount = 1;
            toShaderOptimal.subresourceRange.baseArrayLayer = arrayLayer;
            toShaderOptimal.subresourceRange.layerCount = 1;
            toShaderOptimal.srcAccessMask = {};
            toShaderOptimal.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

            commandBuffer.pipelineBarrier( vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eTransfer, {}, 0, nullptr, 0, nullptr, 1, &toShaderOptimal );
        } );

        vmaDestroyBuffer( m_Context->Vma, buffer, allocation );
        arrayLayer++;
    }
}

void VulkanCubeMapResource::Deallocate()
{
    vmaDestroyImage( m_Context->Vma, m_Image, m_Allocation );
    m_Context->LogicalDevice.destroyImageView( m_ImageView );
    m_Context->LogicalDevice.destroySampler( m_Sampler );
}

#endif
