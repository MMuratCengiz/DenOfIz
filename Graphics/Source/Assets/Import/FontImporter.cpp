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
// ReSharper disable CppMemberFunctionMayBeStatic
#include <DenOfIzGraphics/Assets/FileSystem/FileIO.h>
#include <DenOfIzGraphics/Assets/FileSystem/PathResolver.h>
#include <DenOfIzGraphics/Assets/Import/FontImporter.h>
#include <DenOfIzGraphics/Utilities/Common_Asserts.h>
#include <algorithm>
#include <unordered_set>

#include "DenOfIzGraphics/Assets/Font/Font.h"
#include "msdf-atlas-gen/AtlasGenerator.h"
#include "msdf-atlas-gen/BitmapAtlasStorage.h"
#include "msdf-atlas-gen/FontGeometry.h"
#include "msdf-atlas-gen/ImmediateAtlasGenerator.h"
#include "msdf-atlas-gen/TightAtlasPacker.h"
#include "msdf-atlas-gen/glyph-generators.h"

using namespace DenOfIz;

FontImporter::FontImporter( FontImporterDesc desc ) : m_desc( std::move( desc ) )
{
    if ( const FT_Error error = FT_Init_FreeType( &m_ftLibrary ); error != 0 )
    {
        LOG( FATAL ) << "Failed to initialize FreeType library: " << FT_Error_String( error );
    }

    m_msdfFtHandle = msdfgen::initializeFreetype( );
    if ( !m_msdfFtHandle )
    {
        LOG( FATAL ) << "Failed to initialize MSDF Freetype library";
    }

    m_importerDesc.Name = "Font Importer";
    m_importerDesc.SupportedExtensions.AddElement( "ttf" );
    m_importerDesc.SupportedExtensions.AddElement( "otf" );
    m_importerDesc.SupportedExtensions.AddElement( "ttc" );
}

FontImporter::~FontImporter( )
{
    if ( m_msdfFtHandle )
    {
        msdfgen::deinitializeFreetype( m_msdfFtHandle );
        m_msdfFtHandle = nullptr;
    }

    FT_Done_FreeType( m_ftLibrary );
}

ImporterDesc FontImporter::GetImporterInfo( ) const
{
    return m_importerDesc;
}

bool FontImporter::CanProcessFileExtension( const InteropString &extension ) const
{
    const std::string ext = extension.Get( );
    for ( int i = 0; i < m_importerDesc.SupportedExtensions.NumElements( ); ++i )
    {
        if ( strcmp( extension.Get( ), m_importerDesc.SupportedExtensions.GetElement( i ).Get( ) ) == 0 )
        {
            return true;
        }
    }
    return false;
}

ImporterResult FontImporter::Import( const ImportJobDesc &desc )
{
    ImportContext context;
    context.SourceFilePath    = desc.SourceFilePath;
    context.TargetDirectory   = desc.TargetDirectory;
    context.AssetNamePrefix   = desc.AssetNamePrefix;
    context.Options           = FontImportDesc::CreateFromBase( desc.Options );
    context.Result.ResultCode = ImporterResultCode::Success;

    // For MSDF, we need RGB data (3 bytes per pixel) instead of grayscale
    context.FontAsset.AtlasData.Resize( context.Options.AtlasWidth * context.Options.AtlasHeight * FontAsset::NumChannels );
    memset( context.FontAsset.AtlasData.Data( ), 0, context.FontAsset.AtlasData.NumElements( ) );

    context.FontAsset.InitialFontSize   = context.Options.InitialFontSize;
    context.FontAsset.AtlasWidth        = context.Options.AtlasWidth;
    context.FontAsset.AtlasHeight       = context.Options.AtlasHeight;
    context.FontAsset.NumAtlasDataBytes = context.Options.AtlasWidth * context.Options.AtlasHeight * FontAsset::NumChannels;
    context.FontAsset.AtlasData.Resize( context.FontAsset.NumAtlasDataBytes );

    if ( const ImporterResultCode result = ImportFontInternal( context ); result != ImporterResultCode::Success )
    {
        context.Result.ResultCode   = result;
        context.Result.ErrorMessage = context.ErrorMessage;
        return context.Result;
    }

    AssetUri fontAssetUri;
    WriteFontAsset( context, fontAssetUri );
    context.Result.CreatedAssets.AddElement( fontAssetUri );
    return context.Result;
}

bool FontImporter::ValidateFile( const InteropString &filePath ) const
{
    FT_Face           face;
    const std::string resolvedPath = PathResolver::ResolvePath( filePath.Get( ) );

    if ( FT_New_Face( m_ftLibrary, resolvedPath.c_str( ), 0, &face ) )
    {
        return false;
    }

    FT_Done_Face( face );
    return true;
}

ImporterResultCode FontImporter::ImportFontInternal( ImportContext &context )
{
    FT_Face           face;
    const std::string resolvedPath = PathResolver::ResolvePath( context.SourceFilePath.Get( ) );

    context.FontAsset.Data         = FileIO::ReadFile( InteropString( resolvedPath.c_str( ) ) );
    context.FontAsset.DataNumBytes = context.FontAsset.Data.NumElements( );

    FT_Error error = FT_New_Face( m_ftLibrary, resolvedPath.c_str( ), 0, &face );

    if ( error )
    {
        context.ErrorMessage = InteropString( "Failed to load font: " ).Append( FT_Error_String( error ) );
        return ImporterResultCode::InvalidParameters;
    }

    error = FT_Set_Char_Size( face, context.Options.InitialFontSize * 64, context.Options.InitialFontSize * 64, 0, 0 );
    if ( error )
    {
        FT_Done_Face( face );
        context.ErrorMessage = InteropString( "Failed to set font size: " ).Append( FT_Error_String( error ) );
        return ImporterResultCode::InvalidParameters;
    }

    ExtractFontMetrics( context, face );
    GenerateAtlas( context );

    FT_Done_Face( face );
    return ImporterResultCode::Success;
}

void FontImporter::ExtractFontMetrics( ImportContext &context, const FT_Face face )
{
    context.FontAsset.Metrics.Ascent     = static_cast<uint32_t>( face->size->metrics.ascender >> 6 );
    context.FontAsset.Metrics.Descent    = static_cast<uint32_t>( abs( face->size->metrics.descender ) >> 6 );
    context.FontAsset.Metrics.LineGap    = static_cast<uint32_t>( ( face->size->metrics.height - ( face->size->metrics.ascender - face->size->metrics.descender ) ) >> 6 );
    context.FontAsset.Metrics.LineHeight = static_cast<uint32_t>( face->size->metrics.height >> 6 );

    if ( FT_IS_SCALABLE( face ) )
    {
        context.FontAsset.Metrics.UnderlinePos       = static_cast<uint32_t>( abs( face->underline_position ) >> 6 );
        context.FontAsset.Metrics.UnderlineThickness = static_cast<uint32_t>( face->underline_thickness >> 6 );
    }
    else
    {
        context.FontAsset.Metrics.UnderlinePos       = context.FontAsset.Metrics.Descent / 2;
        context.FontAsset.Metrics.UnderlineThickness = context.FontAsset.Metrics.Ascent / 20;
    }
}

void FontImporter::GenerateAtlas( ImportContext &context ) const
{
    const Byte          *data         = context.FontAsset.Data.Data( );
    const uint64_t       dataNumBytes = context.FontAsset.DataNumBytes;
    msdfgen::FontHandle *msdfFont     = msdfgen::loadFontData( m_msdfFtHandle, data, dataNumBytes );
    if ( !msdfFont )
    {
        LOG( ERROR ) << "Failed to load MSDF font for glyph generation";
        return;
    }

    std::vector<msdf_atlas::GlyphGeometry> glyphs;
    msdf_atlas::FontGeometry fontGeometry( &glyphs );

    fontGeometry.loadCharset( msdfFont, 1.0, msdf_atlas::Charset::ASCII );

    for ( msdf_atlas::GlyphGeometry &glyph : glyphs )
    {
        constexpr double maxCornerAngle = 3.0;
        glyph.edgeColoring( &msdfgen::edgeColoringByDistance, maxCornerAngle, 0 );
    }

    msdf_atlas::TightAtlasPacker packer;
    packer.setDimensionsConstraint( msdf_atlas::DimensionsConstraint::SQUARE );
    packer.setMinimumScale( context.Options.InitialFontSize );
    packer.setPixelRange( Font::MsdfPixelRange );
    packer.setMiterLimit( 1.0 );
    packer.pack( glyphs.data( ), glyphs.size( ) );

    int width = 0, height = 0;
    packer.getDimensions( width, height );

    msdf_atlas::GeneratorAttributes attributes;
    attributes.config.overlapSupport = true;

    msdf_atlas::ImmediateAtlasGenerator<float, 3, msdf_atlas::msdfGenerator, msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 3>> generator( width, height );

    generator.setAttributes( attributes );
    generator.setThreadCount( 4 );

    // Generate Atlas:
    generator.generate( glyphs.data( ), glyphs.size( ) );

    const auto &atlasStorage = generator.atlasStorage( );

    // Resize atlas if necessary
    if ( width != static_cast<int>( context.FontAsset.AtlasWidth ) || height != static_cast<int>( context.FontAsset.AtlasHeight ) )
    {
        context.FontAsset.AtlasWidth        = width;
        context.FontAsset.AtlasHeight       = height;
        context.FontAsset.NumAtlasDataBytes = width * height * FontAsset::NumChannels;
        context.FontAsset.AtlasData.Clear( );
        context.FontAsset.AtlasData.Resize( context.FontAsset.NumAtlasDataBytes );
    }

    const msdfgen::BitmapConstRef<msdfgen::byte, 3> &bitmap    = atlasStorage;
    const msdfgen::byte                             *pixels    = bitmap.pixels;
    auto                                            &atlasData = context.FontAsset.AtlasData;

    for ( int y = 0; y < height; y++ )
    {
        int invertedY = height - 1 - y;
        for ( int x = 0; x < width; x++ )
        {
            const int      srcIdx = 3 * ( invertedY * width + x );
            const uint32_t dstIdx = FontAsset::NumChannels * ( y * width + x );
            atlasData.SetElement( dstIdx, pixels[ srcIdx ] );
            atlasData.SetElement( dstIdx + 1, pixels[ srcIdx + 1 ] );
            atlasData.SetElement( dstIdx + 2, pixels[ srcIdx + 2 ] );
            atlasData.SetElement( dstIdx + 3, 255 );
        }
    }

    const auto &layout = generator.getLayout( );

    double emSize     = fontGeometry.getMetrics( ).emSize;
    double ascender   = fontGeometry.getMetrics( ).ascenderY;
    double descender  = fontGeometry.getMetrics( ).descenderY;
    double lineHeight = fontGeometry.getMetrics( ).lineHeight;
    double scale      = context.Options.InitialFontSize / emSize;

    context.FontAsset.Metrics.Ascent     = static_cast<uint32_t>( ascender * scale );
    context.FontAsset.Metrics.Descent    = static_cast<uint32_t>( std::abs( descender ) * scale );
    context.FontAsset.Metrics.LineHeight = static_cast<uint32_t>( lineHeight * scale );
    context.FontAsset.Metrics.LineGap    = context.FontAsset.Metrics.LineHeight - ( context.FontAsset.Metrics.Ascent + context.FontAsset.Metrics.Descent );

    for ( size_t i = 0; i < layout.size( ); i++ )
    {
        const auto &box   = layout[ i ];
        const auto &glyph = glyphs[ i ];
        if ( glyph.isWhitespace( ) )
        {
            continue;
        }

        FontGlyph &glyphDesc = context.FontAsset.Glyphs.EmplaceElement( );
        glyphDesc.CodePoint  = glyph.getCodepoint( );
        glyphDesc.Width      = box.rect.w;
        glyphDesc.Height     = box.rect.h;
        glyphDesc.AtlasX     = box.rect.x;
        glyphDesc.AtlasY     = height - ( box.rect.y + box.rect.h );

        const msdfgen::Shape  &shape    = glyph.getShape( );
        msdfgen::Shape::Bounds bounds   = shape.getBounds( );

        constexpr double normFactor = 1000.0;
        glyphDesc.Bounds.XMin   = bounds.l / normFactor;
        glyphDesc.Bounds.XMax   = bounds.r / normFactor;
        glyphDesc.Bounds.YMin   = bounds.b / normFactor;
        glyphDesc.Bounds.YMax   = bounds.t / normFactor;

        // Convert font to msdf Size
        const double emScale = context.Options.InitialFontSize / emSize;

        const double scaledBoundsL = bounds.l / normFactor * context.Options.InitialFontSize;
        glyphDesc.BearingX         = static_cast<int32_t>( scaledBoundsL );
        const double scaledBoundsT = bounds.t / normFactor * context.Options.InitialFontSize;
        glyphDesc.BearingY         = static_cast<int32_t>( scaledBoundsT );

        glyphDesc.XAdvance = static_cast<uint32_t>( glyph.getAdvance( ) * emScale );
        glyphDesc.YAdvance = 0; // TODO
    }

    FontGlyph spaceGlyph;
    spaceGlyph.CodePoint = ' ';
    spaceGlyph.Width     = 0;
    spaceGlyph.Height    = 0;
    spaceGlyph.BearingX  = 0;
    spaceGlyph.BearingY  = 0;
    spaceGlyph.XAdvance  = context.Options.InitialFontSize / 3;
    spaceGlyph.YAdvance  = 0;
    spaceGlyph.AtlasX    = 0;
    spaceGlyph.AtlasY    = 0;
    context.FontAsset.Glyphs.AddElement( spaceGlyph );

    msdfgen::destroyFont( msdfFont );
}

void FontImporter::WriteFontAsset( const ImportContext &context, AssetUri &outAssetUri ) const
{
    const InteropString assetName         = GetAssetNameFromFilePath( context.SourceFilePath );
    const InteropString sanitizedName     = SanitizeAssetName( assetName );
    const InteropString fontAssetFileName = CreateAssetFileName( context.AssetNamePrefix, sanitizedName );

    std::string outputPath = context.TargetDirectory.Get( );
    if ( !outputPath.empty( ) && outputPath.back( ) != '/' && outputPath.back( ) != '\\' )
    {
        outputPath += '/';
    }
    outputPath += fontAssetFileName.Get( );

    BinaryContainer container{ };
    BinaryWriter    writer( container );

    FontAssetWriterDesc writerDesc{ };
    writerDesc.Writer = &writer;
    FontAssetWriter fontWriter( writerDesc );

    FontAsset msdfFontAsset = context.FontAsset;
    fontWriter.Write( msdfFontAsset );
    fontWriter.Finalize( );

    FileIO::WriteFile( outputPath.c_str( ), container.GetData( ) );

    outAssetUri.Path = outputPath.c_str( );
}

InteropString FontImporter::CreateAssetFileName( const InteropString &prefix, const InteropString &name ) const
{
    std::string result;

    if ( prefix.NumChars( ) > 0 )
    {
        result += prefix.Get( );
        result += "_";
    }

    result += name.Get( );
    result += ".";
    result += m_fileExtension.Get( );

    return result.c_str( );
}

InteropString FontImporter::GetAssetNameFromFilePath( const InteropString &filePath )
{
    const std::string path      = filePath.Get( );
    size_t            lastSlash = path.find_last_of( "/\\" );
    if ( lastSlash == std::string::npos )
    {
        lastSlash = 0;
    }
    else
    {
        lastSlash += 1;
    }

    size_t lastDot = path.find_last_of( '.' );
    if ( lastDot == std::string::npos || lastDot < lastSlash )
    {
        lastDot = path.length( );
    }

    return path.substr( lastSlash, lastDot - lastSlash ).c_str( );
}

InteropString FontImporter::SanitizeAssetName( const InteropString &name )
{
    std::string sanitized = name.Get( );

    // Replace invalid characters with underscores
    for ( char &c : sanitized )
    {
        if ( !std::isalnum( c ) && c != '_' && c != '-' )
        {
            c = '_';
        }
    }

    return sanitized.c_str( );
}

Byte FontImporter::FloatToByte( const float &f )
{
    return static_cast<Byte>( ~static_cast<int>( 255.5f - 255.f * msdfgen::clamp( f ) ) );
}
