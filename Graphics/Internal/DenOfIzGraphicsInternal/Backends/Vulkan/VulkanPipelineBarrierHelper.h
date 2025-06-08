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

#include "VulkanContext.h"
#include "VulkanEnumConverter.h"

namespace DenOfIz
{

    class VulkanPipelineBarrierHelper
    {
    public:
        static void ExecutePipelineBarrier( const VulkanContext *context, const VkCommandBuffer &commandBuffer, const QueueType &commandQueueType,
                                            const PipelineBarrierDesc &barrier );

    private:
        static VkImageMemoryBarrier  CreateImageBarrier( const VulkanContext *context, const TextureBarrierDesc &barrier, VkAccessFlags &srcAccessFlags,
                                                         VkAccessFlags &dstAccessFlags, QueueType queueType );
        static VkBufferMemoryBarrier CreateBufferBarrier( const BufferBarrierDesc &barrier, VkAccessFlags &srcAccessFlags, VkAccessFlags &dstAccessFlags, QueueType queueType );
        static VkAccessFlags         GetAccessFlags( const uint32_t &state, const QueueType queueType );
        static VkImageLayout         GetImageLayout( const uint32_t &state );
        static VkPipelineStageFlags  GetPipelineStageFlags( const VulkanContext *context, QueueType queueType, VkAccessFlags accessFlags );
        static uint32_t              GetQueueFamilyIndex( const VulkanContext *context, QueueType queueType );
    };

} // namespace DenOfIz
