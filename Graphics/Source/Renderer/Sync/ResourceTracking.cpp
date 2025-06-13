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

#include "DenOfIzGraphics/Renderer/Sync/ResourceTracking.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

void BatchTransitionDesc::Reset( ICommandList *commandList )
{
    m_bufferTransitions.clear( );
    m_textureTransitions.clear( );
    m_commandList = commandList;
}

void BatchTransitionDesc::TransitionBuffer( IBufferResource *resource, const uint32_t &newUsage, const QueueType queueType )
{
    TransitionBufferDesc &desc = m_bufferTransitions.emplace_back( TransitionBufferDesc{ } );
    desc.Buffer                = resource;
    desc.NewUsage              = newUsage;
    desc.QueueType             = queueType;
}

void BatchTransitionDesc::TransitionTexture( ITextureResource *resource, const uint32_t &newUsage, const QueueType queueType )
{
    TransitionTextureDesc &desc = m_textureTransitions.emplace_back( TransitionTextureDesc{ } );
    desc.Texture                = resource;
    desc.NewUsage               = newUsage;
    desc.QueueType              = queueType;
}

ResourceTracking::~ResourceTracking( )
{
}

void ResourceTracking::TrackBuffer( IBufferResource *buffer, const uint32_t &currentUsage, const QueueType queueType )
{
    if ( m_bufferStates.contains( buffer ) )
    {
        spdlog::warn( "Buffer already tracked" );
    }

    m_bufferStates[ buffer ].CurrentQueue = queueType;
    m_bufferStates[ buffer ].CurrentUsage = currentUsage;
}

void ResourceTracking::TrackTexture( ITextureResource *texture, const uint32_t &currentUsage, const QueueType queueType )
{
    // Trust the user that this is an update.
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

void ResourceTracking::ProcessBufferTransitions( const std::vector<TransitionBufferDesc> &bufferTransitions, PipelineBarrierDesc &barrier )
{
    for ( const auto &desc : bufferTransitions )
    {
        auto it = m_bufferStates.find( desc.Buffer );
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

void ResourceTracking::ProcessTextureTransitions( const std::vector<TransitionTextureDesc> &textureTransitions, PipelineBarrierDesc &barrier )
{
    for ( const auto &desc : textureTransitions )
    {
        auto it = m_textureStates.find( desc.Texture );
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

    ProcessBufferTransitions( desc.m_bufferTransitions, barrier );
    ProcessTextureTransitions( desc.m_textureTransitions, barrier );

    desc.m_commandList->PipelineBarrier( barrier );
}
