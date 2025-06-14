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
        ITextureResource *Resource;
        uint32_t          OldState{ };
        uint32_t          NewState{ };

        bool      EnableQueueBarrier = false;
        QueueType SourceQueue;
        QueueType DestinationQueue;

        bool     EnableSubresourceBarrier = false;
        uint32_t MipLevel                 = 0;
        uint32_t ArrayLayer               = 0;
    };

    struct DZ_API TextureBarrierDescArray
    {
        TextureBarrierDesc const *Elements;
        size_t                    NumElements;
    };

    struct DZ_API BufferBarrierDesc
    {
        IBufferResource const *Resource;
        uint32_t               OldState;
        uint32_t               NewState;
    };

    struct DZ_API BufferBarrierDescArray
    {
        BufferBarrierDesc const *Elements;
        size_t                   NumElements;
    };

    struct DZ_API MemoryBarrierDesc
    {
        IBottomLevelAS          *BottomLevelAS   = nullptr;
        ITopLevelAS             *TopLevelAS      = nullptr;
        IBufferResource         *BufferResource  = nullptr;
        ITextureResource        *TextureResource = nullptr;
        uint32_t                 OldState;
        uint32_t                 NewState;
        static MemoryBarrierDesc Uav( )
        {
            MemoryBarrierDesc barrier{ };
            barrier.OldState = ResourceUsage::UnorderedAccess;
            barrier.NewState = ResourceUsage::UnorderedAccess;
            return barrier;
        }
    };

    struct DZ_API MemoryBarrierDescArray
    {
        MemoryBarrierDesc const *Elements;
        size_t                   NumElements;
    };

    class DZ_API PipelineBarrierDesc
    {
        std::vector<TextureBarrierDesc> m_textureBarriers;
        std::vector<BufferBarrierDesc>  m_bufferBarriers;
        std::vector<MemoryBarrierDesc>  m_memoryBarriers;

    public:
        void Clear( )
        {
            m_textureBarriers.clear( );
            m_bufferBarriers.clear( );
            m_memoryBarriers.clear( );
        }

        PipelineBarrierDesc &TextureBarrier( const TextureBarrierDesc &barrier )
        {
            m_textureBarriers.push_back( barrier );
            return *this;
        }

        PipelineBarrierDesc &BufferBarrier( const BufferBarrierDesc &barrier )
        {
            m_bufferBarriers.push_back( barrier );
            return *this;
        }

        PipelineBarrierDesc &MemoryBarrier( const MemoryBarrierDesc &barrier )
        {
            m_memoryBarriers.push_back( barrier );
            return *this;
        }

        [[nodiscard]] TextureBarrierDescArray GetTextureBarriers( ) const
        {
            return TextureBarrierDescArray{ m_textureBarriers.data( ), m_textureBarriers.size( ) };
        }

        [[nodiscard]] BufferBarrierDescArray GetBufferBarriers( ) const
        {
            return BufferBarrierDescArray{ m_bufferBarriers.data( ), m_bufferBarriers.size( ) };
        }

        [[nodiscard]] MemoryBarrierDescArray GetMemoryBarriers( ) const
        {
            return MemoryBarrierDescArray{ m_memoryBarriers.data( ), m_memoryBarriers.size( ) };
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
