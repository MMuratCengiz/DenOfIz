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

#include <DenOfIzGraphics/Backends/Vulkan/VulkanBufferResource.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanPipelineBarrierHelper.h>
#include <DenOfIzGraphics/Backends/Vulkan/VulkanTextureResource.h>

using namespace DenOfIz;

void VulkanPipelineBarrierHelper::ExecutePipelineBarrier( const VulkanContext *context, const VkCommandBuffer &commandBuffer, const QueueType &commandQueueType,
                                                          const PipelineBarrierDesc &barrier )
{
    VkAccessFlags srcAccessFlags = { };
    VkAccessFlags dstAccessFlags = { };

    std::vector<VkImageMemoryBarrier> imageBarriers;
    for ( const TextureBarrierDesc &imageBarrier : barrier.GetTextureBarriers( ) )
    {
        VkImageMemoryBarrier imageMemoryBarrier = CreateImageBarrier( imageBarrier, srcAccessFlags, dstAccessFlags );
        imageBarriers.push_back( imageMemoryBarrier );
    }

    std::vector<VkBufferMemoryBarrier> bufferBarriers;
    for ( const BufferBarrierDesc &bufferBarrier : barrier.GetBufferBarriers( ) )
    {
        VkBufferMemoryBarrier bufferMemoryBarrier = CreateBufferBarrier( bufferBarrier, srcAccessFlags, dstAccessFlags );
        bufferBarriers.push_back( bufferMemoryBarrier );
    }
    const std::vector<VkMemoryBarrier> memoryBarriers; // Todo

    const VkPipelineStageFlags srcStageMask = GetPipelineStageFlags( context, commandQueueType, srcAccessFlags );
    const VkPipelineStageFlags dstStageMask = GetPipelineStageFlags( context, commandQueueType, dstAccessFlags );

    vkCmdPipelineBarrier( commandBuffer, srcStageMask, dstStageMask, VkDependencyFlags{ }, memoryBarriers.size( ), memoryBarriers.data( ), bufferBarriers.size( ),
                          bufferBarriers.data( ), static_cast<uint32_t>( imageBarriers.size( ) ), imageBarriers.data( ) );
}

VkImageMemoryBarrier VulkanPipelineBarrierHelper::CreateImageBarrier( const TextureBarrierDesc &barrier, VkAccessFlags &srcAccessFlags, VkAccessFlags &dstAccessFlags )
{
    const VulkanTextureResource *imageResource = dynamic_cast<VulkanTextureResource *>( barrier.Resource );
    VkImageMemoryBarrier         imageMemoryBarrier{ };
    imageMemoryBarrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;

    if ( barrier.OldState.IsSet( ResourceState::UnorderedAccess ) && barrier.NewState.IsSet( ResourceState::UnorderedAccess ) )
    {
        imageMemoryBarrier.srcAccessMask = VK_ACCESS_SHADER_WRITE_BIT;
        imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
        imageMemoryBarrier.oldLayout     = VK_IMAGE_LAYOUT_GENERAL;
        imageMemoryBarrier.newLayout     = VK_IMAGE_LAYOUT_GENERAL;
    }
    else
    {
        imageMemoryBarrier.srcAccessMask = GetAccessFlags( barrier.OldState );
        imageMemoryBarrier.dstAccessMask = GetAccessFlags( barrier.NewState );
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

    imageMemoryBarrier.srcQueueFamilyIndex = barrier.SourceQueue;
    if ( barrier.EnableQueueBarrier && !barrier.OldState.IsSet( ResourceState::Undefined ) )
    {
        imageMemoryBarrier.srcQueueFamilyIndex = barrier.SourceQueue;
        imageMemoryBarrier.dstQueueFamilyIndex = barrier.DestinationQueue;
    }
    else
    {
        imageMemoryBarrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        imageMemoryBarrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    }

    srcAccessFlags |= imageMemoryBarrier.srcAccessMask;
    dstAccessFlags |= imageMemoryBarrier.dstAccessMask;

    return imageMemoryBarrier;
}

VkBufferMemoryBarrier VulkanPipelineBarrierHelper::CreateBufferBarrier( const BufferBarrierDesc &barrier, VkAccessFlags &srcAccessFlags, VkAccessFlags &dstAccessFlags )
{
    VkBufferMemoryBarrier memoryBarrier{ };
    memoryBarrier.sType  = VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER;
    memoryBarrier.buffer = dynamic_cast<VulkanBufferResource *>( barrier.Resource )->Instance( );
    memoryBarrier.offset = 0;
    memoryBarrier.size   = VK_WHOLE_SIZE;

    if ( barrier.OldState.IsSet( ResourceState::UnorderedAccess ) && barrier.NewState.IsSet( ResourceState::UnorderedAccess ) )
    {
        memoryBarrier.srcAccessMask |= VK_ACCESS_SHADER_WRITE_BIT;
        memoryBarrier.dstAccessMask |= VK_ACCESS_SHADER_WRITE_BIT | VK_ACCESS_SHADER_READ_BIT;
    }
    else
    {
        memoryBarrier.srcAccessMask |= GetAccessFlags( barrier.OldState );
        memoryBarrier.dstAccessMask |= GetAccessFlags( barrier.NewState );
    }

    srcAccessFlags |= memoryBarrier.srcAccessMask;
    dstAccessFlags |= memoryBarrier.dstAccessMask;
    return memoryBarrier;
}

VkAccessFlags VulkanPipelineBarrierHelper::GetAccessFlags( const BitSet<ResourceState> &state )
{
    VkAccessFlags result = 0;

    if ( state.IsSet( ResourceState::CopySrc ) )
    {
        result |= VK_ACCESS_TRANSFER_READ_BIT;
    }
    if ( state.IsSet( ResourceState::CopyDst ) )
    {
        result |= VK_ACCESS_TRANSFER_WRITE_BIT;
    }
    if ( state.IsSet( ResourceState::VertexAndConstantBuffer ) )
    {
        result |= VK_ACCESS_UNIFORM_READ_BIT | VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT;
    }
    if ( state.IsSet( ResourceState::IndexBuffer ) )
    {
        result |= VK_ACCESS_INDEX_READ_BIT;
    }
    if ( state.IsSet( ResourceState::UnorderedAccess ) )
    {
        result |= VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT;
    }
    if ( state.IsSet( ResourceState::IndirectArgument ) )
    {
        result |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT;
    }
    if ( state.IsSet( ResourceState::RenderTarget ) )
    {
        result |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    }
    if ( state.IsSet( ResourceState::DepthWrite ) )
    {
        result |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }
    if ( state.IsSet( ResourceState::DepthRead ) )
    {
        result |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT;
    }
    if ( state.IsSet( ResourceState::ShaderResource ) )
    {
        result |= VK_ACCESS_SHADER_READ_BIT;
    }
    if ( state.IsSet( ResourceState::Present ) )
    {
        result |= VK_ACCESS_MEMORY_READ_BIT;
    }
    if ( state.IsSet( ResourceState::AccelerationStructureRead ) )
    {
        result |= VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
    }
    if ( state.IsSet( ResourceState::AccelerationStructureWrite ) )
    {
        result |= VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
    }

    return result;
}

VkImageLayout VulkanPipelineBarrierHelper::GetImageLayout( const BitSet<ResourceState> &state )
{
    if ( state.IsSet( ResourceState::CopySrc ) )
    {
        return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    }
    if ( state.IsSet( ResourceState::CopyDst ) )
    {
        return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    }
    if ( state.IsSet( ResourceState::RenderTarget ) )
    {
        return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    }
    if ( state.IsSet( ResourceState::DepthWrite ) )
    {
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }
    if ( state.IsSet( ResourceState::DepthRead ) )
    {
        return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
    }
    if ( state.IsSet( ResourceState::UnorderedAccess ) )
    {
        return VK_IMAGE_LAYOUT_GENERAL;
    }
    if ( state.IsSet( ResourceState::ShaderResource ) )
    {
        return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    }
    if ( state.IsSet( ResourceState::Present ) )
    {
        return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
    }
    if ( state.IsSet( ResourceState::Common ) )
    {
        return VK_IMAGE_LAYOUT_GENERAL;
    }

    return VK_IMAGE_LAYOUT_UNDEFINED;
}

VkPipelineStageFlags VulkanPipelineBarrierHelper::GetPipelineStageFlags( const VulkanContext *context, const QueueType queueType, const VkAccessFlags accessFlags )
{
    VkPipelineStageFlags flags = { };

    const auto capabilities = context->SelectedDeviceInfo.Capabilities;
    if ( capabilities.RayTracing )
    {
        if ( accessFlags & VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR )
        {
            flags |= VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR | VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
        }
        if ( accessFlags & VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR )
        {
            flags |= VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR;
        }
    }

    switch ( queueType )
    {
    case QueueType::Presentation:
    case QueueType::Graphics:
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
    case QueueType::Compute:
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

            break;
        }
    case QueueType::Copy:
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
        flags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
    }

    return flags;
}
