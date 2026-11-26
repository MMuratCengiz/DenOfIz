/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "VulkanContext.h"
#include "VulkanEnumConverter.h"

namespace DenOfIz
{

    class VulkanPipelineBarrierHelper
    {
    public:
        static void ExecutePipelineBarrier( const VulkanContext *context, const VkCommandBuffer &commandBuffer, const DenOfIz_QueueType &commandQueueType,
                                            const DenOfIz_PipelineBarrierDesc *barrier );

    private:
        static VkImageMemoryBarrier  CreateImageBarrier( const VulkanContext *context, const DenOfIz_TextureBarrierDesc &barrier, VkAccessFlags &srcAccessFlags,
                                                         VkAccessFlags &dstAccessFlags, DenOfIz_QueueType queueType );
        static VkBufferMemoryBarrier CreateBufferBarrier( const DenOfIz_BufferBarrierDesc &barrier, VkAccessFlags &srcAccessFlags, VkAccessFlags &dstAccessFlags,
                                                          DenOfIz_QueueType queueType );
        static VkAccessFlags         GetAccessFlags( const uint32_t &state, const DenOfIz_QueueType queueType );
        static VkImageLayout         GetImageLayout( const uint32_t &state );
        static VkPipelineStageFlags  GetPipelineStageFlags( const VulkanContext *context, DenOfIz_QueueType queueType, VkAccessFlags accessFlags );
        static uint32_t              GetQueueFamilyIndex( const VulkanContext *context, DenOfIz_QueueType queueType );
    };

} // namespace DenOfIz
