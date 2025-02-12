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

#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>
#include <mutex>
#include <unordered_map>

namespace DenOfIz
{
    struct DZ_API ResourceState
    {
        std::mutex    Mutex;
        ResourceUsage CurrentUsage = ResourceUsage::Undefined;
        QueueType     CurrentQueue = QueueType::Graphics;
    };

    struct DZ_API TransitionResourceDesc
    {
        ICommandList *CommandList{ };
        ResourceUsage NewUsage  = ResourceUsage::Undefined;
        QueueType     QueueType = QueueType::Graphics;
    };

    struct DZ_API TransitionBufferDesc : TransitionResourceDesc
    {
        IBufferResource *Buffer{ };
    };

    struct DZ_API TransitionTextureDesc : TransitionResourceDesc
    {
        ITextureResource *Texture{ };
    };

    struct DZ_API BatchTransitionDesc
    {
        ICommandList                       *CommandList;
        InteropArray<TransitionBufferDesc>  BufferTransitions;
        InteropArray<TransitionTextureDesc> TextureTransitions;
    };

    class DZ_API ResourceTracking
    {
        std::unordered_map<IBufferResource *, ResourceState>  m_bufferStates;
        std::unordered_map<ITextureResource *, ResourceState> m_textureStates;

    public:
        ResourceTracking( ) = default;
        ~ResourceTracking( );

        void TrackBuffer( IBufferResource *buffer, ResourceUsage currentUsage, QueueType queueType = QueueType::Graphics );
        void TrackTexture( ITextureResource *texture, ResourceUsage currentUsage, QueueType queueType = QueueType::Graphics );

        void UntrackBuffer( IBufferResource *buffer );
        void UntrackTexture( ITextureResource *texture );

        void TransitionBuffer( const TransitionBufferDesc &desc );
        void TransitionTexture( const TransitionTextureDesc &desc );
        void BatchTransition( const BatchTransitionDesc &desc );

    private:
        void ProcessBufferTransitions( const InteropArray<TransitionBufferDesc> &bufferTransitions, PipelineBarrierDesc &barrier );
        void ProcessTextureTransitions( const InteropArray<TransitionTextureDesc> &textureTransitions, PipelineBarrierDesc &barrier );
    };
} // namespace DenOfIz
