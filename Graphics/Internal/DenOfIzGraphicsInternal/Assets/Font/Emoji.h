/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <cstdint>

namespace DenOfIz
{
    enum class TextPresentation : uint8_t
    {
        Default,
        Text,
        Emoji
    };

    TextPresentation ClusterPresentation( const char *cluster, uint32_t length );
} // namespace DenOfIz
