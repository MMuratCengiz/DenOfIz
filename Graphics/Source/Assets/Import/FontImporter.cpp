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
#include <../../../Include/DenOfIzGraphics/Assets/Import/FontImporter.h>
#include <DenOfIzGraphics/Assets/FileSystem/FileIO.h>
#include <DenOfIzGraphics/Assets/FileSystem/PathResolver.h>
#include <DenOfIzGraphics/Utilities/Common_Asserts.h>
#include <algorithm>
#include <ft2build.h>
#include <unordered_set>

#include FT_FREETYPE_H

using namespace DenOfIz;

FontImporter::FontImporter( FontImporterDesc desc ) : m_desc( std::move( desc ) )
{
    if ( const FT_Error error = FT_Init_FreeType( &m_ftLibrary ); error != 0 )
    {
        LOG( FATAL ) << "Failed to initialize FreeType library: " << FT_Error_String( error );
    }

    m_importerDesc.Name = "Font Importer";
    m_importerDesc.SupportedExtensions.AddElement( "ttf" );
    m_importerDesc.SupportedExtensions.AddElement( "otf" );
    m_importerDesc.SupportedExtensions.AddElement( "ttc" );
}

FontImporter::~FontImporter( )
{
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
    context.Options           = FontImportOptions::CreateFromBase( desc.Options );
    context.Result.ResultCode = ImporterResultCode::Success;

    context.AtlasBitmap.Resize( context.Options.AtlasWidth * context.Options.AtlasHeight );
    memset( context.AtlasBitmap.Data( ), 0, context.AtlasBitmap.NumElements( ) );

    context.FontAsset.PixelSize            = context.Options.PixelSize;
    context.FontAsset.AntiAliasing         = context.Options.AntiAliasing;
    context.FontAsset.AtlasWidth           = context.Options.AtlasWidth;
    context.FontAsset.AtlasHeight          = context.Options.AtlasHeight;
    context.FontAsset.AtlasBitmap.NumBytes = context.Options.AtlasWidth * context.Options.AtlasHeight;
    context.FontAsset.FontPath             = context.SourceFilePath.Get( );

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
    FT_Error          error        = FT_New_Face( m_ftLibrary, resolvedPath.c_str( ), 0, &face );

    if ( error )
    {
        context.ErrorMessage = InteropString( "Failed to load font: " ).Append( FT_Error_String( error ) );
        return ImporterResultCode::InvalidParameters;
    }

    error = FT_Set_Pixel_Sizes( face, 0, context.Options.PixelSize );
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
    if ( context.Options.PreloadCharacterSet.NumElements( ) > 0 )
    {
        for ( uint32_t i = 0; i < context.Options.PreloadCharacterSet.NumElements( ); ++i )
        {
            LoadGlyph( context, face, context.Options.PreloadCharacterSet.GetElement( i ) );
        }
    }
    else
    {
        for ( uint32_t c = 32; c < 127; c++ )
        {
            LoadGlyph( context, face, c );
        }
    }
}

bool FontImporter::LoadGlyph( ImportContext &context, const FT_Face face, const uint32_t codePoint )
{
    static const std::unordered_set<uint32_t> ignoredGlyphs = { '\n' };
    if ( ignoredGlyphs.contains( codePoint ) )
    {
        return true;
    }
    for ( uint32_t i = 0; i < context.FontAsset.GlyphData.NumElements( ); ++i )
    {
        if ( context.FontAsset.GlyphData.GetElement( i ).CodePoint == codePoint )
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

    FT_Int32 loadFlags = FT_LOAD_DEFAULT;
    if ( context.FontAsset.AntiAliasing )
    {
        loadFlags |= FT_LOAD_RENDER;
    }
    else
    {
        loadFlags |= FT_LOAD_RENDER | FT_LOAD_MONOCHROME;
    }

    if ( const FT_Error error = FT_Load_Glyph( face, glyphIndex, loadFlags ) )
    {
        LOG( ERROR ) << "Failed to load glyph: " << FT_Error_String( error );
        return false;
    }

    const FT_GlyphSlot slot   = face->glyph;
    const FT_Bitmap    bitmap = slot->bitmap;

    // For empty glyphs like spaces
    if ( bitmap.width == 0 || bitmap.rows == 0 )
    {
        GlyphMetrics metrics;
        metrics.CodePoint = codePoint;
        metrics.Width     = 0;
        metrics.Height    = 0;
        metrics.BearingX  = slot->bitmap_left;
        metrics.BearingY  = slot->bitmap_top;
        metrics.Advance   = slot->advance.x >> 6; // Convert from 26.6 fixed-point format
        metrics.AtlasX    = 0;
        metrics.AtlasY    = 0;

        context.FontAsset.GlyphData.AddElement( metrics );
        return true;
    }

    const Rect rect = AllocateSpace( context, bitmap.width, bitmap.rows );
    CopyGlyphToAtlas( context, face, rect );


    GlyphMetrics glyphMetrics;
    glyphMetrics.CodePoint = codePoint;
    glyphMetrics.Width     = slot->metrics.width;
    glyphMetrics.Height    = slot->metrics.height;
    glyphMetrics.BearingX  = slot->bitmap_left;
    glyphMetrics.BearingY  = slot->bitmap_top;
    glyphMetrics.Advance   = slot->advance.x >> 6; // Convert from 26.6 fixed-point format
    glyphMetrics.AtlasX    = rect.X;
    glyphMetrics.AtlasY    = rect.Y;

    context.FontAsset.GlyphData.AddElement( glyphMetrics );
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
        context.AtlasBitmap.Resize( context.FontAsset.AtlasWidth * context.FontAsset.AtlasHeight );
        memset( context.AtlasBitmap.Data( ), 0, context.AtlasBitmap.NumElements( ) );
        context.FontAsset.GlyphData.Resize( 0 );
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

void FontImporter::CopyGlyphToAtlas( ImportContext &context, const FT_Face face, const Rect &rect )
{
    const FT_GlyphSlot slot   = face->glyph;
    const FT_Bitmap    bitmap = slot->bitmap;

    for ( uint32_t y = 0; y < bitmap.rows; y++ )
    {
        for ( uint32_t x = 0; x < bitmap.width; x++ )
        {
            const uint32_t atlasOffset = ( rect.Y + y ) * context.FontAsset.AtlasWidth + ( rect.X + x );
            if ( atlasOffset >= context.AtlasBitmap.NumElements( ) )
            {
                LOG( ERROR ) << "Atlas offset out of bounds: " << atlasOffset << " >= " << context.AtlasBitmap.NumElements( );
                continue;
            }

            if ( bitmap.pixel_mode == FT_PIXEL_MODE_GRAY )
            {
                context.AtlasBitmap.SetElement( atlasOffset, bitmap.buffer[ y * bitmap.pitch + x ] );
            }
            else if ( bitmap.pixel_mode == FT_PIXEL_MODE_MONO )
            {
                const uint8_t byte = bitmap.buffer[ y * bitmap.pitch + x / 8 ];
                const uint8_t bit  = byte >> ( 7 - x % 8 ) & 1;
                context.AtlasBitmap.SetElement( atlasOffset, bit ? 255 : 0 );
            }
        }
    }
}

void FontImporter::WriteFontAsset( const ImportContext &context, AssetUri &outAssetUri )
{
    const InteropString assetName         = GetAssetNameFromFilePath( context.SourceFilePath );
    const InteropString sanitizedName     = SanitizeAssetName( assetName );
    const InteropString fontAssetFileName = CreateAssetFileName( context.AssetNamePrefix, sanitizedName, "font" );

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

    fontWriter.Write( context.FontAsset, context.AtlasBitmap );
    FileIO::WriteFile( outputPath.c_str( ), container.GetData( ) );

    outAssetUri.Path = outputPath.c_str( );
}

InteropString FontImporter::CreateAssetFileName( const InteropString &prefix, const InteropString &name, const InteropString &extension )
{
    std::string result;

    if ( prefix.NumChars( ) > 0 )
    {
        result += prefix.Get( );
        result += "_";
    }

    result += name.Get( );
    result += ".";
    result += extension.Get( );

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
