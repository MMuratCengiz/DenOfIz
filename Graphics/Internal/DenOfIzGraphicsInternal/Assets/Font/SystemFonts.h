/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <harfbuzz/hb.h>
#include <string>
#include <vector>

namespace DenOfIz
{
    class Font;

    struct SystemFontFile
    {
        std::string Path;
        std::string PostScriptName;
        uint32_t    FaceIndex = 0;
    };

    bool FindSystemFont( const char *text, uint32_t length, SystemFontFile &out );
    void FindSystemFonts( const char *text, uint32_t length, std::vector<SystemFontFile> &out );

    Font *SystemFallbackFont( const char *text, uint32_t length, hb_script_t script, bool colour );
} // namespace DenOfIz
