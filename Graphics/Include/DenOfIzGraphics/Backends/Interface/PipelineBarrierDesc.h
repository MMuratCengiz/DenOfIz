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
        BitSet<ResourceState> OldState{ };
        BitSet<ResourceState> NewState{ };

        bool     EnableQueueBarrier = false;
        uint32_t SourceQueue;
        uint32_t DestinationQueue;

        bool     EnableSubresourceBarrier = false;
        uint32_t MipLevel                 = 0;
        uint32_t ArrayLayer               = 0;
    };

    struct DZ_API BufferBarrierDesc
    {
        IBufferResource      *Resource;
        BitSet<ResourceState> OldState;
        BitSet<ResourceState> NewState;
    };

    struct DZ_API MemoryBarrierDesc
    {
        // todo
        BitSet<ResourceState> OldState;
        BitSet<ResourceState> NewState;
    };

#define DZ_MAX_BARRIERS 16
    struct DZ_API BufferBarriers
    {
        size_t            NumElements = 0;
        BufferBarrierDesc Array[ DZ_MAX_BARRIERS ];

        void SetElement( size_t index, const BufferBarrierDesc &value )
        {
            Array[ index ] = value;
        }
        const BufferBarrierDesc &GetElement( size_t index )
        {
            return Array[ index ];
        }
    };

    struct DZ_API TextureBarriers
    {
        size_t             NumElements = 0;
        TextureBarrierDesc Array[ DZ_MAX_BARRIERS ];

        void SetElement( size_t index, const TextureBarrierDesc &value )
        {
            Array[ index ] = value;
        }
        const TextureBarrierDesc &GetElement( size_t index )
        {
            return Array[ index ];
        }
    };

    struct DZ_API MemoryBarriers
    {
        size_t            NumElements = 0;
        MemoryBarrierDesc Array[ DZ_MAX_BARRIERS ];

        void SetElement( size_t index, const MemoryBarrierDesc &value )
        {
            Array[ index ] = value;
        }
        const MemoryBarrierDesc &GetElement( size_t index )
        {
            return Array[ index ];
        }
    };

    class DZ_API PipelineBarrierDesc
    {
        TextureBarriers m_textureBarriers;
        BufferBarriers  m_bufferBarriers;
        MemoryBarriers  m_memoryBarriers;

    public:
        PipelineBarrierDesc &TextureBarrier( const TextureBarrierDesc &barrier )
        {
            m_textureBarriers.Array[ m_textureBarriers.NumElements++ ] = barrier;
            return *this;
        }

        PipelineBarrierDesc &BufferBarrier( const BufferBarrierDesc barrier )
        {
            m_bufferBarriers.Array[ m_bufferBarriers.NumElements++ ] = barrier;
            return *this;
        }

        PipelineBarrierDesc &MemoryBarrier( const MemoryBarrierDesc barrier )
        {
            m_memoryBarriers.Array[ m_memoryBarriers.NumElements++ ] = barrier;
            return *this;
        }

        [[nodiscard]] const TextureBarriers &GetTextureBarriers( ) const
        {
            return m_textureBarriers;
        }

        [[nodiscard]] const BufferBarriers &GetBufferBarriers( ) const
        {
            return m_bufferBarriers;
        }

        [[nodiscard]] const MemoryBarriers &GetMemoryBarriers( ) const
        {
            return m_memoryBarriers;
        }

        static PipelineBarrierDesc UndefinedToRenderTarget( ITextureResource *resource )
        {
            PipelineBarrierDesc barrier;
            TextureBarrierDesc  textureBarrier{ };
            textureBarrier.OldState = ResourceState::Undefined;
            textureBarrier.NewState = ResourceState::RenderTarget;
            textureBarrier.Resource = resource;
            barrier.TextureBarrier( textureBarrier );
            return barrier;
        }

        static PipelineBarrierDesc RenderTargetToPresent( ITextureResource *resource )
        {
            PipelineBarrierDesc barrier;
            TextureBarrierDesc  textureBarrier{ };
            textureBarrier.OldState = ResourceState::RenderTarget;
            textureBarrier.NewState = ResourceState::Present;
            textureBarrier.Resource = resource;
            barrier.TextureBarrier( textureBarrier );
            return barrier;
        }

        static PipelineBarrierDesc RenderTargetToShaderResource( ITextureResource *resource )
        {
            PipelineBarrierDesc barrier;
            TextureBarrierDesc  textureBarrier{ };
            textureBarrier.OldState = ResourceState::RenderTarget;
            textureBarrier.NewState = ResourceState::PixelShaderResource;
            textureBarrier.Resource = resource;
            barrier.TextureBarrier( textureBarrier );
            return barrier;
        }
    };

} // namespace DenOfIz
