/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Backends/Interface/Texture.h"
#include "Interop.h"

namespace DenOfIz
{
    class NullTexture
    {
        DenOfIz_Texture m_texture = DENOFIZ_NULL_HANDLE;

    public:
        DZ_EXAMPLES_API NullTexture( DenOfIz_LogicalDevice device );
        DZ_EXAMPLES_API ~NullTexture( );
        [[nodiscard]] DZ_EXAMPLES_API DenOfIz_Texture Texture( ) const;
    };
} // namespace DenOfIz
