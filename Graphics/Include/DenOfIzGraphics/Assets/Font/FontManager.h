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
#include <DenOfIzGraphics/Assets/Font/FontAsset.h>
#include <DenOfIzGraphics/Utilities/Common.h>

// Forward declare FreeType structures to avoid exposing them in the header
typedef struct FT_LibraryRec_ *FT_Library;
typedef struct FT_FaceRec_    *FT_Face;

using namespace DirectX;

namespace DenOfIz
{
    class FontManager
    {
        FT_Library                                                  m_ftLibrary{};
        std::unordered_map<std::string, std::shared_ptr<FontAsset>> m_fontCache;

        struct Rect
        {
            uint32_t X;
            uint32_t Y;
            uint32_t Width;
            uint32_t Height;
        };

        // Atlas packing state
        uint32_t m_currentAtlasX = 0;
        uint32_t m_currentAtlasY = 0;
        uint32_t m_rowHeight     = 0;

    public:
        FontManager( );
        ~FontManager( );

        std::shared_ptr<FontAsset> LoadFont( const std::string &fontPath, uint32_t pixelSize = 24, bool antiAliasing = true );
        std::shared_ptr<FontAsset> GetFont( const std::string &fontPath, uint32_t pixelSize = 24 );
        bool                       EnsureGlyphsLoaded( const std::shared_ptr<FontAsset> &font, const std::u32string &text );
        void GenerateTextVertices( const std::shared_ptr<FontAsset> &font, const std::u32string &text, std::vector<float> &vertices, std::vector<uint32_t> &indices, float x, float y,
                                   const XMFLOAT4 &color = XMFLOAT4( 1.0f, 1.0f, 1.0f, 1.0f ), float scale = 1.0f );
        static std::u32string Utf8ToUtf32( const std::string &utf8Text );

    private:
        bool LoadGlyph( const std::shared_ptr<FontAsset> &font, uint32_t codePoint, FT_Face face );
        Rect AllocateSpace( const std::shared_ptr<FontAsset> &font, uint32_t width, uint32_t height );
        void CopyGlyphToAtlas( const std::shared_ptr<FontAsset> &font, FT_Face face, const Rect &rect );
    };

} // namespace DenOfIz
