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
#include <freetype/freetype.h>
#include <harfbuzz/hb-ft.h>
#include <harfbuzz/hb.h>
#include <mutex>
#include <ranges>
#include <unordered_map>

namespace DenOfIz
{
    struct FontImpl
    {
        FT_Library m_ftLibrary{ };
        FT_Face    m_face{ };
        
        mutable std::unordered_map<uint32_t, hb_font_t*> m_hbFonts;
        mutable std::mutex m_hbFontsMutex;
        mutable std::mutex m_faceMutex;

        explicit FontImpl( const FT_Library library ) : m_ftLibrary( library )
        {
        }

        ~FontImpl( )
        {
            for ( auto &font : m_hbFonts | std::views::values )
            {
                if ( font )
                {
                    hb_font_destroy( font );
                }
            }
            
            if ( m_face )
            {
                FT_Done_Face( m_face );
            }
        }
    };
} // namespace DenOfIz
