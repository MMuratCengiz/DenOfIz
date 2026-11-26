/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/RayTracing/BottomLevelAS.h"
#include "../../Common/DeferredDestroyQueue.h"

namespace DenOfIz
{

    class IBottomLevelAS : public IDeviceObject
    {
    public:
        virtual ~IBottomLevelAS( )            = default;
        virtual size_t BuildNumBytes( ) const = 0;
    };
} // namespace DenOfIz
