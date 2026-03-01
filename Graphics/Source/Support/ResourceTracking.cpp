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

#include "DenOfIzGraphics/Support/ResourceTracking.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/ITexture.h"

#include <mutex>
#include <unordered_map>
#include <vector>

namespace DenOfIz
{
    class ResourceTracking
    {
        struct ResourceState
        {
            std::mutex                 Mutex;
            DenOfIz_ResourceUsageFlags CurrentUsage = DENOFIZ_RESOURCE_USAGE_UNDEFINED_BIT;
            DenOfIz_QueueType          CurrentQueue = DENOFIZ_QUEUE_TYPE_GRAPHICS;
            uint32_t                   MipLevels    = 1;
        };

        std::unordered_map<DenOfIz_Buffer, ResourceState>  m_bufferStates;
        std::unordered_map<DenOfIz_Texture, ResourceState> m_textureStates;

        struct TextureSubresourceKey
        {
            DenOfIz_Texture Texture;
            uint32_t        MipLevel;

            bool operator==( const TextureSubresourceKey &other ) const
            {
                return Texture == other.Texture && MipLevel == other.MipLevel;
            }
        };

        struct TextureSubresourceKeyHash
        {
            size_t operator( )( const TextureSubresourceKey &key ) const
            {
                return std::hash<uint64_t>{ }( key.Texture ) ^ ( std::hash<uint32_t>{ }( key.MipLevel ) << 16 );
            }
        };

        std::unordered_map<TextureSubresourceKey, ResourceState, TextureSubresourceKeyHash> m_textureMipStates;

    public:
        ResourceTracking( )  = default;
        ~ResourceTracking( ) = default;

        void TrackBuffer( DenOfIz_Buffer buffer, DenOfIz_QueueType queueType );
        void TrackTexture( DenOfIz_Texture texture, DenOfIz_QueueType queueType );
        void UntrackBuffer( DenOfIz_Buffer buffer );
        void UntrackTexture( DenOfIz_Texture texture );
        void TransitionBuffer( DenOfIz_CommandList commandList, DenOfIz_Buffer buffer, uint32_t newUsage, DenOfIz_QueueType queueType );
        void TransitionTexture( DenOfIz_CommandList commandList, DenOfIz_Texture texture, uint32_t newUsage, DenOfIz_QueueType queueType );
        void TransitionTextureMip( DenOfIz_CommandList commandList, DenOfIz_Texture texture, uint32_t mipLevel, uint32_t newUsage, DenOfIz_QueueType queueType );
        void BatchTransition( DenOfIz_CommandList commandList, const DenOfIz_BatchTransitionDesc *desc );
        void NotifyTextureTransition( DenOfIz_Texture texture, uint32_t newUsage );
        void NotifyTextureTransitionMip( DenOfIz_Texture texture, uint32_t mipLevel, uint32_t newUsage );
        void NotifyBufferTransition( DenOfIz_Buffer buffer, uint32_t newUsage );

    private:
        void ProcessBufferTransitions( const DenOfIz_TransitionBufferDesc *buffers, size_t numBuffers, std::vector<DenOfIz_BufferBarrierDesc> &bufferBarriers );
        void ProcessTextureTransitions( const DenOfIz_TransitionTextureDesc *textures, size_t numTextures, std::vector<DenOfIz_TextureBarrierDesc> &textureBarriers );
    };
} // namespace DenOfIz

using namespace DenOfIz;

void ResourceTracking::TrackBuffer( DenOfIz_Buffer buffer, DenOfIz_QueueType queueType )
{
    m_bufferStates[ buffer ].CurrentQueue = queueType;
    m_bufferStates[ buffer ].CurrentUsage = DENOFIZ_RESOURCE_USAGE_COMMON_BIT;
}

void ResourceTracking::TrackTexture( DenOfIz_Texture texture, DenOfIz_QueueType queueType )
{
    const uint32_t numMipLevels = DENOFIZ_FROM_HANDLE( ITexture, texture )->GetMipLevels( );

    m_textureStates[ texture ].CurrentQueue = queueType;
    m_textureStates[ texture ].CurrentUsage = DENOFIZ_RESOURCE_USAGE_UNDEFINED_BIT;
    m_textureStates[ texture ].MipLevels   = numMipLevels;

    for ( uint32_t mip = 0; mip < numMipLevels; ++mip )
    {
        TextureSubresourceKey key{ texture, mip };
        m_textureMipStates[ key ].CurrentQueue = queueType;
        m_textureMipStates[ key ].CurrentUsage = DENOFIZ_RESOURCE_USAGE_UNDEFINED_BIT;
    }
}

void ResourceTracking::UntrackBuffer( DenOfIz_Buffer buffer )
{
    m_bufferStates.erase( buffer );
}

void ResourceTracking::UntrackTexture( DenOfIz_Texture texture )
{
    auto it = m_textureStates.find( texture );
    if ( it != m_textureStates.end( ) )
    {
        for ( uint32_t mip = 0; mip < it->second.MipLevels; ++mip )
        {
            m_textureMipStates.erase( TextureSubresourceKey{ texture, mip } );
        }
        m_textureStates.erase( it );
    }
}

void ResourceTracking::NotifyTextureTransition( DenOfIz_Texture texture, uint32_t newUsage )
{
    auto it = m_textureStates.find( texture );
    if ( it == m_textureStates.end( ) )
    {
        return;
    }

    std::unique_lock lock( it->second.Mutex );
    it->second.CurrentUsage = newUsage;

    for ( uint32_t mip = 0; mip < it->second.MipLevels; ++mip )
    {
        auto mipIt = m_textureMipStates.find( TextureSubresourceKey{ texture, mip } );
        if ( mipIt != m_textureMipStates.end( ) )
        {
            mipIt->second.CurrentUsage = newUsage;
        }
    }
}

void ResourceTracking::NotifyBufferTransition( DenOfIz_Buffer buffer, uint32_t newUsage )
{
    auto it = m_bufferStates.find( buffer );
    if ( it == m_bufferStates.end( ) )
    {
        return;
    }

    std::unique_lock lock( it->second.Mutex );
    it->second.CurrentUsage = newUsage;
}

void ResourceTracking::TransitionBuffer( DenOfIz_CommandList commandList, DenOfIz_Buffer buffer, uint32_t newUsage, DenOfIz_QueueType queueType )
{
    DenOfIz_TransitionBufferDesc transition{ };
    transition.Buffer    = buffer;
    transition.NewUsage  = newUsage;
    transition.QueueType = queueType;

    DenOfIz_BatchTransitionDesc batchTransition{ };
    batchTransition.Buffers.Elements    = &transition;
    batchTransition.Buffers.NumElements = 1;

    BatchTransition( commandList, &batchTransition );
}

void ResourceTracking::TransitionTexture( DenOfIz_CommandList commandList, DenOfIz_Texture texture, uint32_t newUsage, DenOfIz_QueueType queueType )
{
    DenOfIz_TransitionTextureDesc transition{ };
    transition.Texture   = texture;
    transition.NewUsage  = newUsage;
    transition.QueueType = queueType;

    DenOfIz_BatchTransitionDesc batchTransition{ };
    batchTransition.Textures.Elements    = &transition;
    batchTransition.Textures.NumElements = 1;

    BatchTransition( commandList, &batchTransition );
}

void ResourceTracking::TransitionTextureMip( DenOfIz_CommandList commandList, DenOfIz_Texture texture, uint32_t mipLevel, uint32_t newUsage, DenOfIz_QueueType queueType )
{
    TextureSubresourceKey key{ texture, mipLevel };
    auto                  mipIt = m_textureMipStates.find( key );
    if ( mipIt == m_textureMipStates.end( ) )
    {
        return;
    }

    if ( mipIt->second.CurrentUsage == newUsage && mipIt->second.CurrentQueue == queueType )
    {
        return;
    }

    DenOfIz_TextureBarrierDesc textureBarrier{ };
    textureBarrier.Resource                 = texture;
    textureBarrier.OldState                 = mipIt->second.CurrentUsage;
    textureBarrier.NewState                 = newUsage;
    textureBarrier.EnableQueueBarrier       = mipIt->second.CurrentQueue != queueType;
    textureBarrier.SourceQueue              = mipIt->second.CurrentQueue;
    textureBarrier.DestinationQueue         = queueType;
    textureBarrier.EnableSubresourceBarrier = true;
    textureBarrier.MipLevel                 = mipLevel;
    textureBarrier.ArrayLayer               = 0;

    DenOfIz_PipelineBarrierDesc barrier{ };
    barrier.TextureBarriers.Elements    = &textureBarrier;
    barrier.TextureBarriers.NumElements = 1;

    DenOfIz_CommandList_PipelineBarrier( commandList, &barrier );

    mipIt->second.CurrentUsage = newUsage;
    mipIt->second.CurrentQueue = queueType;
}

void ResourceTracking::NotifyTextureTransitionMip( DenOfIz_Texture texture, uint32_t mipLevel, uint32_t newUsage )
{
    TextureSubresourceKey key{ texture, mipLevel };
    m_textureMipStates[ key ].CurrentUsage = newUsage;
}

void ResourceTracking::ProcessBufferTransitions( const DenOfIz_TransitionBufferDesc *buffers, const size_t numBuffers, std::vector<DenOfIz_BufferBarrierDesc> &bufferBarriers )
{
    for ( size_t i = 0; i < numBuffers; ++i )
    {
        const auto &desc = buffers[ i ];
        auto        it   = m_bufferStates.find( desc.Buffer );
        if ( it == m_bufferStates.end( ) )
        {
            continue;
        }
        if ( it->second.CurrentUsage == desc.NewUsage && it->second.CurrentQueue == desc.QueueType )
        {
            continue;
        }

        std::unique_lock          lock( it->second.Mutex );
        DenOfIz_BufferBarrierDesc bufferBarrier{ };
        bufferBarrier.Resource = desc.Buffer;
        bufferBarrier.OldState = it->second.CurrentUsage;
        bufferBarrier.NewState = desc.NewUsage;
        bufferBarriers.push_back( bufferBarrier );

        it->second.CurrentUsage = desc.NewUsage;
        it->second.CurrentQueue = desc.QueueType;
    }
}

void ResourceTracking::ProcessTextureTransitions( const DenOfIz_TransitionTextureDesc *textures, const size_t numTextures,
                                                  std::vector<DenOfIz_TextureBarrierDesc> &textureBarriers )
{
    for ( size_t i = 0; i < numTextures; ++i )
    {
        const auto &desc = textures[ i ];
        auto        it   = m_textureStates.find( desc.Texture );
        if ( it == m_textureStates.end( ) )
        {
            continue;
        }
        if ( it->second.CurrentUsage == desc.NewUsage && it->second.CurrentQueue == desc.QueueType )
        {
            continue;
        }

        std::unique_lock           lock( it->second.Mutex );
        DenOfIz_TextureBarrierDesc textureBarrier{ };
        textureBarrier.Resource           = desc.Texture;
        textureBarrier.OldState           = it->second.CurrentUsage;
        textureBarrier.NewState           = desc.NewUsage;
        textureBarrier.EnableQueueBarrier = it->second.CurrentQueue != desc.QueueType;
        textureBarrier.SourceQueue        = it->second.CurrentQueue;
        textureBarrier.DestinationQueue   = desc.QueueType;

        textureBarriers.push_back( textureBarrier );

        it->second.CurrentUsage = desc.NewUsage;
        it->second.CurrentQueue = desc.QueueType;

        for ( uint32_t mip = 0; mip < it->second.MipLevels; ++mip )
        {
            auto mipIt = m_textureMipStates.find( TextureSubresourceKey{ desc.Texture, mip } );
            if ( mipIt != m_textureMipStates.end( ) )
            {
                mipIt->second.CurrentUsage = desc.NewUsage;
                mipIt->second.CurrentQueue = desc.QueueType;
            }
        }
    }
}

void ResourceTracking::BatchTransition( DenOfIz_CommandList commandList, const DenOfIz_BatchTransitionDesc *desc )
{
    std::vector<DenOfIz_BufferBarrierDesc>  bufferBarriers;
    std::vector<DenOfIz_TextureBarrierDesc> textureBarriers;

    if ( desc->Buffers.Elements && desc->Buffers.NumElements > 0 )
    {
        ProcessBufferTransitions( desc->Buffers.Elements, desc->Buffers.NumElements, bufferBarriers );
    }

    if ( desc->Textures.Elements && desc->Textures.NumElements > 0 )
    {
        ProcessTextureTransitions( desc->Textures.Elements, desc->Textures.NumElements, textureBarriers );
    }

    DenOfIz_PipelineBarrierDesc barrier{ };
    barrier.BufferBarriers.Elements     = bufferBarriers.data( );
    barrier.BufferBarriers.NumElements  = bufferBarriers.size( );
    barrier.TextureBarriers.Elements    = textureBarriers.data( );
    barrier.TextureBarriers.NumElements = textureBarriers.size( );
    barrier.MemoryBarriers.Elements     = nullptr;
    barrier.MemoryBarriers.NumElements  = 0;

    DenOfIz_CommandList_PipelineBarrier( commandList, &barrier );
}

#define RESOURCE_TRACKING_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ResourceTracking, handle )

extern "C"
{

    void DenOfIz_ResourceTracking_Create( DenOfIz_ResourceTracking *outTracking )
    {
        if ( outTracking == NULL )
        {
            return;
        }

        ResourceTracking *tracking = new ResourceTracking( );
        *outTracking               = DENOFIZ_TO_HANDLE( tracking );
    }

    void DenOfIz_ResourceTracking_TrackBuffer( DenOfIz_ResourceTracking tracking, DenOfIz_Buffer buffer, DenOfIz_QueueType queueType )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( tracking ) || !DENOFIZ_HANDLE_IS_VALID( buffer ) )
        {
            return;
        }

        RESOURCE_TRACKING_IMPL( tracking )->TrackBuffer( buffer, queueType );
    }

    void DenOfIz_ResourceTracking_TrackTexture( DenOfIz_ResourceTracking tracking, DenOfIz_Texture texture, DenOfIz_QueueType queueType )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( tracking ) || !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return;
        }

        RESOURCE_TRACKING_IMPL( tracking )->TrackTexture( texture, queueType );
    }

    void DenOfIz_ResourceTracking_UntrackBuffer( DenOfIz_ResourceTracking tracking, DenOfIz_Buffer buffer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( tracking ) )
        {
            return;
        }

        RESOURCE_TRACKING_IMPL( tracking )->UntrackBuffer( buffer );
    }

    void DenOfIz_ResourceTracking_UntrackTexture( DenOfIz_ResourceTracking tracking, DenOfIz_Texture texture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( tracking ) )
        {
            return;
        }

        RESOURCE_TRACKING_IMPL( tracking )->UntrackTexture( texture );
    }

    void DenOfIz_ResourceTracking_TransitionBuffer( DenOfIz_ResourceTracking tracking, DenOfIz_CommandList commandList, DenOfIz_Buffer buffer, uint32_t newUsage,
                                                    DenOfIz_QueueType queueType )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( tracking ) || !DENOFIZ_HANDLE_IS_VALID( commandList ) || !DENOFIZ_HANDLE_IS_VALID( buffer ) )
        {
            return;
        }

        RESOURCE_TRACKING_IMPL( tracking )->TransitionBuffer( commandList, buffer, newUsage, queueType );
    }

    void DenOfIz_ResourceTracking_TransitionTexture( DenOfIz_ResourceTracking tracking, DenOfIz_CommandList commandList, DenOfIz_Texture texture, uint32_t newUsage,
                                                     DenOfIz_QueueType queueType )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( tracking ) || !DENOFIZ_HANDLE_IS_VALID( commandList ) || !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return;
        }

        RESOURCE_TRACKING_IMPL( tracking )->TransitionTexture( commandList, texture, newUsage, queueType );
    }

    void DenOfIz_ResourceTracking_TransitionTextureMip( DenOfIz_ResourceTracking tracking, DenOfIz_CommandList commandList, DenOfIz_Texture texture, uint32_t mipLevel,
                                                         uint32_t newUsage, DenOfIz_QueueType queueType )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( tracking ) || !DENOFIZ_HANDLE_IS_VALID( commandList ) || !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return;
        }

        RESOURCE_TRACKING_IMPL( tracking )->TransitionTextureMip( commandList, texture, mipLevel, newUsage, queueType );
    }

    void DenOfIz_ResourceTracking_BatchTransition( DenOfIz_ResourceTracking tracking, DenOfIz_CommandList commandList, const DenOfIz_BatchTransitionDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( tracking ) || !DENOFIZ_HANDLE_IS_VALID( commandList ) || desc == NULL )
        {
            return;
        }

        RESOURCE_TRACKING_IMPL( tracking )->BatchTransition( commandList, desc );
    }

    void DenOfIz_ResourceTracking_NotifyTextureTransition( DenOfIz_ResourceTracking tracking, DenOfIz_Texture texture, uint32_t newUsage )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( tracking ) || !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return;
        }

        RESOURCE_TRACKING_IMPL( tracking )->NotifyTextureTransition( texture, newUsage );
    }

    void DenOfIz_ResourceTracking_NotifyTextureTransitionMip( DenOfIz_ResourceTracking tracking, DenOfIz_Texture texture, uint32_t mipLevel, uint32_t newUsage )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( tracking ) || !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return;
        }

        RESOURCE_TRACKING_IMPL( tracking )->NotifyTextureTransitionMip( texture, mipLevel, newUsage );
    }

    void DenOfIz_ResourceTracking_NotifyBufferTransition( DenOfIz_ResourceTracking tracking, DenOfIz_Buffer buffer, uint32_t newUsage )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( tracking ) || !DENOFIZ_HANDLE_IS_VALID( buffer ) )
        {
            return;
        }

        RESOURCE_TRACKING_IMPL( tracking )->NotifyBufferTransition( buffer, newUsage );
    }

    void DenOfIz_ResourceTracking_Destroy( DenOfIz_ResourceTracking tracking )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( tracking ) )
        {
            return;
        }

        delete RESOURCE_TRACKING_IMPL( tracking );
    }
}
