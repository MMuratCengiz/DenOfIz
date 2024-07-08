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

#include "IBufferResource.h"
#include "ITextureResource.h"

#undef MemoryBarrier

namespace DenOfIz
{

    struct TextureBarrierInfo
    {
        ITextureResource     *Resource;
        BitSet<ResourceState> OldState{};
        BitSet<ResourceState> NewState{};

        bool     EnableQueueBarrier = false;
        uint32_t SourceQueue;
        uint32_t DestinationQueue;

        bool     EnableSubresourceBarrier = false;
        uint32_t MipLevel                 = 0;
        uint32_t ArrayLayer               = 0;
    };

    struct BufferBarrierInfo
    {
        IBufferResource      *Resource;
        BitSet<ResourceState> OldState;
        BitSet<ResourceState> NewState;
    };

    struct MemoryBarrierInfo
    {
        // todo
        BitSet<ResourceState> OldState;
        BitSet<ResourceState> NewState;
    };

    class PipelineBarrier
    {
    private:
        std::vector<TextureBarrierInfo> m_textureBarriers;
        std::vector<BufferBarrierInfo>  m_bufferBarriers;
        std::vector<MemoryBarrierInfo>  m_memoryBarriers;

    public:
        inline void TextureBarrier(TextureBarrierInfo barrier)
        {
            m_textureBarriers.push_back(barrier);
        }

        inline void BufferBarrier(BufferBarrierInfo barrier)
        {
            m_bufferBarriers.push_back(barrier);
        }

        inline void MemoryBarrier(MemoryBarrierInfo barrier)
        {
            m_memoryBarriers.push_back(barrier);
        }

        inline const std::vector<TextureBarrierInfo> &GetTextureBarriers() const
        {
            return m_textureBarriers;
        }

        inline const std::vector<BufferBarrierInfo> &GetBufferBarriers() const
        {
            return m_bufferBarriers;
        }

        inline const std::vector<MemoryBarrierInfo> &GetMemoryBarriers() const
        {
            return m_memoryBarriers;
        }

        static PipelineBarrier UndefinedToRenderTarget(ITextureResource *resource)
        {
            PipelineBarrier    barrier;
            TextureBarrierInfo textureBarrier{};
            textureBarrier.OldState = ResourceState::Undefined;
            textureBarrier.NewState = ResourceState::RenderTarget;
            textureBarrier.Resource = resource;
            barrier.TextureBarrier(textureBarrier);
            return barrier;
        }

        static PipelineBarrier RenderTargetToPresent(ITextureResource *resource)
        {
            PipelineBarrier    barrier;
            TextureBarrierInfo textureBarrier{};
            textureBarrier.OldState = ResourceState::RenderTarget;
            textureBarrier.NewState = ResourceState::Present;
            textureBarrier.Resource = resource;
            barrier.TextureBarrier(textureBarrier);
            return barrier;
        }
    };

} // namespace DenOfIz
