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

#include "DenOfIzGraphics/Backends/Interface/RayTracing/ShaderLocalData.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/IBuffer.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/ITexture.h"

namespace DenOfIz
{

    class IShaderLocalData
    {
    public:
        virtual void Begin( ) = 0;
        // Non-const because Vulkan supports inline only, and uses map memory as a workaround the issue
        virtual void Cbv( uint32_t binding, IBuffer *bufferResource )           = 0;
        virtual void Cbv( uint32_t binding, const DenOfIz_ByteArrayView &data ) = 0;
        virtual void Srv( uint32_t binding, const IBuffer *textureResource )    = 0;
        virtual void Srv( uint32_t binding, const ITexture *textureResource )   = 0;
        virtual void Uav( uint32_t binding, const IBuffer *textureResource )    = 0;
        virtual void Uav( uint32_t binding, const ITexture *textureResource )   = 0;
        virtual void Sampler( uint32_t binding, const ISampler *sampler )       = 0;
        virtual void End( )                                                     = 0;
        virtual ~IShaderLocalData( )                                            = default;
    };
} // namespace DenOfIz
