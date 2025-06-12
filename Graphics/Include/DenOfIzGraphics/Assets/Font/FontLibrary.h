/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <mutex>
#include <unordered_map>
#include <vector>
#include "DenOfIzGraphics/Assets/Import/FontImporter.h"
#include "DenOfIzGraphics/Utilities/Interop.h"
#include "Font.h"

namespace DenOfIz
{
    class FontLibrary
    {
        FontImporter m_fontImporter{ FontImporterDesc{} };

        FT_Library m_ftLibrary{ };
        std::mutex m_mutex;

        std::vector<std::unique_ptr<FontAsset>>                m_assets;
        std::vector<std::unique_ptr<Font>>                     m_fontStore;
        std::unordered_map<std::string, std::unique_ptr<Font>> m_fonts;

    public:
        DZ_API FontLibrary( );
        DZ_API ~FontLibrary( );
        DZ_API Font *LoadFont( const FontDesc &desc );
        DZ_API Font *LoadFont( const InteropString &ttf );
    };
} // namespace DenOfIz
