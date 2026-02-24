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

#include "DenOfIzGraphics/Assets/Serde/Asset.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum DenOfIz_AntiAliasingMode
    {
        DENOFIZ_ANTI_ALIASING_MODE_NONE,
        DENOFIZ_ANTI_ALIASING_MODE_GRAYSCALE,
        DENOFIZ_ANTI_ALIASING_MODE_SUBPIXEL
    } DenOfIz_AntiAliasingMode;

    typedef struct DenOfIz_GlyphBounds
    {
        double XMin;
        double YMin;
        double XMax;
        double YMax;
    } DenOfIz_GlyphBounds;

    typedef struct DenOfIz_FontGlyph
    {
        uint32_t            CodePoint;
        DenOfIz_GlyphBounds Bounds;
        uint32_t            Width;
        uint32_t            Height;
        float               BearingX;
        float               BearingY;
        float               XAdvance;
        float               YAdvance;
        uint32_t            AtlasX;
        uint32_t            AtlasY;
    } DenOfIz_FontGlyph;

    typedef struct DenOfIz_FontGlyphArray
    {
        DenOfIz_FontGlyph *Elements;
        size_t             NumElements;
    } DenOfIz_FontGlyphArray;

    typedef struct DenOfIz_FontMetrics
    {
        float Ascent;
        float Descent;
        float LineGap;
        float LineHeight;
        float UnderlinePos;
        float UnderlineThickness;
    } DenOfIz_FontMetrics;

    DENOFIZ_DEFINE_HANDLE( DenOfIz_FontAsset )

#define DENOFIZ_FONT_ASSET_NUM_CHANNELS 4

    DZ_API DenOfIz_FontAsset        DenOfIz_FontAsset_Create( );
    DZ_API void                     DenOfIz_FontAsset_Destroy( DenOfIz_FontAsset fontAsset );
    DZ_API uint64_t                 DenOfIz_FontAsset_Magic( DenOfIz_FontAsset fontAsset );
    DZ_API uint32_t                 DenOfIz_FontAsset_Version( DenOfIz_FontAsset fontAsset );
    DZ_API uint64_t                 DenOfIz_FontAsset_NumBytes( DenOfIz_FontAsset fontAsset );
    DZ_API DenOfIz_StringView       DenOfIz_FontAsset_Path( DenOfIz_FontAsset fontAsset );
    DZ_API DenOfIz_ByteArray        DenOfIz_FontAsset_Data( DenOfIz_FontAsset fontAsset );
    DZ_API uint32_t                 DenOfIz_FontAsset_InitialFontSize( DenOfIz_FontAsset fontAsset );
    DZ_API DenOfIz_AntiAliasingMode DenOfIz_FontAsset_GetAntiAliasingMode( DenOfIz_FontAsset fontAsset );
    DZ_API uint32_t                 DenOfIz_FontAsset_AtlasWidth( DenOfIz_FontAsset fontAsset );
    DZ_API uint32_t                 DenOfIz_FontAsset_AtlasHeight( DenOfIz_FontAsset fontAsset );
    DZ_API DenOfIz_FontMetrics      DenOfIz_FontAsset_GetMetrics( DenOfIz_FontAsset fontAsset );
    DZ_API DenOfIz_FontGlyphArray   DenOfIz_FontAsset_Glyphs( DenOfIz_FontAsset fontAsset );
    DZ_API size_t                   DenOfIz_FontAsset_NumGlyphs( DenOfIz_FontAsset fontAsset );
    DZ_API DenOfIz_ByteArray        DenOfIz_FontAsset_AtlasData( DenOfIz_FontAsset fontAsset );
    DZ_API void                     DenOfIz_FontAsset_SetVersion( DenOfIz_FontAsset fontAsset, uint32_t version );
    DZ_API void                     DenOfIz_FontAsset_SetNumBytes( DenOfIz_FontAsset fontAsset, uint64_t numBytes );
    DZ_API void                     DenOfIz_FontAsset_SetPath( DenOfIz_FontAsset fontAsset, DenOfIz_StringView path );
    DZ_API void                     DenOfIz_FontAsset_SetData( DenOfIz_FontAsset fontAsset, const Byte *data, size_t numBytes );
    DZ_API void                     DenOfIz_FontAsset_SetInitialFontSize( DenOfIz_FontAsset fontAsset, uint32_t fontSize );
    DZ_API void                     DenOfIz_FontAsset_SetAntiAliasingMode( DenOfIz_FontAsset fontAsset, DenOfIz_AntiAliasingMode mode );
    DZ_API void                     DenOfIz_FontAsset_SetAtlasWidth( DenOfIz_FontAsset fontAsset, uint32_t width );
    DZ_API void                     DenOfIz_FontAsset_SetAtlasHeight( DenOfIz_FontAsset fontAsset, uint32_t height );
    DZ_API void                     DenOfIz_FontAsset_SetMetrics( DenOfIz_FontAsset fontAsset, const DenOfIz_FontMetrics *metrics );
    DZ_API void                     DenOfIz_FontAsset_SetAtlasData( DenOfIz_FontAsset fontAsset, const Byte *data, size_t numBytes );
    DZ_API void                     DenOfIz_FontAsset_AddGlyph( DenOfIz_FontAsset fontAsset, const DenOfIz_FontGlyph *glyph );
    DZ_API void                     DenOfIz_FontAsset_SetGlyphs( DenOfIz_FontAsset fontAsset, const DenOfIz_FontGlyph *glyphs, size_t count );
    DZ_API void                     DenOfIz_FontAsset_ReserveGlyphs( DenOfIz_FontAsset fontAsset, size_t capacity );
    DZ_API void                     DenOfIz_FontAsset_ClearGlyphs( DenOfIz_FontAsset fontAsset );
    DZ_API DenOfIz_StringView       DenOfIz_FontAsset_Extension( );
    DZ_API DenOfIz_FontAsset        DenOfIz_FontAsset_GetJetbrainsMono( );

#ifdef __cplusplus
}
#endif
