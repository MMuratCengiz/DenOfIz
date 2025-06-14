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

#include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>
#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
#include <DenOfIzGraphics/Backends/Interface/ShaderData.h>
#include "VulkanContext.h"
#include "VulkanTextureResource.h"

namespace DenOfIz
{

    class VulkanEnumConverter
    {
    public:
        static VkQueueFlags                         ConvertQueueFlags( const QueueType &queueType );
        static VkShaderStageFlagBits                ConvertShaderStage( const ShaderStage &shaderStage );
        static VkSampleCountFlagBits                ConvertSampleCount( const MSAASampleCount &sampleCount );
        static VkStencilOp                          ConvertStencilOp( const StencilOp &stencilOp );
        static VkCompareOp                          ConvertCompareOp( const CompareOp &compareOp );
        static VkAttachmentLoadOp                   ConvertLoadOp( const LoadOp &loadOp );
        static VkAttachmentStoreOp                  ConvertStoreOp( const StoreOp &storeOp );
        static VkBlendOp                            ConvertBlendOp( const BlendOp &op );
        static VkLogicOp                            ConvertLogicOp( const LogicOp &op );
        static VkBlendFactor                        ConvertBlend( const Blend &blend );
        static VkFilter                             ConvertFilter( const Filter &filter );
        static VkSamplerAddressMode                 ConvertAddressMode( const SamplerAddressMode &addressMode );
        static VkSamplerMipmapMode                  ConvertMipmapMode( const MipmapMode &mipmapMode );
        static VkBufferUsageFlags                   ConvertBufferUsage( const uint32_t& descriptor, const uint32_t& usages );
        static VkImageAspectFlagBits                ConvertImageAspect( TextureAspect aspect );
        static VkImageUsageFlags                    ConvertTextureDescriptorToUsage( const uint32_t& descriptor, const uint32_t& initialState );
        static VmaMemoryUsage                       ConvertHeapType( HeapType location );
        static VkFormat                             ConvertImageFormat( const Format &imageFormat );
        static VkDescriptorType                     ConvertResourceDescriptorToDescriptorType( const uint32_t &descriptor );
        static VkPrimitiveTopology                  ConvertPrimitiveTopology( const PrimitiveTopology &topology );
        static VkPipelineBindPoint                  ConvertPipelineBindPoint( const BindPoint &point );
        static VkImageUsageFlags                    ConvertTextureUsage( uint32_t descriptor, uint32_t usage );
        static VkImageLayout                        ConvertTextureDescriptorToLayout( uint32_t initialState );
        static VkBuildAccelerationStructureFlagsKHR ConvertAccelerationStructureBuildFlags( uint32_t buildFlags );
    };

} // namespace DenOfIz
