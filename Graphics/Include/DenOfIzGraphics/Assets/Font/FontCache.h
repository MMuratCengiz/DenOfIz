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

#include <DenOfIzGraphics/Assets/Serde/Font/FontAsset.h>
#include <DenOfIzGraphics/Utilities/Common.h>

namespace DenOfIz
{
    struct AddGlyphDesc
    {
        uint32_t           CodePoint;
        uint32_t           Width;
        uint32_t           Height;
        uint32_t           BearingX;
        uint32_t           BearingY;
        uint32_t           Advance;
        // MSDF data - stored as RGB
        InteropArray<Byte> MsdfData;
        uint32_t           MsdfPitch;
    };

    class DZ_API FontCache
    {
        FontAsset            m_fontAsset;
        std::vector<uint8_t> m_atlasBitmap;
        bool                 m_atlasNeedsUpdate = false;

        uint32_t m_currentAtlasX = 0;
        uint32_t m_currentAtlasY = 0;
        uint32_t m_rowHeight     = 0;

        struct Rect
        {
            uint32_t X;
            uint32_t Y;
            uint32_t Width;
            uint32_t Height;
        };

    public:
        explicit FontCache( const FontAsset &fontAsset );
        ~FontCache( ) = default;

        void InitializeAtlasBitmap( const InteropArray<Byte> &initialBitmap = { } );

        [[nodiscard]] const FontAsset            &GetFontAsset( ) const;
        [[nodiscard]] const std::vector<uint8_t> &GetAtlasBitmap( ) const;

        void AddGlyph( const AddGlyphDesc &desc );

        [[nodiscard]] bool                HasGlyph( uint32_t codePoint ) const;
        [[nodiscard]] const GlyphMetrics *GetGlyphMetrics( uint32_t codePoint ) const;

        void               ClearAtlasBitmap( );
        void               ResizeAtlas( uint32_t newWidth, uint32_t newHeight );
        [[nodiscard]] bool AtlasNeedsUpdate( ) const;
        void               MarkAtlasUpdated( );

    private:
        Rect AllocateSpace( uint32_t width, uint32_t height );
        void CopyMsdfDataToAtlas( const Rect &rect, const InteropArray<Byte> &msdfData, uint32_t msdfPitch );
    };
} // namespace DenOfIz
