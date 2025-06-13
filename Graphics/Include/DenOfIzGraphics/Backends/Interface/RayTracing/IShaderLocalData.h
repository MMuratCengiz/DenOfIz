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

#include "DenOfIzGraphics/Backends/Interface/IBufferResource.h"
#include "DenOfIzGraphics/Backends/Interface/ITextureResource.h"
#include "DenOfIzGraphics/Utilities/Interop.h"
#include "ILocalRootSignature.h"

namespace DenOfIz
{
    struct DZ_API ShaderLocalDataDesc
    {
        ILocalRootSignature *Layout;
    };

    class DZ_API IShaderLocalData
    {
    public:
        virtual void Begin( ) = 0;
        // Non-const because Vulkan supports inline only, and uses map memory as a workaround the issue
        virtual void Cbv( uint32_t binding, IBufferResource *bufferResource )         = 0;
        virtual void Cbv( uint32_t binding, const ByteArrayView &data )               = 0;
        virtual void Srv( uint32_t binding, const IBufferResource *textureResource )  = 0;
        virtual void Srv( uint32_t binding, const ITextureResource *textureResource ) = 0;
        virtual void Uav( uint32_t binding, const IBufferResource *textureResource )  = 0;
        virtual void Uav( uint32_t binding, const ITextureResource *textureResource ) = 0;
        virtual void Sampler( uint32_t binding, const ISampler *sampler )             = 0;
        virtual void End( )                                                           = 0;
        virtual ~IShaderLocalData( )                                                  = default;
    };
} // namespace DenOfIz
