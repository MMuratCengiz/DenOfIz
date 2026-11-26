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

    // UTS #51 presentation of a grapheme cluster: VS15 after the base asks for text; VS16, a keycap, a skin tone modifier, a ZWJ sequence joining
    // emoji or an Emoji_Presentation base (regional indicators among them) for emoji.
    TextPresentation ClusterPresentation( const char *cluster, uint32_t length );
} // namespace DenOfIz
