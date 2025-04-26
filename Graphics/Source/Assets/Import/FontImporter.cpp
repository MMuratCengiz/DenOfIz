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
    context.FontAsset.AtlasData.Resize( context.Options.AtlasWidth * context.Options.AtlasHeight * 3 );
    memset( context.FontAsset.AtlasData.Data( ), 0, context.FontAsset.AtlasData.NumElements( ) );

    context.FontAsset.InitialFontSize   = context.Options.InitialFontSize;
    context.FontAsset.AntiAliasing      = context.Options.AntiAliasing;
    context.FontAsset.AtlasWidth        = context.Options.AtlasWidth;
    context.FontAsset.AtlasHeight       = context.Options.AtlasHeight;
    context.FontAsset.NumAtlasDataBytes = context.Options.AtlasWidth * context.Options.AtlasHeight * 3;
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
    PreloadCharacterSet( context, face );

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

void FontImporter::PreloadCharacterSet( ImportContext &context, const FT_Face face )
{
    // Load only ASCII for now
    for ( uint32_t c = 32; c < 127; c++ )
    {
        LoadGlyph( context, face, c );
    }
}

bool FontImporter::LoadGlyph( ImportContext &context, const FT_Face face, const uint32_t codePoint )
{
    static const std::unordered_set<uint32_t> ignoredGlyphs = { '\n' };
    if ( ignoredGlyphs.contains( codePoint ) )
    {
        return true;
    }
    for ( uint32_t i = 0; i < context.FontAsset.Glyphs.NumElements( ); ++i )
    {
        if ( context.FontAsset.Glyphs.GetElement( i ).CodePoint == codePoint )
        {
            return true;
        }
    }
    const FT_UInt glyphIndex = FT_Get_Char_Index( face, codePoint );
    if ( glyphIndex == 0 )
    {
        LOG( WARNING ) << "Glyph not found for code point: " << codePoint;
        return false;
    }
    if ( const FT_Error error = FT_Load_Glyph( face, glyphIndex, FT_LOAD_DEFAULT ) )
    {
        LOG( ERROR ) << "Failed to load glyph: " << FT_Error_String( error );
        return false;
    }
    const FT_GlyphSlot slot = face->glyph;

    // For empty glyphs like spaces
    if ( slot->metrics.width == 0 || slot->metrics.height == 0 )
    {
        FontGlyph glyphDesc;
        glyphDesc.CodePoint = codePoint;
        glyphDesc.Width     = 0;
        glyphDesc.Height    = 0;
        glyphDesc.BearingX  = slot->metrics.horiBearingX >> 6;
        glyphDesc.BearingY  = slot->metrics.horiBearingY >> 6;
        glyphDesc.XAdvance  = slot->advance.x >> 6;
        glyphDesc.YAdvance  = slot->advance.y >> 6;
        glyphDesc.AtlasX    = 0;
        glyphDesc.AtlasY    = 0;

        context.FontAsset.Glyphs.AddElement( glyphDesc );
        return true;
    }

    const Byte          *data         = context.FontAsset.Data.Data( );
    const uint64_t       dataNumBytes = context.FontAsset.DataNumBytes;
    msdfgen::FontHandle *msdfFont     = msdfgen::loadFontData( m_msdfFtHandle, data, dataNumBytes );
    if ( !msdfFont )
    {
        LOG( ERROR ) << "Failed to load MSDF font for glyph generation";
        return false;
    }

    FontGlyph glyphDesc;
    glyphDesc.CodePoint = codePoint;
    glyphDesc.BearingX  = slot->metrics.horiBearingX >> 6;
    glyphDesc.BearingY  = slot->metrics.horiBearingY >> 6;
    glyphDesc.XAdvance  = slot->advance.x >> 6;
    glyphDesc.YAdvance  = slot->advance.y >> 6;
    glyphDesc.Width     = slot->metrics.width >> 6;
    glyphDesc.Height    = slot->metrics.height >> 6;

    const bool success = GenerateMsdfForGlyph( glyphDesc, msdfFont, codePoint, context.Options.InitialFontSize );
    msdfgen::destroyFont( msdfFont );

    if ( !success )
    {
        LOG( ERROR ) << "Failed to generate MSDF for glyph: " << codePoint;
        return false;
    }

    // Skip empty shapes (i.e., space)
    if ( glyphDesc.Width == 0 || glyphDesc.Height == 0 )
    {
        context.FontAsset.Glyphs.AddElement( glyphDesc );
        return true;
    }

    const Rect rect = AllocateSpace( context, glyphDesc.Width, glyphDesc.Height );
    CopyMsdfDataToAtlas( context, glyphDesc, rect );

    glyphDesc.AtlasX = rect.X;
    glyphDesc.AtlasY = rect.Y;

    context.FontAsset.Glyphs.AddElement( glyphDesc );
    return true;
}

bool FontImporter::GenerateMsdfForGlyph( FontGlyph &glyphDesc, msdfgen::FontHandle *msdfFont, const uint32_t codePoint, const uint32_t fontSize ) const
{
    msdfgen::Shape shape;
    if ( !msdfgen::loadGlyph( shape, msdfFont, codePoint, msdfgen::FONT_SCALING_EM_NORMALIZED ) ) // Removed advance here
    {
        LOG( ERROR ) << "Failed to load glyph shape for MSDF generation: " << codePoint;
        return false;
    }

    shape.inverseYAxis = true;
    shape.normalize( );
    shape.orientContours( );

    if ( shape.contours.empty( ) ) // Skip empty shapes (i.e., space)
    {
        glyphDesc.Width  = 0;
        glyphDesc.Height = 0;
        return true;
    }

    constexpr double             pixelRange = Font::MsdfPixelRange;
    const msdfgen::Shape::Bounds bounds     = shape.getBounds( );
    const double scale = fontSize;

    const uint32_t bitmapWidth  = static_cast<uint32_t>( std::round( ( bounds.r - bounds.l ) * scale ) ) + 2 * static_cast<uint32_t>( std::ceil( pixelRange ) );
    const uint32_t bitmapHeight = static_cast<uint32_t>( std::round( ( bounds.t - bounds.b ) * scale ) ) + 2 * static_cast<uint32_t>( std::ceil( pixelRange ) );

    const msdfgen::Projection projection( scale, msdfgen::Vector2( -bounds.l, -bounds.b ) );
    const msdfgen::Range      range( Font::MsdfPixelRange / scale );

    msdfgen::Bitmap<float, 3>    msdf( bitmapWidth, bitmapHeight );
    msdfgen::MSDFGeneratorConfig config;
    config.errorCorrection.mode = msdfgen::ErrorCorrectionConfig::EDGE_PRIORITY;
    const msdfgen::SDFTransformation transform( projection, range );
    msdfgen::generateMSDF( msdf, shape, transform, config );
    msdfgen::simulate8bit( msdf );

    glyphDesc.Width       = msdf.width( );
    glyphDesc.Height      = msdf.height( );
    glyphDesc.Bounds.XMin = bounds.l;
    glyphDesc.Bounds.XMax = bounds.r;
    glyphDesc.Bounds.YMin = bounds.b;
    glyphDesc.Bounds.YMax = bounds.t;

    InteropArray<Byte> msdfData( glyphDesc.Width * glyphDesc.Height * 3 );
    const uint32_t     pitch = glyphDesc.Width * 3; // 3 bytes per pixel (RGB)
    glyphDesc.Pitch          = pitch;

    for ( uint32_t y = 0; y < glyphDesc.Height; y++ )
    {
        for ( uint32_t x = 0; x < glyphDesc.Width; x++ )
        {
            const uint32_t pixelOffset = y * pitch + x * 3;
            const float   *msdfPixel   = msdf( x, y );

            msdfData.SetElement( pixelOffset, static_cast<uint8_t>( msdfPixel[ 0 ] * 255.f ) );     // R
            msdfData.SetElement( pixelOffset + 1, static_cast<uint8_t>( msdfPixel[ 1 ] * 255.f ) ); // G
            msdfData.SetElement( pixelOffset + 2, static_cast<uint8_t>( msdfPixel[ 2 ] * 255.f ) ); // B
        }
    }

    glyphDesc.Data = std::move( msdfData );
    return true;
}

FontImporter::Rect FontImporter::AllocateSpace( ImportContext &context, const uint32_t width, const uint32_t height )
{
    // If we can't fit on the current row, move to the next row
    if ( context.CurrentAtlasX + width > context.FontAsset.AtlasWidth )
    {
        context.CurrentAtlasX = 0;
        context.CurrentAtlasY += context.RowHeight;
        context.RowHeight = 0;
    }

    // Resize if we can't fit in the atlas
    if ( context.CurrentAtlasY + height > context.FontAsset.AtlasHeight )
    {
        context.FontAsset.AtlasHeight *= 2;
        context.FontAsset.AtlasData.Resize( context.FontAsset.AtlasWidth * context.FontAsset.AtlasHeight * 3 ); // 3 bytes per pixel for RGB
        memset( context.FontAsset.AtlasData.Data( ), 0, context.FontAsset.AtlasData.NumElements( ) );
        context.FontAsset.Glyphs.Resize( 0 );
        context.CurrentAtlasX = 0;
        context.CurrentAtlasY = 0;
        context.RowHeight     = 0;

        LOG( WARNING ) << "Font atlas resized to " << context.FontAsset.AtlasWidth << "x" << context.FontAsset.AtlasHeight;
    }

    Rect rect;
    rect.X      = context.CurrentAtlasX;
    rect.Y      = context.CurrentAtlasY;
    rect.Width  = width;
    rect.Height = height;

    context.CurrentAtlasX += width;
    context.RowHeight = std::max( context.RowHeight, height );
    return rect;
}

void FontImporter::CopyMsdfDataToAtlas( ImportContext &context, const FontGlyph &glyphDesc, const Rect &rect )
{
    // Skip empty glyphs
    if ( glyphDesc.Width == 0 || glyphDesc.Height == 0 || glyphDesc.Data.NumElements( ) == 0 )
    {
        return;
    }

    auto &atlasData = context.FontAsset.AtlasData;
    for ( uint32_t y = 0; y < rect.Height; y++ )
    {
        for ( uint32_t x = 0; x < rect.Width; x++ )
        {
            const uint32_t atlasOffset = ( ( rect.Y + y ) * context.FontAsset.AtlasWidth + ( rect.X + x ) ) * 3;
            if ( atlasOffset + 2 >= atlasData.NumElements( ) )
            {
                LOG( ERROR ) << "Atlas offset out of bounds: " << atlasOffset << " >= " << atlasData.NumElements( );
                continue;
            }

            const uint32_t srcOffset = y * glyphDesc.Pitch + x * 3;
            if ( srcOffset + 2 >= glyphDesc.Data.NumElements( ) )
            {
                LOG( ERROR ) << "MSDF source offset out of bounds: " << srcOffset << " >= " << glyphDesc.Data.NumElements( );
                continue;
            }

            atlasData.SetElement( atlasOffset, glyphDesc.Data.GetElement( srcOffset ) );         // R
            atlasData.SetElement( atlasOffset + 1, glyphDesc.Data.GetElement( srcOffset + 1 ) ); // G
            atlasData.SetElement( atlasOffset + 2, glyphDesc.Data.GetElement( srcOffset + 2 ) ); // B
        }
    }
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

    FontAssetWriterDesc writerDesc;
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
