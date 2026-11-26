/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/RayTracing/ShaderLocalData.h"
#include "../../Common/DeferredDestroyQueue.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/IBuffer.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/ITexture.h"

namespace DenOfIz
{

    class IShaderLocalData : public IDeviceObject
    {
    public:
        virtual void Begin( )                                                   = 0;
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
