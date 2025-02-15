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

#include <DenOfIzGraphics/Renderer/Sync/ResourceTracking.h>

using namespace DenOfIz;

BatchTransitionDesc& BatchTransitionDesc::TransitionBuffer( IBufferResource *resource, const ResourceUsage newUsage, const QueueType queueType)
{
    TransitionBufferDesc& desc = BufferTransitions.EmplaceElement( );
    desc.Buffer    = resource;
    desc.NewUsage  = newUsage;
    desc.QueueType = queueType;

    return *this;
}

BatchTransitionDesc& BatchTransitionDesc::TransitionTexture( ITextureResource *resource, const ResourceUsage newUsage, const QueueType queueType )
{
    TransitionTextureDesc& desc = TextureTransitions.EmplaceElement( );
    desc.Texture    = resource;
    desc.NewUsage  = newUsage;
    desc.QueueType = queueType;

    return *this;
}

ResourceTracking::~ResourceTracking( )
{
}

void ResourceTracking::TrackBuffer( IBufferResource *buffer, const ResourceUsage currentUsage, const QueueType queueType )
{
    if ( m_bufferStates.contains( buffer ) )
    {
        LOG( WARNING ) << "Buffer already tracked";
    }

    m_bufferStates[ buffer ].CurrentQueue = queueType;
    m_bufferStates[ buffer ].CurrentUsage = currentUsage;
}

void ResourceTracking::TrackTexture( ITextureResource *texture, const ResourceUsage currentUsage, const QueueType queueType )
{
    if ( m_textureStates.contains( texture ) )
    {
        LOG( WARNING ) << "Texture already tracked";
    }

    m_textureStates[ texture ].CurrentQueue = queueType;
    m_textureStates[ texture ].CurrentUsage = texture->InitialState( );
}

void ResourceTracking::UntrackBuffer( IBufferResource *buffer )
{
    m_bufferStates.erase( buffer );
}

void ResourceTracking::UntrackTexture( ITextureResource *texture )
{
    m_textureStates.erase( texture );
}

void ResourceTracking::ProcessBufferTransitions( const InteropArray<TransitionBufferDesc> &bufferTransitions, PipelineBarrierDesc &barrier )
{
    for ( uint32_t i = 0; i < bufferTransitions.NumElements( ); ++i )
    {
        const auto &desc = bufferTransitions.GetElement( i );
        auto        it   = m_bufferStates.find( desc.Buffer );
        if ( it == m_bufferStates.end( ) )
        {
            continue;
        }
        if ( it->second.CurrentUsage == desc.NewUsage && it->second.CurrentQueue == desc.QueueType )
        {
            continue;
        }

        std::unique_lock  lock( it->second.Mutex );
        BufferBarrierDesc bufferBarrier{ };
        bufferBarrier.Resource = desc.Buffer;
        bufferBarrier.OldState = it->second.CurrentUsage;
        bufferBarrier.NewState = desc.NewUsage;
        barrier.BufferBarrier( bufferBarrier );

        it->second.CurrentUsage = desc.NewUsage;
        it->second.CurrentQueue = desc.QueueType;
    }
}

void ResourceTracking::ProcessTextureTransitions( const InteropArray<TransitionTextureDesc> &textureTransitions, PipelineBarrierDesc &barrier )
{
    for ( uint32_t i = 0; i < textureTransitions.NumElements( ); ++i )
    {
        const auto &desc = textureTransitions.GetElement( i );
        auto        it   = m_textureStates.find( desc.Texture );
        if ( it == m_textureStates.end( ) )
        {
            continue;
        }
        if ( it->second.CurrentUsage == desc.NewUsage && it->second.CurrentQueue == desc.QueueType )
        {
            continue;
        }

        std::unique_lock   lock( it->second.Mutex );
        TextureBarrierDesc textureBarrier{ };
        textureBarrier.Resource           = desc.Texture;
        textureBarrier.OldState           = it->second.CurrentUsage;
        textureBarrier.NewState           = desc.NewUsage;
        textureBarrier.EnableQueueBarrier = it->second.CurrentQueue != desc.QueueType;
        textureBarrier.SourceQueue        = it->second.CurrentQueue;
        textureBarrier.DestinationQueue   = desc.QueueType;

        barrier.TextureBarrier( textureBarrier );

        it->second.CurrentUsage = desc.NewUsage;
        it->second.CurrentQueue = desc.QueueType;
    }
}

void ResourceTracking::BatchTransition( const BatchTransitionDesc &desc )
{
    PipelineBarrierDesc barrier;

    ProcessBufferTransitions( desc.BufferTransitions, barrier );
    ProcessTextureTransitions( desc.TextureTransitions, barrier );

    desc.CommandList->PipelineBarrier( barrier );
}