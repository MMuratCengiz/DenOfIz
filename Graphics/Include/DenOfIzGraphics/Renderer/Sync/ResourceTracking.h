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

#include <mutex>
#include <unordered_map>
#include "DenOfIzGraphics/Backends/Interface/ILogicalDevice.h"

namespace DenOfIz
{
    struct ResourceState
    {
        std::mutex Mutex;
        uint32_t   CurrentUsage = ResourceUsage::Undefined;
        QueueType  CurrentQueue = QueueType::Graphics;
    };

    struct DZ_API TransitionResourceDesc
    {
        ICommandList *CommandList = nullptr;
        uint32_t      NewUsage    = ResourceUsage::Undefined;
        QueueType     QueueType   = QueueType::Graphics;
    };

    struct DZ_API TransitionBufferDesc : TransitionResourceDesc
    {
        IBufferResource *Buffer = nullptr;
    };
    template class DZ_API InteropArray<TransitionBufferDesc>;

    struct DZ_API TransitionTextureDesc : TransitionResourceDesc
    {
        ITextureResource *Texture = nullptr;
    };
    template class DZ_API InteropArray<TransitionTextureDesc>;

    struct DZ_API BatchTransitionDesc
    {
    private:
        ICommandList                       *m_commandList;
        InteropArray<TransitionBufferDesc>  m_bufferTransitions;
        InteropArray<TransitionTextureDesc> m_textureTransitions;

        friend class ResourceTracking;

    public:
        explicit BatchTransitionDesc( ICommandList *commandList ) : m_commandList( commandList )
        {
        }
        void Reset( ICommandList *commandList ); // Resource pooling
        void TransitionBuffer( IBufferResource *resource, const uint32_t &newUsage, QueueType queueType = QueueType::Graphics );
        void TransitionTexture( ITextureResource *resource, const uint32_t &newUsage, QueueType queueType = QueueType::Graphics );
    };

    class ResourceTracking
    {
        std::unordered_map<IBufferResource *, ResourceState>  m_bufferStates;
        std::unordered_map<ITextureResource *, ResourceState> m_textureStates;

    public:
        DZ_API ResourceTracking( ) = default;
        DZ_API ~ResourceTracking( );

        DZ_API void TrackBuffer( IBufferResource *buffer, const uint32_t &currentUsage, QueueType queueType = QueueType::Graphics );
        DZ_API void TrackTexture( ITextureResource *texture, const uint32_t &currentUsage, QueueType queueType = QueueType::Graphics );

        DZ_API void UntrackBuffer( IBufferResource *buffer );
        DZ_API void UntrackTexture( ITextureResource *texture );

        DZ_API void BatchTransition( const BatchTransitionDesc &desc );

    private:
        void ProcessBufferTransitions( const InteropArray<TransitionBufferDesc> &bufferTransitions, PipelineBarrierDesc &barrier );
        void ProcessTextureTransitions( const InteropArray<TransitionTextureDesc> &textureTransitions, PipelineBarrierDesc &barrier );
    };
} // namespace DenOfIz
