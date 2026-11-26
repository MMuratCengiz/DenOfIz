/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Utilities/Common_Macro.h"

namespace DenOfIz
{

    class IFence
    {
    public:
        virtual void Wait( )  = 0;
        virtual void Reset( ) = 0;
        virtual ~IFence( )    = default;
    };

} // namespace DenOfIz
