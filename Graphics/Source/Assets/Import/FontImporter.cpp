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

#include <freetype/freetype.h>
#include <msdfgen-ext.h>
#include <msdfgen.h>
#include "msdf-atlas-gen/AtlasGenerator.h"
#include "msdf-atlas-gen/BitmapAtlasStorage.h"
#include "msdf-atlas-gen/FontGeometry.h"
#include "msdf-atlas-gen/ImmediateAtlasGenerator.h"
#include "msdf-atlas-gen/TightAtlasPacker.h"
#include "msdf-atlas-gen/glyph-generators.h"

#include <thread>
#include <unordered_set>
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphics/Assets/FileSystem/PathResolver.h"
#include "DenOfIzGraphics/Assets/Font/Font.h"
#include "DenOfIzGraphics/Assets/Import/AssetPathUtilities.h"
#include "DenOfIzGraphics/Assets/Import/FontImporter.h"
#include "DenOfIzGraphics/Assets/Serde/Font/FontAssetWriter.h"
#include "DenOfIzGraphicsInternal/Utilities/DZArenaHelper.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

namespace DenOfIz
{
    struct FontImporterImpl
    {
        FT_Library               m_ftLibrary{ };
        msdfgen::FreetypeHandle *m_msdfFtHandle{ };
        std::vector<AssetUri>    m_createdAssets;

        struct Rect
        {
            uint32_t X;
            uint32_t Y;
            uint32_t Width;
            uint32_t Height;
        };

        struct ImportContext
        {
            InteropString  SourceFilePath;
            InteropString  TargetDirectory;
            InteropString  AssetNamePrefix;
            FontImportDesc Desc;
            ImporterResult Result;
            InteropString  ErrorMessage;

            FontAsset *FontAsset;

            uint32_t CurrentAtlasX = 0;
            uint32_t CurrentAtlasY = 0;
            uint32_t RowHeight     = 0;

            msdfgen::FontHandle *MsdfFont = nullptr;
        };

        struct FontStats
        {
            uint32_t GlyphCount         = 0;
            uint32_t AtlasWidth         = 0;
            uint32_t AtlasHeight        = 0;
            size_t   EstimatedArenaSize = 0;
        };
    };
} // namespace DenOfIz

namespace
{
    ImporterResultCode          ImportFontInternal( const FontImporterImpl &impl, FontImporterImpl::ImportContext &context );
    void                        ExtractFontMetrics( const FontImporterImpl::ImportContext &context, FT_Face face );
    void                        GenerateAtlas( const FontImporterImpl &impl, FontImporterImpl::ImportContext &context );
    void                        WriteFontAsset( const FontImporterImpl::ImportContext &context, AssetUri &outAssetUri );
    FontImporterImpl::FontStats CalculateFontStats( const FontImporterImpl &impl, const FontImportDesc &desc );
} // namespace

FontImporter::FontImporter( ) : m_name( "Font Importer" ), m_impl( std::make_unique<FontImporterImpl>( ) )
{
    if ( const FT_Error error = FT_Init_FreeType( &m_impl->m_ftLibrary ); error != 0 )
    {
        spdlog::critical( "Failed to initialize FreeType library: {}", FT_Error_String( error ) );
    }

    m_impl->m_msdfFtHandle = msdfgen::initializeFreetype( );
    if ( !m_impl->m_msdfFtHandle )
    {
        spdlog::critical( "Failed to initialize MSDF Freetype library" );
    }

    m_supportedExtensions               = InteropStringArray::Create( 3 );
    m_supportedExtensions.Elements[ 0 ] = "ttf";
    m_supportedExtensions.Elements[ 1 ] = "otf";
    m_supportedExtensions.Elements[ 2 ] = "ttc";
}

FontImporter::~FontImporter( )
{
    if ( m_impl->m_msdfFtHandle )
    {
        msdfgen::deinitializeFreetype( m_impl->m_msdfFtHandle );
        m_impl->m_msdfFtHandle = nullptr;
    }
    m_supportedExtensions.Dispose( );
}

InteropString FontImporter::GetName( ) const
{
    return m_name;
}

InteropStringArray FontImporter::GetSupportedExtensions( ) const
{
    const InteropStringArray copy = InteropStringArray::Create( m_supportedExtensions.NumElements );
    for ( size_t i = 0; i < m_supportedExtensions.NumElements; ++i )
    {
        copy.Elements[ i ] = m_supportedExtensions.Elements[ i ];
    }
    return copy;
}

bool FontImporter::CanProcessFileExtension( const InteropString &extension ) const
{
    const std::string ext = extension.Get( );
    for ( size_t i = 0; i < m_supportedExtensions.NumElements; ++i )
    {
        if ( strcmp( extension.Get( ), m_supportedExtensions.Elements[ i ].Get( ) ) == 0 )
        {
            return true;
        }
    }
    return false;
}

ImporterResult FontImporter::Import( const FontImportDesc &desc ) const
{
    const FontImporterImpl::FontStats stats = CalculateFontStats( *m_impl, desc );

    FontImporterImpl::ImportContext context;
    context.SourceFilePath    = desc.SourceFilePath;
    context.TargetDirectory   = desc.TargetDirectory;
    context.AssetNamePrefix   = desc.AssetNamePrefix;
    context.Desc              = desc;
    context.Result.ResultCode = ImporterResultCode::Success;
    context.FontAsset         = new FontAsset( );
    context.FontAsset->_Arena.EnsureCapacity( stats.EstimatedArenaSize );

    const size_t atlasSize       = stats.AtlasWidth * stats.AtlasHeight * FontAsset::NumChannels;
    context.FontAsset->AtlasData = ByteArray::Create( atlasSize );
    memset( context.FontAsset->AtlasData.Elements, 0, atlasSize );

    context.FontAsset->InitialFontSize   = context.Desc.InitialFontSize;
    context.FontAsset->AtlasWidth        = stats.AtlasWidth;
    context.FontAsset->AtlasHeight       = stats.AtlasHeight;
    context.FontAsset->NumAtlasDataBytes = atlasSize;

    if ( const ImporterResultCode result = ImportFontInternal( *m_impl, context ); result != ImporterResultCode::Success )
    {
        context.Result.ResultCode   = result;
        context.Result.ErrorMessage = context.ErrorMessage;
        delete context.FontAsset;
        return context.Result;
    }

    AssetUri fontAssetUri;
    WriteFontAsset( context, fontAssetUri );
    m_impl->m_createdAssets.push_back( fontAssetUri );

    context.Result.CreatedAssets.NumElements = static_cast<uint32_t>( m_impl->m_createdAssets.size( ) );
    context.Result.CreatedAssets.Elements    = m_impl->m_createdAssets.data( );

    delete context.FontAsset;
    return context.Result;
}

bool FontImporter::ValidateFile( const InteropString &filePath ) const
{
    FT_Face           face;
    const std::string resolvedPath = PathResolver::ResolvePath( filePath.Get( ) );

    if ( FT_New_Face( m_impl->m_ftLibrary, resolvedPath.c_str( ), 0, &face ) )
    {
        return false;
    }

    FT_Done_Face( face );
    return true;
}

namespace
{
    FontImporterImpl::FontStats CalculateFontStats( const FontImporterImpl &impl, const FontImportDesc &desc )
    {
        FontImporterImpl::FontStats stats;

        const std::string    resolvedPath = PathResolver::ResolvePath( desc.SourceFilePath.Get( ) );
        const ByteArray      fontData     = FileIO::ReadFile( InteropString( resolvedPath.c_str( ) ) );
        msdfgen::FontHandle *msdfFont     = msdfgen::loadFontData( impl.m_msdfFtHandle, fontData.Elements, fontData.NumElements );
        if ( !msdfFont )
        {
            spdlog::warn( "Failed to load font for pre-calculation, using default estimates" );
            stats.GlyphCount  = 128; // ASCII charset estimate
            stats.AtlasWidth  = desc.AtlasWidth;
            stats.AtlasHeight = desc.AtlasHeight;
        }
        else
        {
            std::vector<msdf_atlas::GlyphGeometry> glyphs;
            msdf_atlas::FontGeometry               fontGeometry( &glyphs );
            fontGeometry.loadCharset( msdfFont, 1.0, msdf_atlas::Charset::ASCII );

            stats.GlyphCount = 0;
            for ( const auto &glyph : glyphs )
            {
                if ( !glyph.isWhitespace( ) )
                {
                    stats.GlyphCount++;
                }
            }
            stats.GlyphCount++; // ' '
            for ( auto &glyph : glyphs )
            {
                constexpr double maxCornerAngle = 3.0;
                glyph.edgeColoring( &msdfgen::edgeColoringByDistance, maxCornerAngle, 0 );
            }

            msdf_atlas::TightAtlasPacker packer;
            packer.setDimensionsConstraint( msdf_atlas::DimensionsConstraint::SQUARE );
            packer.setMinimumScale( desc.InitialFontSize );
            packer.setPixelRange( Font::MsdfPixelRange );
            packer.setMiterLimit( 1.0 );
            packer.pack( glyphs.data( ), glyphs.size( ) );

            int width = 0, height = 0;
            packer.getDimensions( width, height );
            stats.AtlasWidth  = static_cast<uint32_t>( width );
            stats.AtlasHeight = static_cast<uint32_t>( height );

            msdfgen::destroyFont( msdfFont );
        }

        fontData.Dispose( );

        stats.EstimatedArenaSize = sizeof( FontGlyph ) * stats.GlyphCount;
        stats.EstimatedArenaSize += sizeof( UserProperty ) * 10;
        stats.EstimatedArenaSize += 4096;
        return stats;
    }

    ImporterResultCode ImportFontInternal( const FontImporterImpl &impl, FontImporterImpl::ImportContext &context )
    {
        FT_Face           face;
        const std::string resolvedPath = PathResolver::ResolvePath( context.SourceFilePath.Get( ) );

        context.FontAsset->Uri.Path = context.SourceFilePath;

        context.FontAsset->Data         = FileIO::ReadFile( InteropString( resolvedPath.c_str( ) ) );
        context.FontAsset->DataNumBytes = context.FontAsset->Data.NumElements;

        FT_Error error = FT_New_Face( impl.m_ftLibrary, resolvedPath.c_str( ), 0, &face );

        if ( error )
        {
            context.ErrorMessage = InteropString( "Failed to load font: " ).Append( FT_Error_String( error ) );
            return ImporterResultCode::InvalidParameters;
        }

        error = FT_Set_Char_Size( face, 0, context.Desc.InitialFontSize * 64, 0, 0 );
        if ( error )
        {
            FT_Done_Face( face );
            context.ErrorMessage = InteropString( "Failed to set font size: " ).Append( FT_Error_String( error ) );
            return ImporterResultCode::InvalidParameters;
        }

        ExtractFontMetrics( context, face );
        GenerateAtlas( impl, context );

        FT_Done_Face( face );
        return ImporterResultCode::Success;
    }

    void ExtractFontMetrics( const FontImporterImpl::ImportContext &context, const FT_Face face )
    {
        context.FontAsset->Metrics.Ascent     = static_cast<uint32_t>( face->size->metrics.ascender >> 6 );
        context.FontAsset->Metrics.Descent    = static_cast<uint32_t>( abs( face->size->metrics.descender ) >> 6 );
        context.FontAsset->Metrics.LineGap    = static_cast<uint32_t>( ( face->size->metrics.height - ( face->size->metrics.ascender - face->size->metrics.descender ) ) >> 6 );
        context.FontAsset->Metrics.LineHeight = static_cast<uint32_t>( face->size->metrics.height >> 6 );

        if ( FT_IS_SCALABLE( face ) )
        {
            context.FontAsset->Metrics.UnderlinePos       = static_cast<uint32_t>( abs( face->underline_position ) >> 6 );
            context.FontAsset->Metrics.UnderlineThickness = static_cast<uint32_t>( face->underline_thickness >> 6 );
        }
        else
        {
            context.FontAsset->Metrics.UnderlinePos       = context.FontAsset->Metrics.Descent / 2;
            context.FontAsset->Metrics.UnderlineThickness = context.FontAsset->Metrics.Ascent / 20;
        }
    }

    void GenerateAtlas( const FontImporterImpl &impl, FontImporterImpl::ImportContext &context )
    {
        const Byte          *data         = context.FontAsset->Data.Elements;
        const uint64_t       dataNumBytes = context.FontAsset->DataNumBytes;
        msdfgen::FontHandle *msdfFont     = msdfgen::loadFontData( impl.m_msdfFtHandle, data, dataNumBytes );
        if ( !msdfFont )
        {
            spdlog::error( "Failed to load MSDF font for glyph generation" );
            return;
        }

        std::vector<msdf_atlas::GlyphGeometry> glyphs;
        msdf_atlas::FontGeometry               fontGeometry( &glyphs );

        fontGeometry.loadCharset( msdfFont, 1.0, msdf_atlas::Charset::ASCII );

        for ( msdf_atlas::GlyphGeometry &glyph : glyphs )
        {
            constexpr double maxCornerAngle = 3.0;
            glyph.edgeColoring( &msdfgen::edgeColoringByDistance, maxCornerAngle, 0 );
        }

        msdf_atlas::TightAtlasPacker packer;
        packer.setDimensionsConstraint( msdf_atlas::DimensionsConstraint::SQUARE );
        packer.setMinimumScale( context.Desc.InitialFontSize );
        packer.setPixelRange( Font::MsdfPixelRange );
        packer.setMiterLimit( 1.0 );
        packer.pack( glyphs.data( ), glyphs.size( ) );

        int width = 0, height = 0;
        packer.getDimensions( width, height );

        if ( width != static_cast<int>( context.FontAsset->AtlasWidth ) || height != static_cast<int>( context.FontAsset->AtlasHeight ) )
        {
            spdlog::warn( "Atlas dimensions mismatch - expected {}x{}, got {}x{}", context.FontAsset->AtlasWidth, context.FontAsset->AtlasHeight, width, height );
            if ( static_cast<size_t>( width * height * FontAsset::NumChannels ) > context.FontAsset->AtlasData.NumElements )
            {
                context.FontAsset->AtlasData.Dispose( );
                context.FontAsset->AtlasWidth        = width;
                context.FontAsset->AtlasHeight       = height;
                context.FontAsset->NumAtlasDataBytes = width * height * FontAsset::NumChannels;
                context.FontAsset->AtlasData         = ByteArray::Create( context.FontAsset->NumAtlasDataBytes );
                memset( context.FontAsset->AtlasData.Elements, 0, context.FontAsset->NumAtlasDataBytes );
            }
        }

        msdf_atlas::GeneratorAttributes attributes;
        attributes.config.overlapSupport = true;
        attributes.scanlinePass          = true;
        msdf_atlas::ImmediateAtlasGenerator<float, 4, msdf_atlas::mtsdfGenerator, msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 4>> generator( width, height );

        generator.setAttributes( attributes );
        generator.setThreadCount( std::thread::hardware_concurrency( ) );

        // Generate Atlas:
        generator.generate( glyphs.data( ), glyphs.size( ) );

        const auto &atlasStorage = generator.atlasStorage( );

        const msdfgen::BitmapConstRef<msdfgen::byte, 4> &bitmap    = atlasStorage;
        const msdfgen::byte                             *pixels    = bitmap.pixels;
        Byte                                            *atlasData = context.FontAsset->AtlasData.Elements;

        for ( int y = 0; y < height; y++ )
        {
            int invertedY = height - 1 - y;
            for ( int x = 0; x < width; x++ )
            {
                const int      srcIdx   = 4 * ( invertedY * width + x );
                const uint32_t dstIdx   = FontAsset::NumChannels * ( y * width + x );
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

        context.FontAsset->Metrics.Ascent     = static_cast<uint32_t>( ascender * scale );
        context.FontAsset->Metrics.Descent    = static_cast<uint32_t>( std::abs( descender ) * scale );
        context.FontAsset->Metrics.LineHeight = static_cast<uint32_t>( lineHeight * scale );
        context.FontAsset->Metrics.LineGap    = context.FontAsset->Metrics.LineHeight - ( context.FontAsset->Metrics.Ascent + context.FontAsset->Metrics.Descent );

        // Count non-whitespace glyphs first
        size_t numNonWhitespaceGlyphs = 0;
        for ( size_t i = 0; i < layout.size( ); i++ )
        {
            if ( !glyphs[ i ].isWhitespace( ) )
            {
                numNonWhitespaceGlyphs++;
            }
        }

        // Allocate array for glyphs (including space glyph)
        DZArenaArrayHelper<FontGlyphArray, FontGlyph>::AllocateAndConstructArray( context.FontAsset->_Arena, context.FontAsset->Glyphs, numNonWhitespaceGlyphs + 1 );

        size_t glyphIndex = 0;
        for ( size_t i = 0; i < layout.size( ); i++ )
        {
            const auto &box   = layout[ i ];
            const auto &glyph = glyphs[ i ];
            if ( glyph.isWhitespace( ) )
            {
                continue;
            }

            FontGlyph &glyphDesc = context.FontAsset->Glyphs.Elements[ glyphIndex++ ];
            glyphDesc.CodePoint  = glyph.getCodepoint( );
            glyphDesc.Width      = box.rect.w;
            glyphDesc.Height     = box.rect.h;
            glyphDesc.AtlasX     = box.rect.x;
            glyphDesc.AtlasY     = height - ( box.rect.y + box.rect.h );

            double planeBoundsL, planeBoundsB, planeBoundsR, planeBoundsT;
            glyph.getQuadPlaneBounds( planeBoundsL, planeBoundsB, planeBoundsR, planeBoundsT );

            glyphDesc.Bounds.XMin = planeBoundsL;
            glyphDesc.Bounds.XMax = planeBoundsR;
            glyphDesc.Bounds.YMin = planeBoundsB;
            glyphDesc.Bounds.YMax = planeBoundsT;

            glyphDesc.BearingX = static_cast<int32_t>( std::round( planeBoundsL * context.Desc.InitialFontSize ) );
            glyphDesc.BearingY = static_cast<int32_t>( std::round( planeBoundsT * context.Desc.InitialFontSize ) );

            glyphDesc.XAdvance = static_cast<uint32_t>( std::round( glyph.getAdvance( ) * context.Desc.InitialFontSize ) );
            glyphDesc.YAdvance = 0;
        }

        // Add space glyph at the end
        FontGlyph &spaceGlyph = context.FontAsset->Glyphs.Elements[ glyphIndex ];
        spaceGlyph.CodePoint  = ' ';
        spaceGlyph.Width      = 0;
        spaceGlyph.Height     = 0;
        spaceGlyph.BearingX   = 0;
        spaceGlyph.BearingY   = 0;
        spaceGlyph.XAdvance   = static_cast<uint32_t>( std::round( context.Desc.InitialFontSize * 0.3f ) );
        spaceGlyph.YAdvance   = 0;
        spaceGlyph.AtlasX     = 0;
        spaceGlyph.AtlasY     = 0;

        msdfgen::destroyFont( msdfFont );
    }

    void WriteFontAsset( const FontImporterImpl::ImportContext &context, AssetUri &outAssetUri )
    {
        const InteropString assetName         = AssetPathUtilities::GetAssetNameFromFilePath( context.SourceFilePath );
        const InteropString sanitizedName     = AssetPathUtilities::SanitizeAssetName( assetName );
        const InteropString fontAssetFileName = AssetPathUtilities::CreateAssetFileName( context.AssetNamePrefix, sanitizedName, FontAsset::Extension( ) );

        std::string outputPath = context.TargetDirectory.Get( );
        if ( !outputPath.empty( ) && outputPath.back( ) != '/' && outputPath.back( ) != '\\' )
        {
            outputPath += '/';
        }
        outputPath += fontAssetFileName.Get( );

        BinaryContainer *container = context.Desc.TargetContainer;
        if ( container == nullptr )
        {
            container = new BinaryContainer( );
        }
        BinaryWriter writer( *container );

        FontAssetWriterDesc writerDesc{ };
        writerDesc.Writer = &writer;
        FontAssetWriter fontWriter( writerDesc );

        const FontAsset *msdfFontAsset = context.FontAsset;
        fontWriter.Write( *msdfFontAsset );
        fontWriter.End( );

        if ( context.Desc.TargetContainer == nullptr )
        {
            FileIO::WriteFile( outputPath.c_str( ), container->GetData( ) );
            outAssetUri.Path = outputPath.c_str( );
            delete container;
        }
    }

    Byte FloatToByte( const float &f )
    {
        return static_cast<Byte>( ~static_cast<int>( 255.5f - 255.f * msdfgen::clamp( f ) ) );
    }
} // anonymous namespace
