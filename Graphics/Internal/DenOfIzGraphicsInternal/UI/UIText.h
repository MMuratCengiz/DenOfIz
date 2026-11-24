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

#include <array>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "DenOfIzGraphicsInternal/UI/UITypes.h"

struct hb_font_t;
struct hb_face_t;
struct hb_blob_t;
struct hb_buffer_t;

namespace DenOfIz
{
    class Font;

    namespace UIGlyphFlags
    {
        constexpr uint8_t BreakAfter = 1 << 0;
        constexpr uint8_t Space      = 1 << 1;
        constexpr uint8_t Newline    = 1 << 2;
        constexpr uint8_t Tab        = 1 << 3;
    } // namespace UIGlyphFlags

    struct UIGlyph
    {
        uint32_t                 Cluster;
        float                    Advance;
        float                    OffsetX;
        float                    OffsetY;
        const DenOfIz_FontGlyph *Glyph;
        uint8_t                  Flags;
    };

    struct UIFontFace
    {
        Font             *FontPtr     = nullptr;
        DenOfIz_FontAsset Asset       = DENOFIZ_NULL_HANDLE;
        hb_blob_t        *Blob        = nullptr;
        hb_face_t        *Face        = nullptr;
        hb_font_t        *HbFont      = nullptr;
        DenOfIz_Texture   Atlas       = DENOFIZ_NULL_HANDLE;
        uint32_t          Generation  = 0;
        float             BaseSize    = 1.0f;
        float             Ascent      = 0.0f;
        float             Descent     = 0.0f;
        float             LineHeight  = 0.0f;
        float             AtlasWidth  = 1.0f;
        float             AtlasHeight = 1.0f;
        float             SpaceAdvance = 0.0f;
        std::vector<UIGlyph> Ellipsis;
        float                EllipsisAdvance = 0.0f;
    };

    struct UIShapedRun
    {
        uint16_t             FontId     = 0;
        uint32_t             Generation = 0;
        uint64_t             Hash       = 0;
        std::string          Text;
        std::vector<UIGlyph> Glyphs;
        float                LongestWord   = 0.0f;
        uint32_t             LongestWordGlyphs = 0;
        mutable uint64_t     LastUsedFrame = 0;
    };

    struct UITextParams
    {
        uint16_t               FontId        = 0;
        float                  FontSize      = 14.0f;
        float                  LetterSpacing = 0.0f;
        float                  LineHeight    = 0.0f;
        DenOfIz_UITextWrap     Wrap          = DENOFIZ_UI_TEXT_WRAP_WORDS;
        DenOfIz_UITextOverflow Overflow      = DENOFIZ_UI_TEXT_OVERFLOW_CLIP;
        uint32_t               MaxLines      = 0;

        bool operator==( const UITextParams &other ) const;
        static UITextParams From( const DenOfIz_UITextDesc &desc );
    };

    struct UITextLineLayout
    {
        uint32_t GlyphStart;
        uint32_t GlyphEnd;
        uint32_t ByteStart;
        uint32_t ByteEnd;
        float    Width;
        bool     Ellipsis;
    };

    struct UIGlyphQuad
    {
        float    X0, Y0, X1, Y1;
        float    U0, V0, U1, V1;
        uint32_t Line;
    };

    class UITextLayout
    {
    public:
        const UIShapedRun            *Run = nullptr;
        const UIFontFace             *Face = nullptr;
        UITextParams                  Params;
        float                         MaxWidth = 0.0f;
        float                         Scale    = 1.0f;
        float                         Width    = 0.0f;
        float                         Height   = 0.0f;
        float                         LineHeight = 0.0f;
        float                         Baseline   = 0.0f;
        uint32_t                      TextureSlot = 0;
        std::vector<UITextLineLayout> Lines;
        std::vector<float>            GlyphX;
        mutable std::vector<UIGlyphQuad> Quads;
        mutable bool                  QuadsBuilt    = false;
        mutable uint64_t              LastUsedFrame = 0;

        float                 AlignOffset( uint32_t line, float boxWidth, DenOfIz_UITextAlign align ) const;
        float                 CaretX( uint32_t line, uint32_t byteOffset ) const;
        uint32_t              LineOfOffset( uint32_t byteOffset ) const;
        uint32_t              OffsetAt( float x, float y, float boxWidth, DenOfIz_UITextAlign align ) const;
        const std::vector<UIGlyphQuad> &GetQuads( ) const;
    };

    struct UITextLayoutKey
    {
        const UIShapedRun *Run;
        UITextParams       Params;
        float              MaxWidth;

        bool operator==( const UITextLayoutKey &other ) const;
    };

    struct UITextLayoutKeyHash
    {
        size_t operator( )( const UITextLayoutKey &key ) const;
    };

    struct UIRunKey
    {
        uint64_t Hash;
        uint32_t Length;
        uint16_t FontId;
        uint32_t Generation;

        bool operator==( const UIRunKey &other ) const;
    };

    struct UIRunKeyHash
    {
        size_t operator( )( const UIRunKey &key ) const;
    };

    class UIText
    {
        DenOfIz_LogicalDevice                                                           m_device;
        std::array<UIFontFace, UITextureSlots::MaxFonts>                                  m_faces{ };
        std::unordered_map<UIRunKey, std::unique_ptr<UIShapedRun>, UIRunKeyHash>        m_runs;
        std::unordered_map<UITextLayoutKey, std::unique_ptr<UITextLayout>, UITextLayoutKeyHash> m_layouts;
        uint64_t                                                                        m_frame      = 1;
        uint64_t                                                                        m_epoch      = 1;
        uint32_t                                                                        m_generation = 1;
        hb_buffer_t                                                                    *m_buffer = nullptr;

    public:
        explicit UIText( DenOfIz_LogicalDevice device );
        ~UIText( );

        void              AddFont( uint16_t fontId, Font *font );
        void              RemoveFont( uint16_t fontId );
        const UIFontFace *GetFace( uint16_t fontId ) const;
        DenOfIz_Texture   GetAtlas( uint32_t slot ) const;

        static uint64_t     Hash( const char *chars, size_t length );
        const UIShapedRun  *Shape( uint16_t fontId, const char *chars, uint32_t length, uint64_t hash );
        const UITextLayout *Layout( const UIShapedRun *run, const UITextParams &params, float maxWidth );
        float               MinContentWidth( const UIShapedRun *run, const UITextParams &params ) const;
        DenOfIz_Float2      Measure( const char *chars, uint32_t length, const DenOfIz_UITextDesc &desc, float maxWidth );

        void     BeginFrame( uint64_t frame );
        void     Evict( uint64_t maxAge );
        uint64_t Epoch( ) const;
        uint64_t Frame( ) const;
        uint16_t ResolveFontId( uint16_t fontId ) const;

    private:
        void     LoadFace( UIFontFace &face, Font *font );
        void     ReleaseFace( UIFontFace &face );
        void     UploadAtlas( UIFontFace &face );
        void     ShapeInto( const UIFontFace &face, const char *chars, uint32_t length, std::vector<UIGlyph> &outGlyphs );
        void     BuildLayout( UITextLayout &layout ) const;
    };
} // namespace DenOfIz
