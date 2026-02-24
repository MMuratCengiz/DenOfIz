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

#include "DenOfIzGraphics/Assets/Import/FontImporter.h"
#include "DenOfIzGraphics/Assets/Font/Font.h"
#include "DenOfIzGraphics/Assets/Serde/Font/FontAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Font/FontAssetWriter.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"
#include "DenOfIzGraphicsInternal/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphicsInternal/Assets/Import/AssetPathUtilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <freetype/freetype.h>
#include <msdfgen-ext.h>
#include <msdfgen.h>
#include <string>
#include <thread>
#include <vector>

#include "msdf-atlas-gen/AtlasGenerator.h"
#include "msdf-atlas-gen/BitmapAtlasStorage.h"
#include "msdf-atlas-gen/FontGeometry.h"
#include "msdf-atlas-gen/ImmediateAtlasGenerator.h"
#include "msdf-atlas-gen/TightAtlasPacker.h"
#include "msdf-atlas-gen/glyph-generators.h"

using namespace DenOfIz;

#define FONT_IMPORTER_IMPL( handle ) DENOFIZ_FROM_HANDLE( FontImporter, handle )

namespace
{
    std::string ToLowerCopy( std::string value )
    {
        std::transform( value.begin( ), value.end( ), value.begin( ), []( unsigned char c ) { return static_cast<char>( std::tolower( c ) ); } );
        return value;
    }
} // namespace

namespace DenOfIz
{
    class FontImporter
    {
    public:
        FT_Library                      m_ftLibrary{ };
        msdfgen::FreetypeHandle        *m_msdfFtHandle{ };
        std::vector<std::string>        m_createdAssetStorage;
        std::vector<DenOfIz_StringView> m_createdAssetViews;
        std::string                     m_errorMessage;

        std::string                     m_name;
        std::vector<std::string>        m_supportedExtensions;
        std::vector<DenOfIz_StringView> m_supportedExtensionViews;

        struct Rect
        {
            uint32_t X;
            uint32_t Y;
            uint32_t Width;
            uint32_t Height;
        };

        struct ImportContext
        {
            std::string            SourceFilePath;
            std::string            TargetDirectory;
            std::string            AssetNamePrefix;
            DenOfIz_FontImportDesc Desc;
            DenOfIz_ImporterResult Result;
            std::string            ErrorMessage;
            DenOfIz_FontAsset      FontAsset;
            std::vector<Byte>      AtlasDataStorage;
            uint32_t               CurrentAtlasX;
            uint32_t               CurrentAtlasY;
            uint32_t               RowHeight;
            msdfgen::FontHandle   *MsdfFont;
        };

        struct FontStats
        {
            uint32_t GlyphCount;
            uint32_t AtlasWidth;
            uint32_t AtlasHeight;
            size_t   EstimatedArenaSize;
        };

        FontImporter( )
        {
            m_name                = "Font Importer";
            m_supportedExtensions = { "ttf", "otf", "ttc" };

            m_supportedExtensionViews.reserve( m_supportedExtensions.size( ) );
            for ( const std::string &ext : m_supportedExtensions )
            {
                m_supportedExtensionViews.emplace_back( ext.c_str( ), static_cast<uint32_t>( ext.size( ) ) );
            }

            if ( const FT_Error error = FT_Init_FreeType( &m_ftLibrary ); error != 0 )
            {
                spdlog::critical( "Failed to initialize FreeType library: {}", FT_Error_String( error ) );
            }

            m_msdfFtHandle = msdfgen::initializeFreetype( );
            if ( !m_msdfFtHandle )
            {
                spdlog::critical( "Failed to initialize MSDF Freetype library" );
            }
        }

        ~FontImporter( )
        {
            if ( m_msdfFtHandle )
            {
                msdfgen::deinitializeFreetype( m_msdfFtHandle );
                m_msdfFtHandle = nullptr;
            }
            if ( m_ftLibrary )
            {
                FT_Done_FreeType( m_ftLibrary );
                m_ftLibrary = nullptr;
            }
        }

        DenOfIz_StringView GetName( ) const
        {
            return { m_name.c_str( ), static_cast<uint32_t>( m_name.size( ) ) };
        }

        DenOfIz_StringViewArray GetSupportedExtensions( )
        {
            return { m_supportedExtensionViews.data( ), m_supportedExtensionViews.size( ) };
        }

        bool CanProcessFileExtension( const DenOfIz_StringView &extension ) const
        {
            std::string ext( extension.Chars, extension.NumChars );
            ext = ToLowerCopy( ext );
            for ( const std::string &supported : m_supportedExtensions )
            {
                if ( ext == supported )
                {
                    return true;
                }
            }
            return false;
        }

        bool ValidateFile( const DenOfIz_StringView &filePath ) const
        {
            FT_Face           face;
            const std::string resolvedPath = FileIO::GetResourcePath( filePath );

            if ( FT_New_Face( m_ftLibrary, resolvedPath.c_str( ), 0, &face ) )
            {
                return false;
            }

            FT_Done_Face( face );
            return true;
        }

        msdf_atlas::Charset        CreateExtendedCharset( ) const;
        msdf_atlas::Charset        CreateCharsetFromRanges( const DenOfIz_UnicodeRangeArray &ranges ) const;
        msdf_atlas::Charset        GetCharsetToImport( const DenOfIz_UnicodeRangeArray &customRanges ) const;
        FontStats                  CalculateFontStats( const DenOfIz_FontImportDesc &desc ) const;
        DenOfIz_ImporterResultCode ImportFontInternal( ImportContext &context );
        void                       ExtractFontMetrics( ImportContext &context, FT_Face face );
        void                       GenerateAtlas( ImportContext &context );
        void                       WriteFontAsset( ImportContext &context, std::string &outAssetUri );
        DenOfIz_ImporterResult     Import( const DenOfIz_FontImportDesc &desc );
    };

    msdf_atlas::Charset FontImporter::CreateExtendedCharset( ) const
    {
        msdf_atlas::Charset charset;

        for ( msdf_atlas::unicode_t cp = 0x20; cp < 0x7F; ++cp )
        {
            charset.add( cp );
        }

        for ( msdf_atlas::unicode_t cp = 0xA0; cp <= 0xFF; ++cp )
        {
            charset.add( cp );
        }

        charset.add( 0x25B2 );
        charset.add( 0x25BC );
        charset.add( 0x25C0 );
        charset.add( 0x25B6 );
        charset.add( 0x25BA );
        charset.add( 0x2190 );
        charset.add( 0x2191 );
        charset.add( 0x2192 );
        charset.add( 0x2193 );
        charset.add( 0x2022 );
        charset.add( 0x25CB );
        charset.add( 0x25CF );
        charset.add( 0x2713 );
        charset.add( 0x2717 );
        charset.add( 0x26A0 );
        charset.add( 0x26A1 );
        charset.add( 0x25A2 );
        charset.add( 0x25A3 );
        charset.add( 0x25A9 );
        charset.add( 0x25B8 );
        charset.add( 0x23CE );
        charset.add( 0x2026 );
        charset.add( 0x00B7 );
        charset.add( 0x21A9 );
        charset.add( 0x21AA );
        charset.add( 0x2500 );
        charset.add( 0x2502 );
        charset.add( 0x251C );
        charset.add( 0x2514 );
        charset.add( 0x25A0 );
        charset.add( 0x25A1 );
        charset.add( 0x25B4 );
        charset.add( 0x25B5 );
        charset.add( 0x25BE );
        charset.add( 0x25BF );
        charset.add( 0x27E8 );
        charset.add( 0x27E9 );
        charset.add( 0x2039 );
        charset.add( 0x203A );
        charset.add( 0x2260 );
        charset.add( 0x2264 );
        charset.add( 0x2265 );
        charset.add( 0x221E );
        charset.add( 0x2205 );
        charset.add( 0x2208 );
        charset.add( 0x2510 );
        charset.add( 0x250C );
        charset.add( 0x2518 );
        charset.add( 0x253C );

        return charset;
    }

    msdf_atlas::Charset FontImporter::CreateCharsetFromRanges( const DenOfIz_UnicodeRangeArray &ranges ) const
    {
        msdf_atlas::Charset charset;

        for ( size_t i = 0; i < ranges.NumElements; ++i )
        {
            const DenOfIz_UnicodeRange &range = ranges.Elements[ i ];
            for ( uint32_t cp = range.Start; cp <= range.End; ++cp )
            {
                charset.add( cp );
            }
        }

        return charset;
    }

    msdf_atlas::Charset FontImporter::GetCharsetToImport( const DenOfIz_UnicodeRangeArray &customRanges ) const
    {
        if ( customRanges.Elements != nullptr && customRanges.NumElements > 0 )
        {
            return CreateCharsetFromRanges( customRanges );
        }
        return CreateExtendedCharset( );
    }

    FontImporter::FontStats FontImporter::CalculateFontStats( const DenOfIz_FontImportDesc &desc ) const
    {
        FontStats stats{ };

        std::string              sourceFilePath( desc.SourceFilePath.Chars, desc.SourceFilePath.NumChars );
        const std::string        resolvedPath = FileIO::GetResourcePath( DenOfIz_StringView( sourceFilePath.c_str( ), static_cast<uint32_t>( sourceFilePath.size( ) ) ) );
        const DenOfIz_StringView resolvedView( resolvedPath.c_str( ), static_cast<uint32_t>( resolvedPath.size( ) ) );
        std::vector<Byte>        fontData( FileIO::GetFileNumBytes( resolvedView ) );
        FileIO::ReadFile( resolvedView, { fontData.data( ), fontData.size( ) } );
        msdfgen::FontHandle *msdfFont = msdfgen::loadFontData( m_msdfFtHandle, fontData.data( ), fontData.size( ) );
        if ( !msdfFont )
        {
            spdlog::warn( "Failed to load font for pre-calculation, using default estimates" );
            stats.GlyphCount  = 128;
            stats.AtlasWidth  = desc.AtlasWidth;
            stats.AtlasHeight = desc.AtlasHeight;
        }
        else
        {
            std::vector<msdf_atlas::GlyphGeometry> glyphs;
            msdf_atlas::FontGeometry               fontGeometry( &glyphs );
            fontGeometry.loadCharset( msdfFont, 1.0, GetCharsetToImport( desc.CustomRanges ) );

            stats.GlyphCount = 0;
            for ( const auto &glyph : glyphs )
            {
                if ( !glyph.isWhitespace( ) )
                {
                    stats.GlyphCount++;
                }
            }
            stats.GlyphCount++;
            for ( auto &glyph : glyphs )
            {
                glyph.wrapBox( desc.InitialFontSize, DENOFIZ_FONT_MSDF_PIXEL_RANGE / desc.InitialFontSize, 1.0 );
                constexpr double maxCornerAngle = 3.0;
                glyph.edgeColoring( &msdfgen::edgeColoringSimple, maxCornerAngle, 0 );
            }

            msdf_atlas::TightAtlasPacker packer;
            packer.setDimensionsConstraint( msdf_atlas::DimensionsConstraint::SQUARE );
            packer.setMinimumScale( desc.InitialFontSize );
            packer.setSpacing( 2 );
            msdf_atlas::Padding padding( 1.0 );
            packer.setInnerPixelPadding( padding );
            packer.setPixelRange( DENOFIZ_FONT_MSDF_PIXEL_RANGE );
            packer.setMiterLimit( 1.0 );
            packer.pack( glyphs.data( ), glyphs.size( ) );

            int width = 0, height = 0;
            packer.getDimensions( width, height );
            stats.AtlasWidth  = static_cast<uint32_t>( width );
            stats.AtlasHeight = static_cast<uint32_t>( height );

            msdfgen::destroyFont( msdfFont );
        }

        stats.EstimatedArenaSize = sizeof( DenOfIz_FontGlyph ) * stats.GlyphCount;
        stats.EstimatedArenaSize += FileIO::GetFileNumBytes( DENOFIZ_STRING( resolvedPath.c_str( ) ) );
        stats.EstimatedArenaSize += stats.AtlasWidth * stats.AtlasHeight * DENOFIZ_FONT_ASSET_NUM_CHANNELS;
        stats.EstimatedArenaSize += 4096;
        return stats;
    }

    DenOfIz_ImporterResultCode FontImporter::ImportFontInternal( ImportContext &context )
    {
        FT_Face           face;
        const std::string resolvedPath = FileIO::GetResourcePath( DenOfIz_StringView( context.SourceFilePath.c_str( ), static_cast<uint32_t>( context.SourceFilePath.size( ) ) ) );

        DenOfIz_FontAsset_SetPath( context.FontAsset, DENOFIZ_STRING( context.SourceFilePath.c_str( ) ) );
        const DenOfIz_StringView resolvedView( resolvedPath.c_str( ), static_cast<uint32_t>( resolvedPath.size( ) ) );
        const uint64_t           fileSize = FileIO::GetFileNumBytes( resolvedView );
        std::vector<Byte>        fontData( fileSize );
        FileIO::ReadFile( resolvedView, { fontData.data( ), fontData.size( ) } );
        DenOfIz_FontAsset_SetData( context.FontAsset, fontData.data( ), fontData.size( ) );

        FT_Error error = FT_New_Face( m_ftLibrary, resolvedPath.c_str( ), 0, &face );

        if ( error )
        {
            context.ErrorMessage = "Failed to load font: " + std::string( FT_Error_String( error ) );
            return DENOFIZ_IMPORTER_RESULT_INVALID_PARAMETERS;
        }

        error = FT_Set_Char_Size( face, 0, context.Desc.InitialFontSize * 64, 0, 0 );
        if ( error )
        {
            FT_Done_Face( face );
            context.ErrorMessage = "Failed to set font size: " + std::string( FT_Error_String( error ) );
            return DENOFIZ_IMPORTER_RESULT_INVALID_PARAMETERS;
        }

        ExtractFontMetrics( context, face );
        GenerateAtlas( context );

        FT_Done_Face( face );
        return DENOFIZ_IMPORTER_RESULT_SUCCESS;
    }

    void FontImporter::ExtractFontMetrics( ImportContext &context, const FT_Face face )
    {
        const float ascender  = static_cast<float>( face->size->metrics.ascender ) / 64.0f;
        const float descender = static_cast<float>( face->size->metrics.descender ) / 64.0f;
        const float height    = static_cast<float>( face->size->metrics.height ) / 64.0f;

        DenOfIz_FontMetrics metrics{ };
        metrics.Ascent     = ascender;
        metrics.Descent    = std::abs( descender );
        metrics.LineHeight = height;
        metrics.LineGap    = height - ( ascender - descender );

        if ( FT_IS_SCALABLE( face ) )
        {
            const float underlinePos       = static_cast<float>( face->underline_position ) / 64.0f;
            const float underlineThickness = static_cast<float>( face->underline_thickness ) / 64.0f;

            metrics.UnderlinePos       = std::abs( underlinePos );
            metrics.UnderlineThickness = underlineThickness;
        }
        else
        {
            metrics.UnderlinePos       = metrics.Descent / 2.0f;
            metrics.UnderlineThickness = metrics.Ascent / 20.0f;
        }

        DenOfIz_FontAsset_SetMetrics( context.FontAsset, &metrics );
    }

    void FontImporter::GenerateAtlas( ImportContext &context )
    {
        const DenOfIz_ByteArray fontData     = DenOfIz_FontAsset_Data( context.FontAsset );
        const Byte             *data         = fontData.Elements;
        const uint64_t          dataNumBytes = fontData.NumElements;
        msdfgen::FontHandle    *msdfFont     = msdfgen::loadFontData( m_msdfFtHandle, data, dataNumBytes );
        if ( !msdfFont )
        {
            spdlog::error( "Failed to load MSDF font for glyph generation" );
            return;
        }

        std::vector<msdf_atlas::GlyphGeometry> glyphs;
        msdf_atlas::FontGeometry               fontGeometry( &glyphs );

        fontGeometry.loadCharset( msdfFont, 1.0, GetCharsetToImport( context.Desc.CustomRanges ) );

        for ( msdf_atlas::GlyphGeometry &glyph : glyphs )
        {
            glyph.wrapBox( context.Desc.InitialFontSize, DENOFIZ_FONT_MSDF_PIXEL_RANGE / context.Desc.InitialFontSize, 1.0 );
            constexpr double maxCornerAngle = 3.0;
            glyph.edgeColoring( &msdfgen::edgeColoringSimple, maxCornerAngle, 0 );
        }

        msdf_atlas::TightAtlasPacker packer;
        packer.setDimensionsConstraint( msdf_atlas::DimensionsConstraint::SQUARE );
        packer.setMinimumScale( context.Desc.InitialFontSize );
        packer.setSpacing( 2 );
        msdf_atlas::Padding padding( 1.0 );
        packer.setInnerPixelPadding( padding );

        packer.setPixelRange( DenOfIz_Font_MsdfPixelRange( ) );
        packer.setMiterLimit( 1.0 );
        packer.pack( glyphs.data( ), glyphs.size( ) );

        int width = 0, height = 0;
        packer.getDimensions( width, height );

        if ( width != static_cast<int>( DenOfIz_FontAsset_AtlasWidth( context.FontAsset ) ) || height != static_cast<int>( DenOfIz_FontAsset_AtlasHeight( context.FontAsset ) ) )
        {
            spdlog::warn( "Atlas dimensions mismatch - expected {}x{}, got {}x{}", DenOfIz_FontAsset_AtlasWidth( context.FontAsset ),
                          DenOfIz_FontAsset_AtlasHeight( context.FontAsset ), width, height );
            const size_t            newAtlasSize     = width * height * DENOFIZ_FONT_ASSET_NUM_CHANNELS;
            const DenOfIz_ByteArray currentAtlasData = DenOfIz_FontAsset_AtlasData( context.FontAsset );
            if ( newAtlasSize > currentAtlasData.NumElements )
            {
                DenOfIz_FontAsset_SetAtlasWidth( context.FontAsset, width );
                DenOfIz_FontAsset_SetAtlasHeight( context.FontAsset, height );
                context.AtlasDataStorage.resize( newAtlasSize );
                DenOfIz_FontAsset_SetAtlasData( context.FontAsset, context.AtlasDataStorage.data( ), newAtlasSize );
            }
        }

        msdf_atlas::GeneratorAttributes attributes;
        attributes.config.overlapSupport = true;
        attributes.scanlinePass          = true;
        msdf_atlas::ImmediateAtlasGenerator<float, 4, msdf_atlas::mtsdfGenerator, msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 4>> generator( width, height );

        generator.setAttributes( attributes );
        generator.setThreadCount( std::thread::hardware_concurrency( ) );

        generator.generate( glyphs.data( ), glyphs.size( ) );

        const auto &atlasStorage = generator.atlasStorage( );

        const msdfgen::BitmapConstRef<msdfgen::byte, 4> &bitmap       = atlasStorage;
        const msdfgen::byte                             *pixels       = bitmap.pixels;
        const DenOfIz_ByteArray                          atlasDataArr = DenOfIz_FontAsset_AtlasData( context.FontAsset );
        Byte                                            *atlasData    = atlasDataArr.Elements;

        for ( int y = 0; y < height; y++ )
        {
            int invertedY = height - 1 - y;
            for ( int x = 0; x < width; x++ )
            {
                const int      srcIdx   = 4 * ( invertedY * width + x );
                const uint32_t dstIdx   = DENOFIZ_FONT_ASSET_NUM_CHANNELS * ( y * width + x );
                atlasData[ dstIdx ]     = pixels[ srcIdx ];
                atlasData[ dstIdx + 1 ] = pixels[ srcIdx + 1 ];
                atlasData[ dstIdx + 2 ] = pixels[ srcIdx + 2 ];
                atlasData[ dstIdx + 3 ] = pixels[ srcIdx + 3 ];
            }
        }

        const auto &layout = generator.getLayout( );

        double emSize     = fontGeometry.getMetrics( ).emSize;
        double ascender   = fontGeometry.getMetrics( ).ascenderY;
        double descender  = fontGeometry.getMetrics( ).descenderY;
        double lineHeight = fontGeometry.getMetrics( ).lineHeight;
        double scale      = context.Desc.InitialFontSize / emSize;

        DenOfIz_FontMetrics metrics{ };
        metrics.Ascent     = static_cast<float>( ascender * scale );
        metrics.Descent    = static_cast<float>( std::abs( descender ) * scale );
        metrics.LineHeight = static_cast<float>( lineHeight * scale );
        metrics.LineGap    = metrics.LineHeight - ( metrics.Ascent + metrics.Descent );
        DenOfIz_FontAsset_SetMetrics( context.FontAsset, &metrics );

        size_t numNonWhitespaceGlyphs = 0;
        for ( size_t i = 0; i < layout.size( ); i++ )
        {
            if ( !glyphs[ i ].isWhitespace( ) )
            {
                numNonWhitespaceGlyphs++;
            }
        }

        DenOfIz_FontAsset_ReserveGlyphs( context.FontAsset, numNonWhitespaceGlyphs + 1 );

        for ( size_t i = 0; i < layout.size( ); i++ )
        {
            const auto &box   = layout[ i ];
            const auto &glyph = glyphs[ i ];
            if ( glyph.isWhitespace( ) )
            {
                continue;
            }

            DenOfIz_FontGlyph glyphDesc{ };
            glyphDesc.CodePoint = glyph.getCodepoint( );
            glyphDesc.Width     = box.rect.w;
            glyphDesc.Height    = box.rect.h;
            glyphDesc.AtlasX    = box.rect.x;
            glyphDesc.AtlasY    = height - ( box.rect.y + box.rect.h );

            double planeBoundsL, planeBoundsB, planeBoundsR, planeBoundsT;
            glyph.getQuadPlaneBounds( planeBoundsL, planeBoundsB, planeBoundsR, planeBoundsT );

            glyphDesc.Bounds.XMin = planeBoundsL;
            glyphDesc.Bounds.XMax = planeBoundsR;
            glyphDesc.Bounds.YMin = planeBoundsB;
            glyphDesc.Bounds.YMax = planeBoundsT;

            glyphDesc.BearingX = static_cast<float>( planeBoundsL * context.Desc.InitialFontSize );
            glyphDesc.BearingY = static_cast<float>( planeBoundsT * context.Desc.InitialFontSize );
            glyphDesc.XAdvance = static_cast<float>( glyph.getAdvance( ) * context.Desc.InitialFontSize );
            glyphDesc.YAdvance = 0.0f;

            DenOfIz_FontAsset_AddGlyph( context.FontAsset, &glyphDesc );
        }

        DenOfIz_FontGlyph spaceGlyph{ };
        spaceGlyph.CodePoint = ' ';
        spaceGlyph.Width     = 0;
        spaceGlyph.Height    = 0;
        spaceGlyph.BearingX  = 0.0f;
        spaceGlyph.BearingY  = 0.0f;
        spaceGlyph.XAdvance  = static_cast<float>( context.Desc.InitialFontSize ) * 0.25f;
        spaceGlyph.YAdvance  = 0.0f;
        spaceGlyph.AtlasX    = 0;
        spaceGlyph.AtlasY    = 0;
        DenOfIz_FontAsset_AddGlyph( context.FontAsset, &spaceGlyph );

        msdfgen::destroyFont( msdfFont );
    }

    void FontImporter::WriteFontAsset( ImportContext &context, std::string &outAssetUri )
    {
        const DenOfIz_StringView ext = DenOfIz_FontAsset_Extension( );
        const std::string        extStr( ext.Chars, ext.NumChars );
        const std::string        assetName         = AssetPathUtilities::GetAssetNameFromFilePath( DENOFIZ_STRING( context.SourceFilePath.c_str( ) ) );
        const std::string        sanitizedName     = AssetPathUtilities::SanitizeAssetName( assetName );
        const std::string        fontAssetFileName = AssetPathUtilities::CreateAssetFileName( context.AssetNamePrefix, sanitizedName, extStr );

        std::string outputPath = context.TargetDirectory;
        if ( !outputPath.empty( ) && outputPath.back( ) != '/' && outputPath.back( ) != '\\' )
        {
            outputPath += '/';
        }
        outputPath += fontAssetFileName;

        DenOfIz_BinaryContainer container = context.Desc.TargetContainer;
        if ( container == DENOFIZ_NULL_HANDLE )
        {
            container = DenOfIz_BinaryContainer_Create( );
        }
        DenOfIz_BinaryWriter writer = DenOfIz_BinaryWriter_CreateFromContainer( container );

        DenOfIz_FontAssetWriterDesc writerDesc{ };
        writerDesc.Writer                  = writer;
        DenOfIz_FontAssetWriter fontWriter = DenOfIz_FontAssetWriter_Create( &writerDesc );

        DenOfIz_FontAssetWriter_Write( fontWriter, context.FontAsset );
        DenOfIz_FontAssetWriter_End( fontWriter );
        DenOfIz_FontAssetWriter_Destroy( fontWriter );

        if ( context.Desc.TargetContainer == DENOFIZ_NULL_HANDLE )
        {
            const DenOfIz_StringView outputPathView( outputPath.c_str( ), static_cast<uint32_t>( outputPath.size( ) ) );
            FileIO::WriteFile( outputPathView, DenOfIz_BinaryContainer_GetData( container ) );
            outAssetUri = outputPath;
            DenOfIz_BinaryContainer_Destroy( container );
        }
        DenOfIz_BinaryWriter_Destroy( writer );
    }

    DenOfIz_ImporterResult FontImporter::Import( const DenOfIz_FontImportDesc &desc )
    {
        m_createdAssetStorage.clear( );
        m_createdAssetViews.clear( );

        const FontStats stats = CalculateFontStats( desc );

        ImportContext context{ };
        context.SourceFilePath    = std::string( desc.SourceFilePath.Chars, desc.SourceFilePath.NumChars );
        context.TargetDirectory   = std::string( desc.TargetDirectory.Chars, desc.TargetDirectory.NumChars );
        context.AssetNamePrefix   = std::string( desc.AssetNamePrefix.Chars, desc.AssetNamePrefix.NumChars );
        context.Desc              = desc;
        context.Result.ResultCode = DENOFIZ_IMPORTER_RESULT_SUCCESS;
        context.FontAsset         = DenOfIz_FontAsset_Create( );

        const size_t atlasSize = stats.AtlasWidth * stats.AtlasHeight * DENOFIZ_FONT_ASSET_NUM_CHANNELS;
        context.AtlasDataStorage.resize( atlasSize );
        DenOfIz_FontAsset_SetAtlasData( context.FontAsset, context.AtlasDataStorage.data( ), atlasSize );

        DenOfIz_FontAsset_SetInitialFontSize( context.FontAsset, context.Desc.InitialFontSize );
        DenOfIz_FontAsset_SetAtlasWidth( context.FontAsset, stats.AtlasWidth );
        DenOfIz_FontAsset_SetAtlasHeight( context.FontAsset, stats.AtlasHeight );

        if ( const DenOfIz_ImporterResultCode result = ImportFontInternal( context ); result != DENOFIZ_IMPORTER_RESULT_SUCCESS )
        {
            context.Result.ResultCode   = result;
            m_errorMessage              = context.ErrorMessage;
            char *errorMsgCopy          = strdup( m_errorMessage.c_str( ) );
            context.Result.ErrorMessage = { errorMsgCopy, static_cast<uint32_t>( m_errorMessage.length( ) ) };
            DenOfIz_FontAsset_Destroy( context.FontAsset );
            return context.Result;
        }

        std::string fontAssetUri;
        WriteFontAsset( context, fontAssetUri );
        m_createdAssetStorage.push_back( fontAssetUri );

        if ( !m_createdAssetStorage.empty( ) )
        {
            auto *createdAssetsArray = static_cast<DenOfIz_StringView *>( malloc( m_createdAssetStorage.size( ) * sizeof( DenOfIz_StringView ) ) );
            for ( size_t i = 0; i < m_createdAssetStorage.size( ); ++i )
            {
                char *assetPathCopy              = strdup( m_createdAssetStorage[ i ].c_str( ) );
                createdAssetsArray[ i ].Chars    = assetPathCopy;
                createdAssetsArray[ i ].NumChars = static_cast<uint32_t>( m_createdAssetStorage[ i ].length( ) );
            }
            context.Result.CreatedAssets.NumElements = static_cast<uint32_t>( m_createdAssetStorage.size( ) );
            context.Result.CreatedAssets.Elements    = createdAssetsArray;
        }
        else
        {
            context.Result.CreatedAssets.NumElements = 0;
            context.Result.CreatedAssets.Elements    = nullptr;
        }

        DenOfIz_FontAsset_Destroy( context.FontAsset );
        return context.Result;
    }
} // namespace DenOfIz

extern "C"
{

    DenOfIz_FontImporter DenOfIz_FontImporter_Create( )
    {
        auto *impl = new FontImporter( );
        return DENOFIZ_TO_HANDLE( impl );
    }

    void DenOfIz_FontImporter_Destroy( DenOfIz_FontImporter importer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( importer ) )
        {
            return;
        }
        delete FONT_IMPORTER_IMPL( importer );
    }

    DenOfIz_StringView DenOfIz_FontImporter_GetName( DenOfIz_FontImporter importer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( importer ) )
        {
            return { NULL, 0 };
        }
        return FONT_IMPORTER_IMPL( importer )->GetName( );
    }

    DenOfIz_StringViewArray DenOfIz_FontImporter_GetSupportedExtensions( DenOfIz_FontImporter importer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( importer ) )
        {
            return { NULL, 0 };
        }
        return FONT_IMPORTER_IMPL( importer )->GetSupportedExtensions( );
    }

    bool DenOfIz_FontImporter_CanProcessFileExtension( DenOfIz_FontImporter importer, DenOfIz_StringView extension )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( importer ) )
        {
            return false;
        }
        return FONT_IMPORTER_IMPL( importer )->CanProcessFileExtension( extension );
    }

    bool DenOfIz_FontImporter_ValidateFile( DenOfIz_FontImporter importer, DenOfIz_StringView filePath )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( importer ) )
        {
            return false;
        }
        return FONT_IMPORTER_IMPL( importer )->ValidateFile( filePath );
    }

    DenOfIz_ImporterResult DenOfIz_FontImporter_Import( DenOfIz_FontImporter importer, const DenOfIz_FontImportDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( importer ) || desc == NULL )
        {
            DenOfIz_ImporterResult result{ };
            result.ResultCode = DENOFIZ_IMPORTER_RESULT_INVALID_PARAMETERS;
            return result;
        }
        return FONT_IMPORTER_IMPL( importer )->Import( *desc );
    }

    void DenOfIz_ImporterResult_Free( DenOfIz_ImporterResult *result )
    {
        if ( result == NULL )
        {
            return;
        }
        if ( result->ErrorMessage.Chars != NULL )
        {
            free( const_cast<char *>( result->ErrorMessage.Chars ) );
            result->ErrorMessage.Chars    = NULL;
            result->ErrorMessage.NumChars = 0;
        }
        if ( result->CreatedAssets.Elements != NULL )
        {
            for ( size_t i = 0; i < result->CreatedAssets.NumElements; ++i )
            {
                if ( result->CreatedAssets.Elements[ i ].Chars != NULL )
                {
                    free( const_cast<char *>( result->CreatedAssets.Elements[ i ].Chars ) );
                }
            }
            free( (void *)result->CreatedAssets.Elements );
            result->CreatedAssets.Elements    = NULL;
            result->CreatedAssets.NumElements = 0;
        }
    }
}
