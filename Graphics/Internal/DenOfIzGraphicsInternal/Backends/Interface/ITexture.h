/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/Texture.h"
#include "../Common/DeferredDestroyQueue.h"

namespace DenOfIz
{

    class ITexture : public IDeviceObject
    {
    public:
        virtual ~ITexture( )                        = default;
        virtual DenOfIz_Format GetFormat( ) const   = 0;
        virtual uint32_t       GetMipLevels( ) const = 0;
        virtual const DenOfIz_TextureDesc &GetDesc( ) const = 0;
    };

    class ISampler : public IDeviceObject
    {
    public:
        virtual ~ISampler( ) = default;
    };
} // namespace DenOfIz
