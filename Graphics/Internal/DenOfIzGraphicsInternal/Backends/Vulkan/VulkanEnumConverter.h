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

#pragma once

#include <DenOfIzGraphics/Backends/Interface/ShaderData.h>
#include <DenOfIzGraphicsInternal/Backends/Interface/IBuffer.h>
#include <DenOfIzGraphicsInternal/Backends/Interface/IPipeline.h>
#include "VulkanContext.h"
#include "VulkanTexture.h"

VkQueueFlags          DenOfIz_VulkanEnumConverter_ConvertQueueFlags( DenOfIz_QueueType queueType );
VkShaderStageFlags    DenOfIz_VulkanEnumConverter_ConvertShaderStage( DenOfIz_ShaderStageFlags shaderStages );
VkSampleCountFlagBits DenOfIz_VulkanEnumConverter_ConvertSampleCount( DenOfIz_MSAASampleCount sampleCount );
VkStencilOp           DenOfIz_VulkanEnumConverter_ConvertStencilOp( DenOfIz_StencilOp stencilOp );
VkCompareOp           DenOfIz_VulkanEnumConverter_ConvertCompareOp( DenOfIz_CompareOp compareOp );
VkAttachmentLoadOp    DenOfIz_VulkanEnumConverter_ConvertLoadOp( DenOfIz_LoadOp loadOp );
VkAttachmentStoreOp   DenOfIz_VulkanEnumConverter_ConvertStoreOp( DenOfIz_StoreOp storeOp );
VkBlendOp             DenOfIz_VulkanEnumConverter_ConvertBlendOp( DenOfIz_BlendOp op );
VkLogicOp             DenOfIz_VulkanEnumConverter_ConvertLogicOp( DenOfIz_LogicOp op );
VkBlendFactor         DenOfIz_VulkanEnumConverter_ConvertBlend( DenOfIz_Blend blend );
VkFilter              DenOfIz_VulkanEnumConverter_ConvertFilter( DenOfIz_Filter filter );
VkSamplerAddressMode  DenOfIz_VulkanEnumConverter_ConvertAddressMode( DenOfIz_SamplerAddressMode addressMode );
VkSamplerMipmapMode   DenOfIz_VulkanEnumConverter_ConvertMipmapMode( DenOfIz_MipmapMode mipmapMode );
VkBufferUsageFlags    DenOfIz_VulkanEnumConverter_ConvertBufferUsage( DenOfIz_ResourceUsageFlags descriptor, DenOfIz_ResourceUsageFlags usages );
VkImageAspectFlagBits DenOfIz_VulkanEnumConverter_ConvertImageAspect( DenOfIz_TextureAspect aspect );
VkImageUsageFlags     DenOfIz_VulkanEnumConverter_ConvertTextureDescriptorToUsage( DenOfIz_ResourceDescriptorFlags descriptor, DenOfIz_ResourceUsageFlags initialState );
VmaMemoryUsage        DenOfIz_VulkanEnumConverter_ConvertHeapType( DenOfIz_HeapType location );
VkFormat              DenOfIz_VulkanEnumConverter_ConvertImageFormat( DenOfIz_Format imageFormat );
VkDescriptorType      DenOfIz_VulkanEnumConverter_ConvertResourceDescriptorToDescriptorType( DenOfIz_ResourceDescriptorFlags descriptor );
VkPrimitiveTopology   DenOfIz_VulkanEnumConverter_ConvertPrimitiveTopology( DenOfIz_PrimitiveTopology topology );
VkPipelineBindPoint   DenOfIz_VulkanEnumConverter_ConvertPipelineBindPoint( DenOfIz_BindPoint point );
VkImageUsageFlags     DenOfIz_VulkanEnumConverter_ConvertTextureUsage( DenOfIz_ResourceDescriptorFlags descriptor, DenOfIz_ResourceUsageFlags usageFlags );
VkImageLayout         DenOfIz_VulkanEnumConverter_ConvertTextureDescriptorToLayout( DenOfIz_ResourceUsageFlags initialState );
VkBuildAccelerationStructureFlagsKHR DenOfIz_VulkanEnumConverter_ConvertAccelerationStructureBuildFlags( DenOfIz_ASBuildFlags buildFlags );
VkPipelineStageFlagBits              DenOfIz_VulkanEnumConverter_ConvertPipelineStageFlags( DenOfIz_PipelineStageFlags pipelineStage );
VkQueryType                          DenOfIz_VulkanEnumConverter_ConvertQueryType( DenOfIz_QueryType queryType );

VkBufferUsageFlags DenOfIz_VulkanEnumConverter_ConvertBufferUsageToVk( DenOfIz_BufferUsageFlags usage );
VkImageUsageFlags  DenOfIz_VulkanEnumConverter_ConvertTextureUsageToVk( DenOfIz_TextureUsageFlags usage, DenOfIz_Format format );
VkImageAspectFlags DenOfIz_VulkanEnumConverter_GetImageAspectFromFormat( DenOfIz_Format format );
