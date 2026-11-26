/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"

namespace DenOfIz
{
    class EmbeddedImGuiShaders
    {
    public:
        static DenOfIz_ShaderProgram GetImGuiShaderProgram( );
    };
} // namespace DenOfIz
