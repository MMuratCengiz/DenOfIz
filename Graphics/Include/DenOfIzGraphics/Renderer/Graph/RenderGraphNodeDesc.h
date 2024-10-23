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

#include <DenOfIzGraphics/Utilities/Interop.h>
#include <DenOfIzGraphics/Backends/Interface/IBufferResource.h>
#include <DenOfIzGraphics/Backends/Interface/ITextureResource.h>
#include <DenOfIzGraphics/Backends/GraphicsApi.h>

namespace DenOfIz
{
    enum class NodeResourceUsageType
    {
        Buffer,
        Texture
    };

    struct DZ_API NodeResourceUsageDesc
    {
        explicit NodeResourceUsageDesc( IBufferResource *resource );
        explicit NodeResourceUsageDesc( ITextureResource *resource );
        NodeResourceUsageDesc( ) = default;

        uint32_t              FrameIndex = 0;
        ResourceState         State      = ResourceState::Undefined;
        NodeResourceUsageType Type       = NodeResourceUsageType::Buffer;
        union
        {
            IBufferResource  *BufferResource;
            ITextureResource *TextureResource;
        };
        static NodeResourceUsageDesc BufferState( const uint32_t frameIndex, IBufferResource *bufferResource, const ResourceState state );
        static NodeResourceUsageDesc TextureState( const uint32_t frameIndex, ITextureResource *textureResource, const ResourceState state );
    };
    template class DZ_API InteropArray<NodeResourceUsageDesc>;


    // Interop friendly callback
    class DZ_API NodeExecutionCallback
    {
    public:
        virtual ~NodeExecutionCallback( ) {};
        virtual void Execute( uint32_t frameIndex, ICommandList *commandList ) {};
    };

    struct DZ_API NodeDesc
    {
        InteropString                       Name;
        QueueType                           QueueType;
        InteropArray<InteropString>         Dependencies;
        InteropArray<NodeResourceUsageDesc> RequiredStates;
        NodeExecutionCallback              *Execute;
    };

    class DZ_API PresentExecutionCallback
    {
    public:
        virtual ~PresentExecutionCallback( ) {};
        virtual void Execute( uint32_t frameIndex, ICommandList *commandList, ITextureResource *texture ) {};
    };

    struct DZ_API PresentNodeDesc
    {
        InteropArray<InteropString>         Dependencies;
        InteropArray<NodeResourceUsageDesc> RequiredStates;
        ISwapChain                         *SwapChain;
        PresentExecutionCallback           *Execute;
    };
}
