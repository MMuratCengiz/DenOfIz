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

#include <DenOfIzGraphics/Assets/Font/Font.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <harfbuzz/hb.h>
#include <harfbuzz/hb-ft.h>
#include <mutex>
#include <ranges>
#include <unordered_map>

namespace DenOfIz
{
    struct FontImpl
    {
        FT_Library m_ftLibrary{ };
        FT_Face    m_baseFace{ };
        FT_Int32   m_loadFlags{ FT_LOAD_DEFAULT };

        mutable std::unordered_map<uint32_t, FT_Face>     m_sizedFaces;
        mutable std::unordered_map<uint32_t, hb_font_t *> m_hbFonts;
        mutable std::unordered_map<uint32_t, uint32_t>    m_glyphIndexToCodePoint;
        mutable std::mutex                                m_mutex;
        mutable bool                                      m_glyphMapInitialized{ false };

        explicit FontImpl( const FT_Library library ) : m_ftLibrary( library )
        {
        }

        ~FontImpl( )
        {
            for ( const auto &font : m_hbFonts | std::views::values )
            {
                if ( font )
                {
                    hb_font_destroy( font );
                }
            }

            for ( const auto &face : m_sizedFaces | std::views::values )
            {
                if ( face )
                {
                    FT_Done_Face( face );
                }
            }

            if ( m_baseFace )
            {
                FT_Done_Face( m_baseFace );
            }
        }
    };
} // namespace DenOfIz
