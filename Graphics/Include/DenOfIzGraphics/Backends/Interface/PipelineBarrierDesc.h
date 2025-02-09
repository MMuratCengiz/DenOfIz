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

    struct DZ_API TextureBarrierDesc
    {
        ITextureResource     *Resource;
        BitSet<ResourceUsage> OldState{ };
        BitSet<ResourceUsage> NewState{ };

        bool      EnableQueueBarrier = false;
        QueueType SourceQueue;
        QueueType DestinationQueue;

        bool     EnableSubresourceBarrier = false;
        uint32_t MipLevel                 = 0;
        uint32_t ArrayLayer               = 0;
    };
    template class DZ_API InteropArray<TextureBarrierDesc>;

    struct DZ_API BufferBarrierDesc
    {
        IBufferResource      *Resource;
        BitSet<ResourceUsage> OldState;
        BitSet<ResourceUsage> NewState;
    };
    template class DZ_API InteropArray<BufferBarrierDesc>;

    struct DZ_API MemoryBarrierDesc
    {
        IBottomLevelAS          *BottomLevelAS   = nullptr;
        ITopLevelAS             *TopLevelAS      = nullptr;
        IBufferResource         *BufferResource  = nullptr;
        ITextureResource        *TextureResource = nullptr;
        BitSet<ResourceUsage>    OldState;
        BitSet<ResourceUsage>    NewState;
        static MemoryBarrierDesc Uav( )
        {
            MemoryBarrierDesc barrier{ };
            barrier.OldState.Set( ResourceUsage::UnorderedAccess );
            barrier.NewState.Set( ResourceUsage::UnorderedAccess );
            return barrier;
        }
    };
    template class DZ_API InteropArray<MemoryBarrierDesc>;

    class DZ_API PipelineBarrierDesc
    {
        InteropArray<TextureBarrierDesc> m_textureBarriers;
        InteropArray<BufferBarrierDesc>  m_bufferBarriers;
        InteropArray<MemoryBarrierDesc>  m_memoryBarriers;

    public:
        PipelineBarrierDesc &TextureBarrier( const TextureBarrierDesc &barrier )
        {
            m_textureBarriers.AddElement( barrier );
            return *this;
        }

        PipelineBarrierDesc &BufferBarrier( const BufferBarrierDesc barrier )
        {
            m_bufferBarriers.AddElement( barrier );
            return *this;
        }

        PipelineBarrierDesc &MemoryBarrier( const MemoryBarrierDesc barrier )
        {
            m_memoryBarriers.AddElement( barrier );
            return *this;
        }

        [[nodiscard]] const InteropArray<TextureBarrierDesc> &GetTextureBarriers( ) const
        {
            return m_textureBarriers;
        }

        [[nodiscard]] const InteropArray<BufferBarrierDesc> &GetBufferBarriers( ) const
        {
            return m_bufferBarriers;
        }

        [[nodiscard]] const InteropArray<MemoryBarrierDesc> &GetMemoryBarriers( ) const
        {
            return m_memoryBarriers;
        }

        static PipelineBarrierDesc Uav( )
        {
            PipelineBarrierDesc barrier;
            barrier.MemoryBarrier( MemoryBarrierDesc::Uav( ) );
            return barrier;
        }

        static PipelineBarrierDesc UndefinedToRenderTarget( ITextureResource *resource )
        {
            PipelineBarrierDesc barrier;
            TextureBarrierDesc  textureBarrier{ };
            textureBarrier.OldState = ResourceUsage::Undefined;
            textureBarrier.NewState = ResourceUsage::RenderTarget;
            textureBarrier.Resource = resource;
            barrier.TextureBarrier( textureBarrier );
            return barrier;
        }

        static PipelineBarrierDesc RenderTargetToPresent( ITextureResource *resource )
        {
            PipelineBarrierDesc barrier;
            TextureBarrierDesc  textureBarrier{ };
            textureBarrier.OldState = ResourceUsage::RenderTarget;
            textureBarrier.NewState = ResourceUsage::Present;
            textureBarrier.Resource = resource;
            barrier.TextureBarrier( textureBarrier );
            return barrier;
        }

        static PipelineBarrierDesc RenderTargetToShaderResource( ITextureResource *resource )
        {
            PipelineBarrierDesc barrier;
            TextureBarrierDesc  textureBarrier{ };
            textureBarrier.OldState = ResourceUsage::RenderTarget;
            textureBarrier.NewState = ResourceUsage::PixelShaderResource;
            textureBarrier.Resource = resource;
            barrier.TextureBarrier( textureBarrier );
            return barrier;
        }
    };

} // namespace DenOfIz
