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

// This file contains a modified version of some parts The-Forge:
/*
 * Copyright (c) 2017-2024 The Forge Interactive Inc.
 *
 * This file is part of The-Forge
 * (see https://github.com/ConfettiFX/The-Forge).
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanPipelineBarrierHelper.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanBuffer.h"
#include "DenOfIzGraphicsInternal/Backends/Vulkan/VulkanTexture.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#define VK_TEXTURE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::VulkanTexture, handle )
#define VK_BUFFER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::VulkanBuffer, handle )

typedef VkBufferMemoryBarrier memoryBarrier;
using namespace DenOfIz;

void VulkanPipelineBarrierHelper::ExecutePipelineBarrier( const VulkanContext *context, const VkCommandBuffer &commandBuffer, const DenOfIz_QueueType &commandQueueType,
                                                          const DenOfIz_PipelineBarrierDesc *barrier )
{
    VkAccessFlags srcAccessFlags = { };
    VkAccessFlags dstAccessFlags = { };

    std::vector<VkImageMemoryBarrier>      vkImageBarriers;
    const DenOfIz_TextureBarrierDescArray &textureBarriers = barrier->TextureBarriers;
    for ( int i = 0; i < textureBarriers.NumElements; i++ )
    {
        const DenOfIz_TextureBarrierDesc &imageBarrier       = textureBarriers.Elements[ i ];
        VkImageMemoryBarrier              imageMemoryBarrier = CreateImageBarrier( context, imageBarrier, srcAccessFlags, dstAccessFlags, commandQueueType );
        vkImageBarriers.push_back( imageMemoryBarrier );
    }

    std::vector<VkBufferMemoryBarrier>    vkBufferBarriers;
    const DenOfIz_BufferBarrierDescArray &bufferBarriers = barrier->BufferBarriers;
    for ( int i = 0; i < bufferBarriers.NumElements; i++ )
    {
        const DenOfIz_BufferBarrierDesc &bufferBarrier       = bufferBarriers.Elements[ i ];
        memoryBarrier                    bufferMemoryBarrier = CreateBufferBarrier( bufferBarrier, srcAccessFlags, dstAccessFlags, commandQueueType );
        vkBufferBarriers.push_back( bufferMemoryBarrier );
    }
    std::vector<VkMemoryBarrier>          vkMemoryBarriers;
    const DenOfIz_MemoryBarrierDescArray &memoryBarriers = barrier->MemoryBarriers;
    for ( int i = 0; i < memoryBarriers.NumElements; i++ )
    {
        const DenOfIz_MemoryBarrierDesc &memoryBarrier   = memoryBarriers.Elements[ i ];
        VkMemoryBarrier                 &vkMemoryBarrier = vkMemoryBarriers.emplace_back( VkMemoryBarrier{ } );
        vkMemoryBarrier.sType                            = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        srcAccessFlags                                   = GetAccessFlags( memoryBarrier.OldState, commandQueueType );
        dstAccessFlags                                   = GetAccessFlags( memoryBarrier.NewState, commandQueueType );
        vkMemoryBarrier.srcAccessMask                    = srcAccessFlags;
        vkMemoryBarrier.dstAccessMask                    = dstAccessFlags;
    }

    const VkPipelineStageFlags srcStageMask = GetPipelineStageFlags( context, commandQueueType, srcAccessFlags );
    const VkPipelineStageFlags dstStageMask = GetPipelineStageFlags( context, commandQueueType, dstAccessFlags );

    vkCmdPipelineBarrier( commandBuffer, srcStageMask, dstStageMask, VkDependencyFlags{ }, vkMemoryBarriers.size( ), vkMemoryBarriers.data( ), vkBufferBarriers.size( ),
                          vkBufferBarriers.data( ), vkImageBarriers.size( ), vkImageBarriers.data( ) );
}

VkImageMemoryBarrier VulkanPipelineBarrierHelper::CreateImageBarrier( const VulkanContext *context, const DenOfIz_TextureBarrierDesc &barrier, VkAccessFlags &srcAccessFlags,
                                                                      VkAccessFlags &dstAccessFlags, const DenOfIz_QueueType queueType )
{
    const VulkanTexture *imageResource = VK_TEXTURE_IMPL( barrier.Resource );
    VkImageMemoryBarrier imageMemoryBarrier{ };
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

    if ( barrier.OldState & DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT && barrier.NewState & DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT )
    {
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.oldLayout     = VK_IMAGE_LAYOUT_GENERAL;
        imageMemoryBarrier.newLayout     = VK_IMAGE_LAYOUT_GENERAL;
    }
    else
    {
        imageMemoryBarrier.srcAccessMask = GetAccessFlags( barrier.OldState, queueType );
        imageMemoryBarrier.dstAccessMask = GetAccessFlags( barrier.NewState, queueType );
        imageMemoryBarrier.oldLayout     = GetImageLayout( barrier.OldState );
        imageMemoryBarrier.newLayout     = GetImageLayout( barrier.NewState );
        assert( imageMemoryBarrier.newLayout != VK_IMAGE_LAYOUT_UNDEFINED );
    }

    imageMemoryBarrier.image                           = imageResource->Image( );
    imageMemoryBarrier.subresourceRange.aspectMask     = imageResource->Aspect( );
    imageMemoryBarrier.subresourceRange.baseMipLevel   = barrier.EnableSubresourceBarrier ? barrier.MipLevel : 0;
    imageMemoryBarrier.subresourceRange.levelCount     = barrier.EnableSubresourceBarrier ? 1 : VK_REMAINING_MIP_LEVELS;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = barrier.EnableSubresourceBarrier ? barrier.ArrayLayer : 0;
    imageMemoryBarrier.subresourceRange.layerCount     = barrier.EnableSubresourceBarrier ? 1 : VK_REMAINING_ARRAY_LAYERS;

    imageMemoryBarrier.srcQueueFamilyIndex = GetQueueFamilyIndex( context, barrier.SourceQueue );
    if ( barrier.EnableQueueBarrier && ( barrier.OldState & DENOFIZ_RESOURCE_USAGE_UNDEFINED_BIT ) == 0 )
    {
        imageMemoryBarrier.srcQueueFamilyIndex = GetQueueFamilyIndex( context, barrier.SourceQueue );
        imageMemoryBarrier.dstQueueFamilyIndex = GetQueueFamilyIndex( context, barrier.DestinationQueue );
    }
    else
    {
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    }

    srcAccessFlags |= imageMemoryBarrier.srcAccessMask;
    dstAccessFlags |= imageMemoryBarrier.dstAccessMask;
    imageResource->NotifyLayoutChange( imageMemoryBarrier.newLayout );
    return imageMemoryBarrier;
}

VkBufferMemoryBarrier VulkanPipelineBarrierHelper::CreateBufferBarrier( const DenOfIz_BufferBarrierDesc &barrier, VkAccessFlags &srcAccessFlags, VkAccessFlags &dstAccessFlags,
                                                                        const DenOfIz_QueueType queueType )
{
    VkBufferMemoryBarrier memoryBarrier{ };
    memoryBarrier.sType  = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    memoryBarrier.buffer = VK_BUFFER_IMPL( barrier.Resource )->Instance( );
    memoryBarrier.offset = 0;
    memoryBarrier.size   = VK_WHOLE_SIZE;

    if ( barrier.OldState & DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT && barrier.NewState & DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT )
    {
        memoryBarrier.srcAccessMask |= VK_ACCESS_SHADER_WRITE_BIT;
        memoryBarrier.dstAccessMask |= VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
    }
    else
    {
        memoryBarrier.srcAccessMask |= GetAccessFlags( barrier.OldState, queueType );
        memoryBarrier.dstAccessMask |= GetAccessFlags( barrier.NewState, queueType );
    }

    srcAccessFlags |= memoryBarrier.srcAccessMask;
    dstAccessFlags |= memoryBarrier.dstAccessMask;
    return memoryBarrier;
}

VkAccessFlags VulkanPipelineBarrierHelper::GetAccessFlags( const uint32_t &state, const DenOfIz_QueueType queueType )
{
    VkAccessFlags result = 0;

    if ( state & DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT )
    {
        result |= VK_ACCESS_TRANSFER_READ_BIT;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT )
    {
        result |= VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_VERTEX_AND_CONSTANT_BUFFER_BIT )
    {
        result |= VK_ACCESS_UNIFORM_READ_BIT;
        if ( queueType == DENOFIZ_QUEUE_TYPE_GRAPHICS )
        {
            result |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
        }
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_INDEX_BUFFER_BIT )
    {
        result |= VK_ACCESS_INDEX_READ_BIT;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT )
    {
        result |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_INDIRECT_ARGUMENT_BIT )
    {
        result |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT )
    {
        result |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_DEPTH_WRITE_BIT )
    {
        result |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_DEPTH_READ_BIT )
    {
        result |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT )
    {
        result |= VK_ACCESS_SHADER_READ_BIT;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_PRESENT_BIT )
    {
        result |= VK_ACCESS_MEMORY_READ_BIT;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_READ_BIT )
    {
        result |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_ACCELERATION_STRUCTURE_WRITE_BIT )
    {
        result |= VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    }

    return result;
}

VkImageLayout VulkanPipelineBarrierHelper::GetImageLayout( const uint32_t &state )
{
    if ( state & DENOFIZ_RESOURCE_USAGE_COPY_SRC_BIT )
    {
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT )
    {
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT )
    {
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_DEPTH_WRITE_BIT )
    {
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_DEPTH_READ_BIT )
    {
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_UNORDERED_ACCESS_BIT )
    {
        return VK_IMAGE_LAYOUT_GENERAL;
    }
    if ( state & ( DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT | DENOFIZ_RESOURCE_USAGE_PIXEL_SHADER_RESOURCE_BIT ) )
    {
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_PRESENT_BIT )
    {
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }
    if ( state & DENOFIZ_RESOURCE_USAGE_COMMON_BIT )
    {
        return VK_IMAGE_LAYOUT_GENERAL;
    }

    return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkPipelineStageFlags VulkanPipelineBarrierHelper::GetPipelineStageFlags( const VulkanContext *context, const DenOfIz_QueueType queueType, const VkAccessFlags accessFlags )
{
    VkPipelineStageFlags flags = { };

    const auto capabilities = context->SelectedDeviceInfo.Capabilities;
    switch ( queueType )
    {
    case DENOFIZ_QUEUE_TYPE_GRAPHICS:
        {
            if ( accessFlags & ( VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT ) )
            {
                flags |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT;
            }
            if ( accessFlags & ( VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT ) )
            {
                flags |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT;
                flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
                if ( capabilities.GeometryShaders )
                {
                    flags |= VK_PIPELINE_STAGE_GEOMETRY_SHADER_BIT;
                }
                if ( capabilities.Tessellation )
                {
                    flags |= VK_PIPELINE_STAGE_TESSELLATION_CONTROL_SHADER_BIT;
                    flags |= VK_PIPELINE_STAGE_TESSELLATION_EVALUATION_SHADER_BIT;
                }
                flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;

                if ( capabilities.RayTracing )
                {
                    flags |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
                }
            }

            if ( accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT )
                flags |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;

            if ( accessFlags & ( VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ) )
                flags |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

            if ( accessFlags & ( VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT ) )
            {
                flags |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
            }
            break;
        }
    case DENOFIZ_QUEUE_TYPE_COMPUTE:
        {
            if ( accessFlags & ( VK_ACCESS_INDEX_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT ) || accessFlags & VK_ACCESS_INPUT_ATTACHMENT_READ_BIT ||
                 accessFlags & ( VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT ) ||
                 accessFlags & ( VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT ) )
            {
                return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
            }

            if ( accessFlags & ( VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT ) )
            {
                flags |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
            }

            if ( accessFlags & VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR )
            {
                flags |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
            }
            if ( accessFlags & VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR )
            {
                flags |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
            }
            if ( accessFlags & VK_ACCESS_SHADER_READ_BIT )
            {
                flags |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
            }
            break;
        }
    case DENOFIZ_QUEUE_TYPE_COPY:
        return VK_PIPELINE_STAGE_ALL_COMMANDS_BIT;
    }

    if ( accessFlags & VK_ACCESS_INDIRECT_COMMAND_READ_BIT )
    {
        flags |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT;
    }

    if ( accessFlags & ( VK_ACCESS_TRANSFER_READ_BIT | VK_ACCESS_TRANSFER_WRITE_BIT ) )
    {
        flags |= VK_PIPELINE_STAGE_TRANSFER_BIT;
    }

    if ( accessFlags & ( VK_ACCESS_HOST_READ_BIT | VK_ACCESS_HOST_WRITE_BIT ) )
    {
        flags |= VK_PIPELINE_STAGE_HOST_BIT;
    }

    if ( !flags )
    {
        switch ( queueType )
        {
        case DENOFIZ_QUEUE_TYPE_GRAPHICS:
            flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            break;
        case DENOFIZ_QUEUE_TYPE_COMPUTE:
            flags = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT | VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR;
            break;
        case DENOFIZ_QUEUE_TYPE_COPY:
            flags = VK_PIPELINE_STAGE_TRANSFER_BIT;
            break;
        }
    }

    return flags;
}

uint32_t VulkanPipelineBarrierHelper::GetQueueFamilyIndex( const VulkanContext *context, DenOfIz_QueueType queueType )
{
    switch ( queueType )
    {
    case DENOFIZ_QUEUE_TYPE_GRAPHICS:
        return context->QueueFamilies.at( VulkanQueueType::Graphics ).Index;
    case DENOFIZ_QUEUE_TYPE_COPY:
        return context->QueueFamilies.at( VulkanQueueType::Copy ).Index;
    case DENOFIZ_QUEUE_TYPE_COMPUTE:
        return context->QueueFamilies.at( VulkanQueueType::Compute ).Index;
    }

    spdlog::warn( "Unknown queue type: {}", static_cast<int>( queueType ) );
    return VK_QUEUE_FAMILY_IGNORED;
}
