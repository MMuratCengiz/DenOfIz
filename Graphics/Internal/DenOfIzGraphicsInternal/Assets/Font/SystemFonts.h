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

    // PostScriptName, when set, picks the face of a collection by its name table instead of FaceIndex.
    struct SystemFontFile
    {
        std::string Path;
        std::string PostScriptName;
        uint32_t    FaceIndex = 0;
    };

    // The installed font the OS would draw the UTF-8 text with, implemented per platform.
    bool FindSystemFont( const char *text, uint32_t length, SystemFontFile &out );
    // Other installed fonts covering the text, searched when the OS's pick cannot be drawn or is not of the kind asked for. Slow.
    void FindSystemFonts( const char *text, uint32_t length, std::vector<SystemFontFile> &out );

    // A system font covering every code point of the text, null when there is none. colour asks for an emoji font, otherwise one with outlines
    // is preferred; a font of the other kind is returned when it is the only one. Lookups and fonts are cached for the process.
    Font *SystemFallbackFont( const char *text, uint32_t length, hb_script_t script, bool colour );
} // namespace DenOfIz
