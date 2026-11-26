/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/CommandListPool.h"
#include "ICommandQueue.h"

namespace DenOfIz
{
    class ICommandListPool
    {
    public:
        virtual DenOfIz_CommandListArray GetCommandLists( ) = 0;
        virtual ~ICommandListPool( )                        = default;
    };
} // namespace DenOfIz
