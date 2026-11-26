/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/RayTracing/LocalRootSignature.h"
#include "../../Common/DeferredDestroyQueue.h"

namespace DenOfIz
{
    class ILocalRootSignature : public IDeviceObject
    {
    public:
        virtual ~ILocalRootSignature( ) = default;
    };
} // namespace DenOfIz
