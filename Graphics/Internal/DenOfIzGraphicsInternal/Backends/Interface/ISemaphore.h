/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/Semaphore.h"
#include "DenOfIzGraphics/Utilities/Common_Macro.h"

namespace DenOfIz
{
    class ISemaphore
    {
    public:
        virtual void Notify( )            = 0;
        virtual bool IsCompleted( ) const = 0;
        virtual ~ISemaphore( )            = default;
    };
} // namespace DenOfIz
