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

#include <DenOfIzGraphics/Assets/FileSystem/PathResolver.h>
#include <DenOfIzGraphics/Assets/Font/FontCache.h>
#include <DenOfIzGraphics/Assets/Serde/Font/FontAsset.h>
#include <DenOfIzGraphics/Utilities/Common.h>

// Forward declare FreeType structures to avoid exposing them in the header
typedef struct FT_LibraryRec_ *FT_Library;
typedef struct FT_FaceRec_    *FT_Face;

using namespace DirectX;

namespace DenOfIz
{
    class FontManager
    {
        FT_Library                                                  m_ftLibrary{ };
        std::unordered_map<std::string, std::shared_ptr<FontCache>> m_fontCache;

    public:
        FontManager( );
        ~FontManager( );

        std::shared_ptr<FontCache> LoadFont( const std::string &fontPath, uint32_t pixelSize = 24, bool antiAliasing = true );
        std::shared_ptr<FontCache> GetFont( const std::string &fontPath, uint32_t pixelSize = 24 );
        bool                       EnsureGlyphsLoaded( const std::shared_ptr<FontCache> &fontCache, const std::u32string &text );
        void GenerateTextVertices( const std::shared_ptr<FontCache> &fontCache, const std::u32string &text, std::vector<float> &vertices, std::vector<uint32_t> &indices, float x,
                                   float y, const XMFLOAT4 &color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), float scale = 1.0f );
        static std::u32string Utf8ToUtf32( const std::string &utf8Text );

    private:
        bool LoadGlyph( const std::shared_ptr<FontCache> &fontCache, uint32_t codePoint, FT_Face face );
    };

} // namespace DenOfIz
