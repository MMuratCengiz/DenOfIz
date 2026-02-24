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

#include "DenOfIzGraphics/Assets/Serde/Font/FontAssetWriter.h"
#include "DenOfIzGraphicsInternal/Assets/Serde/Common/AssetWriterHelpers.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#define FONT_ASSET_WRITER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::FontAssetWriter, handle )

namespace DenOfIz
{
    class FontAssetWriter
    {
    public:
        DenOfIz_BinaryWriter m_writer;
        uint64_t             m_dataOffset          = 0;
        uint64_t             m_streamStartLocation = 0;
        DenOfIz_FontAsset    m_fontAsset           = DENOFIZ_NULL_HANDLE;

        explicit FontAssetWriter( const DenOfIz_FontAssetWriterDesc &desc );
        ~FontAssetWriter( );

        void Write( DenOfIz_FontAsset fontAsset );
        void End( ) const;

    private:
        void WriteHeader( uint64_t totalNumBytes ) const;
        void WriteMetadata( DenOfIz_FontAsset fontAsset ) const;
        void WriteGlyph( DenOfIz_FontAsset fontAsset ) const;
    };
} // namespace DenOfIz

using namespace DenOfIz;

FontAssetWriter::FontAssetWriter( const DenOfIz_FontAssetWriterDesc &desc ) : m_writer( desc.Writer )
{
    if ( !DENOFIZ_HANDLE_IS_VALID( m_writer ) )
    {
        spdlog::critical( "BinaryWriter cannot be null for FontAssetWriter" );
    };
}

FontAssetWriter::~FontAssetWriter( ) = default;

void FontAssetWriter::Write( DenOfIz_FontAsset fontAsset )
{
    m_fontAsset           = fontAsset;
    m_streamStartLocation = DenOfIz_BinaryWriter_Position( m_writer );

    WriteHeader( 0 );
    WriteMetadata( fontAsset );
    WriteGlyph( fontAsset );

    DenOfIz_ByteArray atlasData = DenOfIz_FontAsset_AtlasData( fontAsset );
    DenOfIz_BinaryWriter_WriteUInt64( m_writer, atlasData.NumElements );
    DenOfIz_BinaryWriter_WriteBytes( m_writer, { atlasData.Elements, atlasData.NumElements } );
}

void FontAssetWriter::End( ) const
{
    const auto totalNumBytes   = DenOfIz_BinaryWriter_Position( m_writer ) - m_streamStartLocation;
    const auto currentPosition = DenOfIz_BinaryWriter_Position( m_writer );
    DenOfIz_BinaryWriter_Seek( m_writer, m_streamStartLocation );
    WriteHeader( totalNumBytes );
    DenOfIz_BinaryWriter_Seek( m_writer, currentPosition );
}

void FontAssetWriter::WriteHeader( const uint64_t totalNumBytes ) const
{
    DenOfIz_BinaryWriter_WriteUInt64( m_writer, DenOfIz_FontAsset_Magic( m_fontAsset ) );
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, DenOfIz_FontAsset_Version( m_fontAsset ) );
    DenOfIz_BinaryWriter_WriteUInt64( m_writer, totalNumBytes );
    const DenOfIz_StringView path = DenOfIz_FontAsset_Path( m_fontAsset );
    DenOfIz_BinaryWriter_WriteString( m_writer, path );
}

void FontAssetWriter::WriteMetadata( DenOfIz_FontAsset fontAsset ) const
{
    const DenOfIz_ByteArray data = DenOfIz_FontAsset_Data( fontAsset );
    DenOfIz_BinaryWriter_WriteUInt64( m_writer, data.NumElements );
    DenOfIz_BinaryWriter_WriteBytes( m_writer, { data.Elements, data.NumElements } );

    DenOfIz_BinaryWriter_WriteUInt32( m_writer, DenOfIz_FontAsset_InitialFontSize( fontAsset ) );
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, DenOfIz_FontAsset_AtlasWidth( fontAsset ) );
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, DenOfIz_FontAsset_AtlasHeight( fontAsset ) );

    const DenOfIz_FontMetrics metrics = DenOfIz_FontAsset_GetMetrics( fontAsset );
    DenOfIz_BinaryWriter_WriteFloat( m_writer, metrics.Ascent );
    DenOfIz_BinaryWriter_WriteFloat( m_writer, metrics.Descent );
    DenOfIz_BinaryWriter_WriteFloat( m_writer, metrics.LineGap );
    DenOfIz_BinaryWriter_WriteFloat( m_writer, metrics.LineHeight );
    DenOfIz_BinaryWriter_WriteFloat( m_writer, metrics.UnderlinePos );
    DenOfIz_BinaryWriter_WriteFloat( m_writer, metrics.UnderlineThickness );
}

void FontAssetWriter::WriteGlyph( DenOfIz_FontAsset fontAsset ) const
{
    const DenOfIz_FontGlyphArray glyphs    = DenOfIz_FontAsset_Glyphs( fontAsset );
    const uint32_t               numGlyphs = static_cast<uint32_t>( glyphs.NumElements );
    DenOfIz_BinaryWriter_WriteUInt32( m_writer, numGlyphs );
    for ( uint32_t i = 0; i < numGlyphs; ++i )
    {
        const DenOfIz_FontGlyph &glyph = glyphs.Elements[ i ];
        DenOfIz_BinaryWriter_WriteUInt32( m_writer, glyph.CodePoint );
        DenOfIz_BinaryWriter_WriteDouble( m_writer, glyph.Bounds.XMin );
        DenOfIz_BinaryWriter_WriteDouble( m_writer, glyph.Bounds.YMin );
        DenOfIz_BinaryWriter_WriteDouble( m_writer, glyph.Bounds.XMax );
        DenOfIz_BinaryWriter_WriteDouble( m_writer, glyph.Bounds.YMax );
        DenOfIz_BinaryWriter_WriteUInt32( m_writer, glyph.Width );
        DenOfIz_BinaryWriter_WriteUInt32( m_writer, glyph.Height );
        DenOfIz_BinaryWriter_WriteFloat( m_writer, glyph.BearingX );
        DenOfIz_BinaryWriter_WriteFloat( m_writer, glyph.BearingY );
        DenOfIz_BinaryWriter_WriteFloat( m_writer, glyph.XAdvance );
        DenOfIz_BinaryWriter_WriteFloat( m_writer, glyph.YAdvance );
        DenOfIz_BinaryWriter_WriteUInt32( m_writer, glyph.AtlasX );
        DenOfIz_BinaryWriter_WriteUInt32( m_writer, glyph.AtlasY );
    }
}

extern "C"
{

    DenOfIz_FontAssetWriter DenOfIz_FontAssetWriter_Create( const DenOfIz_FontAssetWriterDesc *desc )
    {
        if ( desc == NULL )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        auto *writer = new FontAssetWriter( *desc );
        return DENOFIZ_TO_HANDLE( writer );
    }

    void DenOfIz_FontAssetWriter_Destroy( DenOfIz_FontAssetWriter writer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        delete FONT_ASSET_WRITER_IMPL( writer );
    }

    void DenOfIz_FontAssetWriter_Write( DenOfIz_FontAssetWriter writer, DenOfIz_FontAsset fontAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) || !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
        {
            return;
        }
        FONT_ASSET_WRITER_IMPL( writer )->Write( fontAsset );
    }

    void DenOfIz_FontAssetWriter_End( DenOfIz_FontAssetWriter writer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        FONT_ASSET_WRITER_IMPL( writer )->End( );
    }
}
