/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/CommandQueue.h"

namespace DenOfIz
{
    class ICommandQueue
    {
    public:
        virtual ~ICommandQueue( )                                                                          = default;
        virtual void WaitIdle( )                                                                           = 0;
        virtual void ExecuteCommandLists( const DenOfIz_ExecuteCommandListsDesc &executeCommandListsDesc ) = 0;
    };
} // namespace DenOfIz
