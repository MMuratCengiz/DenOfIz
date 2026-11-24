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

#include "DenOfIzGraphicsInternal/UI/UIText.h"

#include <harfbuzz/hb.h>
#include <bit>
#include <cmath>
#include <cstring>
#include <limits>
#include "DenOfIzGraphics/Assets/Serde/Font/FontAssetReader.h"
#include "DenOfIzGraphics/Backends/Interface/CommandList.h"
#include "DenOfIzGraphics/Backends/Interface/CommandListPool.h"
#include "DenOfIzGraphics/Backends/Interface/CommandQueue.h"
#include "DenOfIzGraphics/Support/ResourceTracking.h"
#include "DenOfIzGraphicsInternal/Assets/Font/EmbeddedFonts.h"
#include "DenOfIzGraphicsInternal/Assets/Font/Font.h"
#include "DenOfIzGraphicsInternal/Assets/Font/FontLibrary.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

using namespace DenOfIz;

namespace
{
    constexpr float    DefaultFontSize = 14.0f;
    constexpr float    WrapEpsilon     = 0.01f;
    constexpr uint32_t TabSpaces       = 4;

    uint32_t DecodeUtf8( const char *chars, const uint32_t length, const uint32_t offset )
    {
        if ( offset >= length )
        {
            return 0;
        }
        const auto b0 = static_cast<uint8_t>( chars[ offset ] );
        if ( b0 < 0x80 )
        {
            return b0;
        }
        auto cont = [ & ]( const uint32_t i ) -> uint32_t { return offset + i < length ? static_cast<uint8_t>( chars[ offset + i ] ) & 0x3F : 0; };
        if ( ( b0 & 0xE0 ) == 0xC0 )
        {
            return ( b0 & 0x1F ) << 6 | cont( 1 );
        }
        if ( ( b0 & 0xF0 ) == 0xE0 )
        {
            return ( b0 & 0x0F ) << 12 | cont( 1 ) << 6 | cont( 2 );
        }
        return ( b0 & 0x07 ) << 18 | cont( 1 ) << 12 | cont( 2 ) << 6 | cont( 3 );
    }

    bool IsCjk( const uint32_t cp )
    {
        return ( cp >= 0x2E80 && cp <= 0x9FFF ) || ( cp >= 0xAC00 && cp <= 0xD7AF ) || ( cp >= 0xF900 && cp <= 0xFAFF ) || ( cp >= 0xFF00 && cp <= 0xFFEF ) ||
               ( cp >= 0x20000 && cp <= 0x2FFFF );
    }

    bool IsSpace( const uint32_t cp )
    {
        return cp == ' ' || cp == 0x3000 || cp == 0x1680 || ( cp >= 0x2000 && cp <= 0x200A && cp != 0x2007 ) || cp == 0x205F;
    }

    bool IsBreakAfter( const uint32_t cp )
    {
        return cp == '-' || cp == 0x2010 || cp == 0x2012 || cp == 0x2013 || cp == 0x2014 || cp == 0x00AD || cp == 0x200B;
    }

    uint64_t Mix64( uint64_t h )
    {
        h ^= h >> 33;
        h *= 0xff51afd7ed558ccdULL;
        h ^= h >> 33;
        h *= 0xc4ceb9fe1a85ec53ULL;
        h ^= h >> 33;
        return h;
    }

    uint64_t HashParams( const UITextParams &p )
    {
        uint64_t h = p.FontId;
        h          = Mix64( h ^ std::bit_cast<uint32_t>( p.FontSize ) );
        h          = Mix64( h ^ std::bit_cast<uint32_t>( p.LetterSpacing ) );
        h          = Mix64( h ^ std::bit_cast<uint32_t>( p.LineHeight ) );
        h          = Mix64( h ^ ( static_cast<uint64_t>( p.Wrap ) | static_cast<uint64_t>( p.Overflow ) << 8 | static_cast<uint64_t>( p.MaxLines ) << 16 ) );
        return h;
    }

    float GlyphWidth( const UIGlyph &glyph, const float scale, const float spacing, const float spaceAdvance, const bool newlineAsSpace )
    {
        if ( glyph.Flags & UIGlyphFlags::Newline )
        {
            return newlineAsSpace ? spaceAdvance * scale + spacing : 0.0f;
        }
        return glyph.Advance * scale + spacing;
    }
} // namespace

bool UITextParams::operator==( const UITextParams &other ) const
{
    return FontId == other.FontId && FontSize == other.FontSize && LetterSpacing == other.LetterSpacing && LineHeight == other.LineHeight && Wrap == other.Wrap &&
           Overflow == other.Overflow && MaxLines == other.MaxLines;
}

UITextParams UITextParams::From( const DenOfIz_UITextDesc &desc )
{
    UITextParams params;
    params.FontId        = desc.FontId;
    params.FontSize      = desc.FontSize > 0.0f ? desc.FontSize : DefaultFontSize;
    params.LetterSpacing = desc.LetterSpacing;
    params.LineHeight    = desc.LineHeight > 0.0f ? desc.LineHeight : 0.0f;
    params.Wrap          = desc.Wrap;
    params.Overflow      = desc.Overflow;
    params.MaxLines      = desc.MaxLines;
    return params;
}

bool UITextLayoutKey::operator==( const UITextLayoutKey &other ) const
{
    return Run == other.Run && MaxWidth == other.MaxWidth && Params == other.Params;
}

size_t UITextLayoutKeyHash::operator( )( const UITextLayoutKey &key ) const
{
    return Mix64( reinterpret_cast<uintptr_t>( key.Run ) ^ HashParams( key.Params ) ^ static_cast<uint64_t>( std::bit_cast<uint32_t>( key.MaxWidth ) ) << 32 );
}

bool UIRunKey::operator==( const UIRunKey &other ) const
{
    return Hash == other.Hash && Length == other.Length && FontId == other.FontId && Generation == other.Generation;
}

size_t UIRunKeyHash::operator( )( const UIRunKey &key ) const
{
    return Mix64( key.Hash ^ static_cast<uint64_t>( key.Length ) << 20 ^ static_cast<uint64_t>( key.FontId ) << 48 ^ key.Generation );
}

float UITextLayout::AlignOffset( const uint32_t line, const float boxWidth, const DenOfIz_UITextAlign align ) const
{
    if ( line >= Lines.size( ) || align == DENOFIZ_UI_TEXT_ALIGN_LEFT )
    {
        return 0.0f;
    }
    const float free = boxWidth - Lines[ line ].Width;
    if ( free <= 0.0f )
    {
        return 0.0f;
    }
    return align == DENOFIZ_UI_TEXT_ALIGN_CENTER ? std::floor( free * 0.5f * 64.0f ) / 64.0f : free;
}

float UITextLayout::CaretX( const uint32_t line, const uint32_t byteOffset ) const
{
    if ( line >= Lines.size( ) )
    {
        return 0.0f;
    }
    const UITextLineLayout &l = Lines[ line ];
    for ( uint32_t i = l.GlyphStart; i < l.GlyphEnd; ++i )
    {
        if ( Run->Glyphs[ i ].Cluster >= byteOffset )
        {
            return GlyphX[ i ];
        }
    }
    if ( l.GlyphEnd == l.GlyphStart )
    {
        return 0.0f;
    }
    const uint32_t last = l.GlyphEnd - 1;
    return GlyphX[ last ] + GlyphWidth( Run->Glyphs[ last ], Scale, Params.LetterSpacing, Face->SpaceAdvance, Params.Wrap == DENOFIZ_UI_TEXT_WRAP_NONE );
}

uint32_t UITextLayout::LineOfOffset( const uint32_t byteOffset ) const
{
    for ( uint32_t i = 0; i + 1 < Lines.size( ); ++i )
    {
        if ( byteOffset < Lines[ i + 1 ].ByteStart )
        {
            return i;
        }
    }
    return Lines.empty( ) ? 0 : static_cast<uint32_t>( Lines.size( ) - 1 );
}

uint32_t UITextLayout::OffsetAt( const float x, const float y, const float boxWidth, const DenOfIz_UITextAlign align ) const
{
    if ( Lines.empty( ) )
    {
        return 0;
    }
    const float    lineF = LineHeight > 0.0f ? std::floor( y / LineHeight ) : 0.0f;
    const uint32_t line  = static_cast<uint32_t>( std::clamp( lineF, 0.0f, static_cast<float>( Lines.size( ) - 1 ) ) );
    const auto    &l     = Lines[ line ];
    const float    lx    = x - AlignOffset( line, boxWidth, align );
    for ( uint32_t i = l.GlyphStart; i < l.GlyphEnd; ++i )
    {
        const UIGlyph &glyph = Run->Glyphs[ i ];
        const float    w     = GlyphWidth( glyph, Scale, Params.LetterSpacing, Face->SpaceAdvance, Params.Wrap == DENOFIZ_UI_TEXT_WRAP_NONE );
        if ( lx < GlyphX[ i ] + w * 0.5f )
        {
            return glyph.Cluster;
        }
    }
    const bool softBreak = line + 1 < Lines.size( ) && Lines[ line + 1 ].ByteStart == l.ByteEnd && l.GlyphEnd > l.GlyphStart;
    return softBreak ? Run->Glyphs[ l.GlyphEnd - 1 ].Cluster : l.ByteEnd;
}

const std::vector<UIGlyphQuad> &UITextLayout::GetQuads( ) const
{
    if ( QuadsBuilt )
    {
        return Quads;
    }
    QuadsBuilt = true;
    Quads.clear( );
    Quads.reserve( Run->Glyphs.size( ) );

    const float invW = 1.0f / Face->AtlasWidth;
    const float invH = 1.0f / Face->AtlasHeight;
    auto        addGlyph = [ & ]( const UIGlyph &glyph, const float penX, const uint32_t line )
    {
        const DenOfIz_FontGlyph *g = glyph.Glyph;
        if ( g == nullptr || g->Width == 0 || g->Height == 0 )
        {
            return;
        }
        UIGlyphQuad quad{ };
        quad.X0   = penX + ( glyph.OffsetX + g->BearingX ) * Scale;
        quad.Y0   = -( g->BearingY + glyph.OffsetY ) * Scale;
        quad.X1   = quad.X0 + static_cast<float>( g->Width ) * Scale;
        quad.Y1   = quad.Y0 + static_cast<float>( g->Height ) * Scale;
        quad.U0   = static_cast<float>( g->AtlasX ) * invW;
        quad.V0   = static_cast<float>( g->AtlasY ) * invH;
        quad.U1   = static_cast<float>( g->AtlasX + g->Width ) * invW;
        quad.V1   = static_cast<float>( g->AtlasY + g->Height ) * invH;
        quad.Line = line;
        Quads.push_back( quad );
    };

    for ( uint32_t line = 0; line < Lines.size( ); ++line )
    {
        const UITextLineLayout &l = Lines[ line ];
        for ( uint32_t i = l.GlyphStart; i < l.GlyphEnd; ++i )
        {
            addGlyph( Run->Glyphs[ i ], GlyphX[ i ], line );
        }
        if ( l.Ellipsis )
        {
            float pen = l.Width - Face->EllipsisAdvance * Scale - Params.LetterSpacing * static_cast<float>( Face->Ellipsis.size( ) );
            for ( const UIGlyph &glyph : Face->Ellipsis )
            {
                addGlyph( glyph, pen, line );
                pen += glyph.Advance * Scale + Params.LetterSpacing;
            }
        }
    }
    return Quads;
}

UIText::UIText( const DenOfIz_LogicalDevice device ) : m_device( device )
{
    m_buffer = hb_buffer_create( );

    static FontLibrary defaultFontLibrary;
    static Font       *defaultFont = defaultFontLibrary.LoadFont( { EmbeddedFonts::GetJetbrainsMono( ) } );
    AddFont( 0, defaultFont );
}

UIText::~UIText( )
{
    for ( UIFontFace &face : m_faces )
    {
        ReleaseFace( face );
    }
    if ( m_buffer )
    {
        hb_buffer_destroy( m_buffer );
    }
}

void UIText::AddFont( const uint16_t fontId, Font *font )
{
    if ( fontId >= m_faces.size( ) )
    {
        spdlog::error( "UI: font id {} is out of range, at most {} fonts are supported", fontId, m_faces.size( ) );
        return;
    }
    if ( font == nullptr )
    {
        spdlog::error( "UI: AddFont called with a null font" );
        return;
    }
    RemoveFont( fontId );
    LoadFace( m_faces[ fontId ], font );
}

void UIText::RemoveFont( const uint16_t fontId )
{
    if ( fontId >= m_faces.size( ) || m_faces[ fontId ].FontPtr == nullptr )
    {
        return;
    }
    std::erase_if( m_layouts, [ fontId ]( const auto &entry ) { return entry.first.Run->FontId == fontId; } );
    std::erase_if( m_runs, [ fontId ]( const auto &entry ) { return entry.first.FontId == fontId; } );
    ++m_epoch;
    ReleaseFace( m_faces[ fontId ] );
}

const UIFontFace *UIText::GetFace( const uint16_t fontId ) const
{
    return &m_faces[ ResolveFontId( fontId ) ];
}

DenOfIz_Texture UIText::GetAtlas( const uint32_t slot ) const
{
    return slot < m_faces.size( ) ? m_faces[ slot ].Atlas : DENOFIZ_NULL_HANDLE;
}

uint16_t UIText::ResolveFontId( const uint16_t fontId ) const
{
    if ( fontId < m_faces.size( ) && m_faces[ fontId ].FontPtr != nullptr )
    {
        return fontId;
    }
    return 0;
}

uint64_t UIText::Hash( const char *chars, const size_t length )
{
    uint64_t hash = 14695981039346656037ULL;
    for ( size_t i = 0; i < length; ++i )
    {
        hash ^= static_cast<uint8_t>( chars[ i ] );
        hash *= 1099511628211ULL;
    }
    return hash;
}

void UIText::LoadFace( UIFontFace &face, Font *font )
{
    face.FontPtr = font;
    face.Asset   = font->Asset( );

    const DenOfIz_ByteArray data = DenOfIz_FontAsset_Data( face.Asset );
    face.Blob   = hb_blob_create( reinterpret_cast<const char *>( data.Elements ), static_cast<unsigned int>( data.NumElements ), HB_MEMORY_MODE_READONLY, nullptr, nullptr );
    face.Face   = hb_face_create( face.Blob, 0 );
    face.HbFont = hb_font_create( face.Face );

    const uint32_t initialSize = DenOfIz_FontAsset_InitialFontSize( face.Asset );
    face.BaseSize              = initialSize > 0 ? static_cast<float>( initialSize ) : 32.0f;
    const int scale            = static_cast<int>( std::lround( face.BaseSize * 64.0f ) );
    hb_font_set_scale( face.HbFont, scale, scale );

    const DenOfIz_FontMetrics metrics = DenOfIz_FontAsset_GetMetrics( face.Asset );
    face.Ascent                       = metrics.Ascent;
    face.Descent                      = metrics.Descent;
    face.LineHeight                   = std::max( metrics.LineHeight, metrics.Ascent + metrics.Descent );
    face.AtlasWidth                   = static_cast<float>( std::max( 1u, DenOfIz_FontAsset_AtlasWidth( face.Asset ) ) );
    face.AtlasHeight                  = static_cast<float>( std::max( 1u, DenOfIz_FontAsset_AtlasHeight( face.Asset ) ) );

    hb_codepoint_t spaceGlyph = 0;
    face.SpaceAdvance         = hb_font_get_nominal_glyph( face.HbFont, ' ', &spaceGlyph ) ? static_cast<float>( hb_font_get_glyph_h_advance( face.HbFont, spaceGlyph ) ) / 64.0f
                                                                                           : face.BaseSize * 0.3f;

    face.Generation = ++m_generation;

    ShapeInto( face, "\xE2\x80\xA6", 3, face.Ellipsis );
    if ( face.Ellipsis.empty( ) || face.Ellipsis[ 0 ].Glyph == nullptr )
    {
        ShapeInto( face, "...", 3, face.Ellipsis );
    }
    face.EllipsisAdvance = 0.0f;
    for ( const UIGlyph &glyph : face.Ellipsis )
    {
        face.EllipsisAdvance += glyph.Advance;
    }

    UploadAtlas( face );
}

void UIText::ReleaseFace( UIFontFace &face )
{
    if ( DENOFIZ_HANDLE_IS_VALID( face.Atlas ) )
    {
        DenOfIz_LogicalDevice_WaitIdle( m_device );
        DenOfIz_Texture_Destroy( face.Atlas );
    }
    if ( face.HbFont )
    {
        hb_font_destroy( face.HbFont );
    }
    if ( face.Face )
    {
        hb_face_destroy( face.Face );
    }
    if ( face.Blob )
    {
        hb_blob_destroy( face.Blob );
    }
    face = UIFontFace{ };
}

void UIText::UploadAtlas( UIFontFace &face )
{
    const DenOfIz_FontAsset fontAsset = face.Asset;

    DenOfIz_TextureDesc textureDesc{ };
    textureDesc.Width     = DenOfIz_FontAsset_AtlasWidth( fontAsset );
    textureDesc.Height    = DenOfIz_FontAsset_AtlasHeight( fontAsset );
    textureDesc.Format    = DENOFIZ_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Usage     = DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT | DENOFIZ_TEXTURE_USAGE_COPY_DST_BIT;
    textureDesc.HeapType  = DENOFIZ_HEAP_TYPE_GPU;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Depth     = 1;
    textureDesc.DebugName = DENOFIZ_STRING( "UI Font Atlas" );
    DenOfIz_LogicalDevice_CreateTexture( m_device, &textureDesc, &face.Atlas );

    const DenOfIz_ByteArray atlasData = DenOfIz_FontAsset_AtlasData( fontAsset );
    if ( atlasData.NumElements == 0 )
    {
        return;
    }

    DenOfIz_CommandQueueDesc commandQueueDesc{ };
    commandQueueDesc.QueueType = DENOFIZ_QUEUE_TYPE_GRAPHICS;
    DenOfIz_CommandQueue commandQueue;
    DenOfIz_LogicalDevice_CreateCommandQueue( m_device, &commandQueueDesc, &commandQueue );

    DenOfIz_CommandListPoolDesc commandListPoolDesc{ };
    commandListPoolDesc.CommandQueue    = commandQueue;
    commandListPoolDesc.NumCommandLists = 1;
    DenOfIz_CommandListPool commandListPool;
    DenOfIz_LogicalDevice_CreateCommandListPool( m_device, &commandListPoolDesc, &commandListPool );
    DenOfIz_CommandListArray commandLists;
    DenOfIz_CommandListPool_GetCommandLists( commandListPool, &commandLists );
    DenOfIz_CommandList commandList = commandLists.Elements[ 0 ];
    DenOfIz_CommandList_Begin( commandList );

    DenOfIz_PhysicalDevice deviceInfo;
    DenOfIz_LogicalDevice_DeviceInfo( m_device, &deviceInfo );
    const auto alignedPitch = Utilities::Align( DenOfIz_FontAsset_AtlasWidth( fontAsset ) * DENOFIZ_FONT_ASSET_NUM_CHANNELS, deviceInfo.Constants.BufferTextureRowAlignment );
    const auto alignedSlice = Utilities::Align( DenOfIz_FontAsset_AtlasHeight( fontAsset ), deviceInfo.Constants.BufferTextureAlignment );

    DenOfIz_BufferDesc stagingDesc{ };
    stagingDesc.NumBytes  = alignedPitch * alignedSlice;
    stagingDesc.Usage     = DENOFIZ_BUFFER_USAGE_COPY_SRC_BIT;
    stagingDesc.DebugName = DENOFIZ_STRING( "UI Font Atlas Staging" );
    stagingDesc.HeapType  = DENOFIZ_HEAP_TYPE_CPU;
    DenOfIz_Buffer stagingBuffer;
    DenOfIz_LogicalDevice_CreateBuffer( m_device, &stagingDesc, &stagingBuffer );

    DenOfIz_ResourceTracking resourceTracking;
    DenOfIz_ResourceTracking_Create( &resourceTracking );
    DenOfIz_ResourceTracking_TrackTexture( resourceTracking, face.Atlas, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    DenOfIz_ResourceTracking_TrackBuffer( resourceTracking, stagingBuffer, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_LoadAtlasIntoGpuTextureDesc loadDesc{ };
    loadDesc.Device        = m_device;
    loadDesc.StagingBuffer = stagingBuffer;
    loadDesc.CommandList   = commandList;
    loadDesc.Texture       = face.Atlas;
    DenOfIz_FontAssetReader_LoadAtlasIntoGpuTexture( fontAsset, &loadDesc );

    DenOfIz_ResourceTracking_TransitionTexture( resourceTracking, commandList, face.Atlas, DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_Format format;
    DenOfIz_Texture_GetFormat( face.Atlas, &format );

    DenOfIz_CopyBufferToTextureDesc copyDesc{ };
    copyDesc.SrcBuffer  = stagingBuffer;
    copyDesc.DstTexture = face.Atlas;
    copyDesc.RowPitch   = DenOfIz_FontAsset_AtlasWidth( fontAsset ) * DENOFIZ_FONT_ASSET_NUM_CHANNELS;
    copyDesc.Format     = format;
    DenOfIz_CommandList_CopyBufferToTexture( commandList, &copyDesc );

    DenOfIz_ResourceTracking_TransitionTexture( resourceTracking, commandList, face.Atlas, DENOFIZ_RESOURCE_USAGE_SHADER_RESOURCE_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_CommandList_End( commandList );
    DenOfIz_ExecuteCommandListsDesc executeDesc{ };
    executeDesc.CommandLists.Elements    = &commandList;
    executeDesc.CommandLists.NumElements = 1;
    DenOfIz_CommandQueue_ExecuteCommandLists( commandQueue, &executeDesc );
    DenOfIz_CommandQueue_WaitIdle( commandQueue );

    DenOfIz_ResourceTracking_Destroy( resourceTracking );
    DenOfIz_Buffer_Destroy( stagingBuffer );
    DenOfIz_CommandListPool_Destroy( commandListPool );
    DenOfIz_CommandQueue_Destroy( commandQueue );
}

void UIText::ShapeInto( const UIFontFace &face, const char *chars, const uint32_t length, std::vector<UIGlyph> &outGlyphs )
{
    outGlyphs.clear( );
    if ( length == 0 || face.HbFont == nullptr )
    {
        return;
    }

    hb_buffer_clear_contents( m_buffer );
    hb_buffer_add_utf8( m_buffer, chars, static_cast<int>( length ), 0, static_cast<int>( length ) );
    hb_buffer_set_direction( m_buffer, HB_DIRECTION_LTR );
    hb_buffer_guess_segment_properties( m_buffer );

    static constexpr hb_feature_t features[] = {
        { HB_TAG( 'k', 'e', 'r', 'n' ), 1, 0, UINT_MAX }, { HB_TAG( 'l', 'i', 'g', 'a' ), 0, 0, UINT_MAX }, { HB_TAG( 'c', 'l', 'i', 'g' ), 0, 0, UINT_MAX },
        { HB_TAG( 'd', 'l', 'i', 'g' ), 0, 0, UINT_MAX }, { HB_TAG( 'c', 'a', 'l', 't' ), 0, 0, UINT_MAX },
    };
    hb_shape( face.HbFont, m_buffer, features, std::size( features ) );

    unsigned int               count     = 0;
    const hb_glyph_info_t     *infos     = hb_buffer_get_glyph_infos( m_buffer, &count );
    const hb_glyph_position_t *positions = hb_buffer_get_glyph_positions( m_buffer, &count );
    if ( infos == nullptr || positions == nullptr )
    {
        return;
    }

    const auto &glyphIndexMap = face.FontPtr->GetGlyphIndexMap( );
    outGlyphs.resize( count );
    for ( unsigned int i = 0; i < count; ++i )
    {
        UIGlyph &glyph = outGlyphs[ i ];
        glyph.Cluster  = infos[ i ].cluster;
        glyph.Advance  = static_cast<float>( positions[ i ].x_advance ) / 64.0f;
        glyph.OffsetX  = static_cast<float>( positions[ i ].x_offset ) / 64.0f;
        glyph.OffsetY  = static_cast<float>( positions[ i ].y_offset ) / 64.0f;
        glyph.Flags    = 0;
        glyph.Glyph    = nullptr;

        const uint32_t codePoint = DecodeUtf8( chars, length, glyph.Cluster );
        if ( codePoint == '\n' )
        {
            glyph.Flags   = UIGlyphFlags::Newline;
            glyph.Advance = 0.0f;
            continue;
        }
        if ( codePoint == '\r' )
        {
            glyph.Advance = 0.0f;
            continue;
        }
        if ( codePoint == '\t' )
        {
            glyph.Flags   = UIGlyphFlags::Tab | UIGlyphFlags::Space | UIGlyphFlags::BreakAfter;
            glyph.Advance = face.SpaceAdvance * TabSpaces;
            continue;
        }
        if ( IsSpace( codePoint ) )
        {
            glyph.Flags = UIGlyphFlags::Space | UIGlyphFlags::BreakAfter;
            continue;
        }

        const auto mapped = glyphIndexMap.find( infos[ i ].codepoint );
        glyph.Glyph       = face.FontPtr->GetGlyph( mapped != glyphIndexMap.end( ) ? mapped->second : codePoint );
        if ( IsBreakAfter( codePoint ) )
        {
            glyph.Flags |= UIGlyphFlags::BreakAfter;
        }
        if ( IsCjk( codePoint ) )
        {
            glyph.Flags |= UIGlyphFlags::BreakAfter;
            if ( i > 0 )
            {
                outGlyphs[ i - 1 ].Flags |= UIGlyphFlags::BreakAfter;
            }
        }
    }
}

const UIShapedRun *UIText::Shape( const uint16_t fontId, const char *chars, const uint32_t length, const uint64_t hash )
{
    const uint16_t    resolved = ResolveFontId( fontId );
    const UIFontFace &face     = m_faces[ resolved ];
    UIRunKey          key{ hash, length, resolved, face.Generation };

    for ( auto it = m_runs.find( key ); it != m_runs.end( ); it = m_runs.find( key ) )
    {
        if ( length == 0 || std::memcmp( it->second->Text.data( ), chars, length ) == 0 )
        {
            it->second->LastUsedFrame = m_frame;
            return it->second.get( );
        }
        key.Hash = Mix64( key.Hash + 0x9E3779B97F4A7C15ULL );
    }

    auto run           = std::make_unique<UIShapedRun>( );
    run->FontId        = resolved;
    run->Generation    = face.Generation;
    run->Hash          = hash;
    run->LastUsedFrame = m_frame;
    run->Text.assign( chars, length );
    ShapeInto( face, run->Text.data( ), length, run->Glyphs );

    float    word       = 0.0f;
    uint32_t wordGlyphs = 0;
    for ( const UIGlyph &glyph : run->Glyphs )
    {
        if ( glyph.Flags & ( UIGlyphFlags::Space | UIGlyphFlags::Newline ) )
        {
            word       = 0.0f;
            wordGlyphs = 0;
            continue;
        }
        word += glyph.Advance;
        ++wordGlyphs;
        if ( word > run->LongestWord )
        {
            run->LongestWord       = word;
            run->LongestWordGlyphs = wordGlyphs;
        }
        if ( glyph.Flags & UIGlyphFlags::BreakAfter )
        {
            word       = 0.0f;
            wordGlyphs = 0;
        }
    }

    const UIShapedRun *result = run.get( );
    m_runs.emplace( key, std::move( run ) );
    return result;
}

const UITextLayout *UIText::Layout( const UIShapedRun *run, const UITextParams &params, const float maxWidth )
{
    const float width = maxWidth >= 0.0f && std::isfinite( maxWidth ) ? maxWidth : -1.0f;
    if ( width >= 0.0f )
    {
        const UITextLayout *natural = Layout( run, params, -1.0f );
        if ( natural->Width <= width + WrapEpsilon )
        {
            return natural;
        }
    }
    const UITextLayoutKey key{ run, params, width };
    run->LastUsedFrame = m_frame;
    if ( const auto it = m_layouts.find( key ); it != m_layouts.end( ) )
    {
        it->second->LastUsedFrame = m_frame;
        return it->second.get( );
    }

    auto layout           = std::make_unique<UITextLayout>( );
    layout->Run           = run;
    layout->Face          = &m_faces[ run->FontId ];
    layout->Params        = params;
    layout->MaxWidth      = width;
    layout->LastUsedFrame = m_frame;
    layout->TextureSlot   = UITextureSlots::FirstFont + run->FontId;
    BuildLayout( *layout );

    const UITextLayout *result = layout.get( );
    m_layouts[ key ]           = std::move( layout );
    return result;
}

void UIText::BuildLayout( UITextLayout &layout ) const
{
    const UIShapedRun  &run     = *layout.Run;
    const UIFontFace   &face    = *layout.Face;
    const UITextParams &params  = layout.Params;
    const float         scale   = params.FontSize / face.BaseSize;
    const float         spacing = params.LetterSpacing;
    const bool          newlineAsSpace = params.Wrap == DENOFIZ_UI_TEXT_WRAP_NONE;
    const bool          canWrap = ( params.Wrap == DENOFIZ_UI_TEXT_WRAP_WORDS || params.Wrap == DENOFIZ_UI_TEXT_WRAP_ANYWHERE ) && layout.MaxWidth >= 0.0f;
    const float         limit   = layout.MaxWidth + WrapEpsilon;

    layout.Scale      = scale;
    layout.LineHeight = params.LineHeight > 0.0f ? params.LineHeight : face.LineHeight * scale;
    layout.Baseline   = ( layout.LineHeight - ( face.Ascent + face.Descent ) * scale ) * 0.5f + face.Ascent * scale;

    const auto    &glyphs = run.Glyphs;
    const uint32_t count  = static_cast<uint32_t>( glyphs.size( ) );
    const uint32_t bytes  = static_cast<uint32_t>( run.Text.size( ) );
    layout.GlyphX.assign( count, 0.0f );
    layout.Lines.clear( );

    auto byteAt = [ & ]( const uint32_t glyph ) { return glyph < count ? glyphs[ glyph ].Cluster : bytes; };
    auto emit   = [ & ]( const uint32_t start, const uint32_t end, const float width )
    { layout.Lines.push_back( UITextLineLayout{ start, end, byteAt( start ), byteAt( end ), width, false } ); };

    uint32_t lineStart    = 0;
    float    x            = 0.0f;
    float    visibleEnd   = 0.0f;
    int64_t  breakAt      = -1;
    float    breakVisible = 0.0f;
    float    breakX       = 0.0f;

    for ( uint32_t i = 0; i < count; )
    {
        const UIGlyph &glyph = glyphs[ i ];
        if ( ( glyph.Flags & UIGlyphFlags::Newline ) && !newlineAsSpace )
        {
            layout.GlyphX[ i ] = x;
            emit( lineStart, i, visibleEnd );
            lineStart  = i + 1;
            x          = 0.0f;
            visibleEnd = 0.0f;
            breakAt    = -1;
            ++i;
            continue;
        }

        const float w     = GlyphWidth( glyph, scale, spacing, face.SpaceAdvance, newlineAsSpace );
        const bool  space = ( glyph.Flags & ( UIGlyphFlags::Space | UIGlyphFlags::Newline ) ) != 0;
        if ( canWrap && !space && i > lineStart && w > 0.0f && glyphs[ i ].Cluster != glyphs[ i - 1 ].Cluster && x + w > limit )
        {
            if ( breakAt >= static_cast<int64_t>( lineStart ) )
            {
                emit( lineStart, static_cast<uint32_t>( breakAt + 1 ), breakVisible );
                for ( uint32_t j = static_cast<uint32_t>( breakAt + 1 ); j < i; ++j )
                {
                    layout.GlyphX[ j ] -= breakX;
                }
                x -= breakX;
                visibleEnd = i > static_cast<uint32_t>( breakAt + 1 ) ? visibleEnd - breakX : 0.0f;
                lineStart  = static_cast<uint32_t>( breakAt + 1 );
                breakAt    = -1;
                continue;
            }
            emit( lineStart, i, visibleEnd );
            lineStart  = i;
            x          = 0.0f;
            visibleEnd = 0.0f;
            breakAt    = -1;
            continue;
        }

        layout.GlyphX[ i ] = x;
        x += w;
        if ( !space )
        {
            visibleEnd = x;
        }
        if ( glyph.Flags & UIGlyphFlags::BreakAfter )
        {
            breakAt      = i;
            breakVisible = visibleEnd;
            breakX       = x;
        }
        ++i;
    }
    emit( lineStart, count, visibleEnd );

    bool truncated = false;
    if ( params.MaxLines > 0 && layout.Lines.size( ) > params.MaxLines )
    {
        layout.Lines.resize( params.MaxLines );
        truncated = true;
    }

    if ( params.Overflow == DENOFIZ_UI_TEXT_OVERFLOW_ELLIPSIS )
    {
        const float ellipsis = face.EllipsisAdvance * scale + spacing * static_cast<float>( face.Ellipsis.size( ) );
        for ( size_t lineIndex = 0; lineIndex < layout.Lines.size( ); ++lineIndex )
        {
            UITextLineLayout &line   = layout.Lines[ lineIndex ];
            const bool        isLast = lineIndex + 1 == layout.Lines.size( );
            const bool        overflows = layout.MaxWidth >= 0.0f && line.Width > limit;
            if ( !overflows && !( truncated && isLast ) )
            {
                continue;
            }
            const float fit = layout.MaxWidth >= 0.0f ? layout.MaxWidth - ellipsis : std::numeric_limits<float>::max( );
            float       width = line.Width;
            while ( line.GlyphEnd > line.GlyphStart )
            {
                const uint32_t last      = line.GlyphEnd - 1;
                const bool     lastSpace = ( glyphs[ last ].Flags & ( UIGlyphFlags::Space | UIGlyphFlags::Newline ) ) != 0;
                width                    = layout.GlyphX[ last ] + GlyphWidth( glyphs[ last ], scale, spacing, face.SpaceAdvance, newlineAsSpace );
                if ( !lastSpace && width <= fit )
                {
                    break;
                }
                --line.GlyphEnd;
            }
            if ( line.GlyphEnd == line.GlyphStart )
            {
                width = 0.0f;
            }
            line.ByteEnd  = byteAt( line.GlyphEnd );
            line.Width    = width + ellipsis;
            line.Ellipsis = true;
        }
    }

    layout.Width = 0.0f;
    for ( const UITextLineLayout &line : layout.Lines )
    {
        layout.Width = std::max( layout.Width, line.Width );
    }
    layout.Height = layout.LineHeight * static_cast<float>( layout.Lines.size( ) );
}

float UIText::MinContentWidth( const UIShapedRun *run, const UITextParams &params ) const
{
    if ( params.Overflow == DENOFIZ_UI_TEXT_OVERFLOW_ELLIPSIS || params.Wrap == DENOFIZ_UI_TEXT_WRAP_ANYWHERE )
    {
        return 0.0f;
    }
    const UIFontFace &face  = m_faces[ run->FontId ];
    const float       scale = params.FontSize / face.BaseSize;
    if ( params.Wrap == DENOFIZ_UI_TEXT_WRAP_WORDS )
    {
        return run->LongestWord * scale + params.LetterSpacing * static_cast<float>( run->LongestWordGlyphs );
    }
    return -1.0f;
}

DenOfIz_Float2 UIText::Measure( const char *chars, const uint32_t length, const DenOfIz_UITextDesc &desc, const float maxWidth )
{
    const UITextParams  params = UITextParams::From( desc );
    const UIShapedRun  *run    = Shape( params.FontId, chars, length, Hash( chars, length ) );
    const UITextLayout *layout = Layout( run, params, maxWidth > 0.0f ? maxWidth : -1.0f );
    return DenOfIz_Float2{ layout->Width, layout->Height };
}

void UIText::BeginFrame( const uint64_t frame )
{
    m_frame = frame;
}

void UIText::Evict( const uint64_t maxAge )
{
    const uint64_t frame   = m_frame;
    const uint64_t layoutAge = std::max<uint64_t>( 2, maxAge / 4 );
    const size_t   removed = std::erase_if( m_layouts, [ frame, layoutAge ]( const auto &entry ) { return frame - entry.second->LastUsedFrame > layoutAge; } ) +
                           std::erase_if( m_runs, [ frame, maxAge ]( const auto &entry ) { return frame - entry.second->LastUsedFrame > maxAge; } );
    if ( removed > 0 )
    {
        ++m_epoch;
    }
}

uint64_t UIText::Epoch( ) const
{
    return m_epoch;
}

uint64_t UIText::Frame( ) const
{
    return m_frame;
}
