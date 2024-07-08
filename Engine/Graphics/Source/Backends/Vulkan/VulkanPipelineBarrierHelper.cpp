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

#include <DenOfIzGraphics/Backends/Vulkan/VulkanPipelineBarrierHelper.h>

using namespace DenOfIz;

void VulkanPipelineBarrierHelper::ExecutePipelineBarrier(VulkanContext *context, vk::CommandBuffer commandBuffer, const QueueType &commandQueueType, const PipelineBarrier &barrier)
{
    vk::AccessFlags srcAccessFlags = {};
    vk::AccessFlags dstAccessFlags = {};

    std::vector<vk::ImageMemoryBarrier> imageBarriers;
    for ( const TextureBarrierDesc &imageBarrier : barrier.GetTextureBarriers() )
    {
        vk::ImageMemoryBarrier imageMemoryBarrier = CreateImageBarrier(imageBarrier, srcAccessFlags, dstAccessFlags);
        imageBarriers.push_back(std::move(imageMemoryBarrier));
    }

    std::vector<vk::BufferMemoryBarrier> bufferBarriers;
    for ( const BufferBarrierDesc &bufferBarrier : barrier.GetBufferBarriers() )
    {
        vk::BufferMemoryBarrier bufferMemoryBarrier = CreateBufferBarrier(bufferBarrier, srcAccessFlags, dstAccessFlags);
        bufferBarriers.push_back(std::move(bufferMemoryBarrier));
    }
    std::vector<vk::MemoryBarrier> memoryBarriers;

    vk::PipelineStageFlags srcStageMask = GetPipelineStageFlags(context, commandQueueType, srcAccessFlags);
    vk::PipelineStageFlags dstStageMask = GetPipelineStageFlags(context, commandQueueType, dstAccessFlags);

    commandBuffer.pipelineBarrier(srcStageMask, dstStageMask, vk::DependencyFlags{}, memoryBarriers, bufferBarriers, imageBarriers);
}

vk::ImageMemoryBarrier VulkanPipelineBarrierHelper::CreateImageBarrier(const TextureBarrierDesc &barrier, vk::AccessFlags &srcAccessFlags, vk::AccessFlags &dstAccessFlags)
{
    VulkanTextureResource *imageResource = (VulkanTextureResource *)barrier.Resource;
    vk::ImageMemoryBarrier imageMemoryBarrier{};

    if ( barrier.OldState.IsSet(ResourceState::UnorderedAccess) && barrier.NewState.IsSet(ResourceState::UnorderedAccess) )
    {
        imageMemoryBarrier.srcAccessMask = vk::AccessFlagBits::eShaderWrite;
        imageMemoryBarrier.dstAccessMask = vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead;
        imageMemoryBarrier.oldLayout     = vk::ImageLayout::eGeneral;
        imageMemoryBarrier.newLayout     = vk::ImageLayout::eGeneral;
    }
    else
    {
        imageMemoryBarrier.srcAccessMask = GetAccessFlags(barrier.OldState);
        imageMemoryBarrier.dstAccessMask = GetAccessFlags(barrier.NewState);
        imageMemoryBarrier.oldLayout     = GetImageLayout(barrier.OldState);
        imageMemoryBarrier.newLayout     = GetImageLayout(barrier.NewState);
        assert(imageMemoryBarrier.newLayout != vk::ImageLayout::eUndefined);
    }

    imageMemoryBarrier.image                           = imageResource->GetImage();
    imageMemoryBarrier.subresourceRange.aspectMask     = imageResource->GetAspect();
    imageMemoryBarrier.subresourceRange.baseMipLevel   = barrier.EnableSubresourceBarrier ? barrier.MipLevel : 0;
    imageMemoryBarrier.subresourceRange.levelCount     = barrier.EnableSubresourceBarrier ? 1 : VK_REMAINING_MIP_LEVELS;
    imageMemoryBarrier.subresourceRange.baseArrayLayer = barrier.EnableSubresourceBarrier ? barrier.ArrayLayer : 0;
    imageMemoryBarrier.subresourceRange.layerCount     = barrier.EnableSubresourceBarrier ? 1 : VK_REMAINING_ARRAY_LAYERS;

    imageMemoryBarrier.srcQueueFamilyIndex = barrier.SourceQueue;
    if ( barrier.EnableQueueBarrier && !barrier.OldState.IsSet(ResourceState::Undefined) )
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

vk::BufferMemoryBarrier VulkanPipelineBarrierHelper::CreateBufferBarrier(const BufferBarrierDesc &barrier, vk::AccessFlags &srcAccessFlags, vk::AccessFlags &dstAccessFlags)
{
    VulkanBufferResource *bufferResource = (VulkanBufferResource *)barrier.Resource;
    vk::MemoryBarrier     memoryBarrier{};

    if ( barrier.OldState.IsSet(ResourceState::UnorderedAccess) && barrier.NewState.IsSet(ResourceState::UnorderedAccess) )
    {
        memoryBarrier.srcAccessMask |= vk::AccessFlagBits::eShaderWrite;
        memoryBarrier.dstAccessMask |= vk::AccessFlagBits::eShaderWrite | vk::AccessFlagBits::eShaderRead;
    }
    else
    {
        memoryBarrier.srcAccessMask |= GetAccessFlags(barrier.OldState);
        memoryBarrier.dstAccessMask |= GetAccessFlags(barrier.NewState);
    }

    srcAccessFlags |= memoryBarrier.srcAccessMask;
    dstAccessFlags |= memoryBarrier.dstAccessMask;
    return vk::BufferMemoryBarrier();
}

vk::AccessFlags VulkanPipelineBarrierHelper::GetAccessFlags(const BitSet<ResourceState> &state)
{
    vk::AccessFlags result;

    if ( state.IsSet(ResourceState::CopySrc) )
    {
        result |= vk::AccessFlagBits::eTransferRead;
    }
    if ( state.IsSet(ResourceState::CopyDst) )
    {
        result |= vk::AccessFlagBits::eTransferWrite;
    }
    if ( state.IsSet(ResourceState::VertexAndConstantBuffer) )
    {
        result |= vk::AccessFlagBits::eUniformRead | vk::AccessFlagBits::eVertexAttributeRead;
    }
    if ( state.IsSet(ResourceState::IndexBuffer) )
    {
        result |= vk::AccessFlagBits::eIndexRead;
    }
    if ( state.IsSet(ResourceState::UnorderedAccess) )
    {
        result |= vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
    }
    if ( state.IsSet(ResourceState::IndirectArgument) )
    {
        result |= vk::AccessFlagBits::eIndirectCommandRead;
    }
    if ( state.IsSet(ResourceState::RenderTarget) )
    {
        result |= vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    }
    if ( state.IsSet(ResourceState::DepthWrite) )
    {
        result |= vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    }
    if ( state.IsSet(ResourceState::DepthRead) )
    {
        result |= vk::AccessFlagBits::eDepthStencilAttachmentRead;
    }
    if ( state.IsSet(ResourceState::ShaderResource) )
    {
        result |= vk::AccessFlagBits::eShaderRead;
    }
    if ( state.IsSet(ResourceState::Present) )
    {
        result |= vk::AccessFlagBits::eMemoryRead;
    }
    if ( state.IsSet(ResourceState::AccelerationStructureRead) )
    {
        result |= vk::AccessFlagBits::eAccelerationStructureReadKHR;
    }
    if ( state.IsSet(ResourceState::AccelerationStructureWrite) )
    {
        result |= vk::AccessFlagBits::eAccelerationStructureWriteKHR;
    }

    return result;
}

vk::ImageLayout VulkanPipelineBarrierHelper::GetImageLayout(const BitSet<ResourceState> &state)
{
    if ( state.IsSet(ResourceState::CopySrc) )
    {
        return vk::ImageLayout::eTransferSrcOptimal;
    }
    if ( state.IsSet(ResourceState::CopyDst) )
    {
        return vk::ImageLayout::eTransferDstOptimal;
    }
    if ( state.IsSet(ResourceState::RenderTarget) )
    {
        return vk::ImageLayout::eColorAttachmentOptimal;
    }
    if ( state.IsSet(ResourceState::DepthWrite) )
    {
        return vk::ImageLayout::eDepthStencilAttachmentOptimal;
    }
    if ( state.IsSet(ResourceState::DepthRead) )
    {
        return vk::ImageLayout::eDepthStencilReadOnlyOptimal;
    }
    if ( state.IsSet(ResourceState::UnorderedAccess) )
    {
        return vk::ImageLayout::eGeneral;
    }
    if ( state.IsSet(ResourceState::ShaderResource) )
    {
        return vk::ImageLayout::eShaderReadOnlyOptimal;
    }
    if ( state.IsSet(ResourceState::Present) )
    {
        return vk::ImageLayout::ePresentSrcKHR;
    }
    if ( state.IsSet(ResourceState::Common) )
    {
        return vk::ImageLayout::eGeneral;
    }

    return vk::ImageLayout::eUndefined;
}

vk::PipelineStageFlags VulkanPipelineBarrierHelper::GetPipelineStageFlags(VulkanContext *context, QueueType queueType, vk::AccessFlags accessFlags)
{
    vk::PipelineStageFlags flags = {};

    auto capabilities = context->SelectedDeviceInfo.Capabilities;

    if ( capabilities.RayTracing )
    {
        if ( accessFlags & vk::AccessFlagBits::eAccelerationStructureReadKHR )
        {
            flags |= vk::PipelineStageFlagBits::eRayTracingShaderKHR | vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR;
        }
        if ( accessFlags & vk::AccessFlagBits::eAccelerationStructureWriteKHR )
        {
            flags |= vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR;
        }
    }

    switch ( queueType )
    {
    case QueueType::Presentation:
    case QueueType::Graphics:
        {
            if ( (accessFlags & (vk::AccessFlagBits::eIndexRead | vk::AccessFlagBits::eVertexAttributeRead)) )
            {
                flags |= vk::PipelineStageFlagBits::eVertexInput;
            }
            if ( (accessFlags & (vk::AccessFlagBits::eUniformRead | vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite)) )
            {
                flags |= vk::PipelineStageFlagBits::eVertexShader;
                flags |= vk::PipelineStageFlagBits::eFragmentShader;
                if ( capabilities.GeometryShaders )
                {
                    flags |= vk::PipelineStageFlagBits::eGeometryShader;
                }
                if ( capabilities.Tessellation )
                {
                    flags |= vk::PipelineStageFlagBits::eTessellationControlShader;
                    flags |= vk::PipelineStageFlagBits::eTessellationEvaluationShader;
                }
                flags |= vk::PipelineStageFlagBits::eComputeShader;

                if ( capabilities.RayTracing )
                {
                    flags |= vk::PipelineStageFlagBits::eRayTracingShaderKHR;
                }
            }

            if ( (accessFlags & vk::AccessFlagBits::eInputAttachmentRead) )
                flags |= vk::PipelineStageFlagBits::eFragmentShader;

            if ( (accessFlags & (vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)) )
                flags |= vk::PipelineStageFlagBits::eColorAttachmentOutput;

            if ( (accessFlags & (vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite)) )
            {
                flags |= vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
            }
            break;
        }
    case QueueType::Compute:
        {
            if ( (accessFlags & (vk::AccessFlagBits::eIndexRead | vk::AccessFlagBits::eVertexAttributeRead)) || (accessFlags & vk::AccessFlagBits::eInputAttachmentRead) ||
                 (accessFlags & (vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)) ||
                 (accessFlags & (vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite)) )
            {
                return vk::PipelineStageFlagBits::eAllCommands;
            }

            if ( (accessFlags & (vk::AccessFlagBits::eUniformRead | vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite)) )
            {
                flags |= vk::PipelineStageFlagBits::eComputeShader;
            }

            break;
        }
    case QueueType::Copy:
        return vk::PipelineStageFlagBits::eAllCommands;
    }

    if ( (accessFlags & vk::AccessFlagBits::eIndirectCommandRead) )
    {
        flags |= vk::PipelineStageFlagBits::eDrawIndirect;
    }

    if ( (accessFlags & (vk::AccessFlagBits::eTransferRead | vk::AccessFlagBits::eTransferWrite)) )
    {
        flags |= vk::PipelineStageFlagBits::eTransfer;
    }

    if ( (accessFlags & (vk::AccessFlagBits::eHostRead | vk::AccessFlagBits::eHostWrite)) )
    {
        flags |= vk::PipelineStageFlagBits::eHost;
    }

    if ( !flags )
    {
        flags = vk::PipelineStageFlagBits::eTopOfPipe;
    }

    return flags;
}
