/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/InputLayout.h"
#include "../Common/DeferredDestroyQueue.h"

namespace DenOfIz
{

    class IInputLayout : public IDeviceObject
    {
    public:
        virtual ~IInputLayout( ) = default;
    };

} // namespace DenOfIz
