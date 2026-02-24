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

#include "DenOfIzGraphics/Assets/Font/FontLibrary.h"
#include "DenOfIzGraphics/Assets/Import/FontImporter.h"
#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"

#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

namespace DenOfIz
{
    class FontLibrary
    {
    public:
        DenOfIz_FontImporter m_fontImporter;
        FT_Library           m_ftLibrary{ };
        std::mutex           m_mutex;

        std::vector<DenOfIz_FontAsset>                         m_assets;
        std::vector<std::unique_ptr<Font>>                     m_fontStore;
        std::unordered_map<std::string, std::unique_ptr<Font>> m_fonts;

        FontLibrary( );
        ~FontLibrary( );

        Font *LoadFont( const DenOfIz_FontDesc &desc );
        Font *LoadFont( const DenOfIz_StringView &ttf );
    };
} // namespace DenOfIz

#define FONT_LIBRARY_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::FontLibrary, handle )
