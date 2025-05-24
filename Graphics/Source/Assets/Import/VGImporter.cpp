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

#include <DenOfIzGraphics/Assets/FileSystem/FileIO.h>
#include <DenOfIzGraphics/Assets/FileSystem/PathResolver.h>
#include <DenOfIzGraphics/Assets/Import/AssetPathUtilities.h>
#include <DenOfIzGraphics/Assets/Import/VGImporter.h>
#include <DenOfIzGraphics/Assets/Serde/Texture/TextureAssetWriter.h>
#include <DenOfIzGraphics/Assets/Stream/BinaryWriter.h>
#include <DenOfIzGraphics/Utilities/Common_Asserts.h>
#include <filesystem>
#include <glog/logging.h>

#include <msdfgen-ext.h>
#include <msdfgen.h>

using namespace DenOfIz;

VGImporter::VGImporter( const VGImporterDesc desc ) : m_desc( desc )
{
    m_importerInfo.Name = "Vector Graphics Importer (Simplified)";
    m_importerInfo.SupportedExtensions.AddElement( ".svg" );
    m_msdfFtHandle = msdfgen::initializeFreetype( );
}

VGImporter::~VGImporter( )
{
    msdfgen::deinitializeFreetype( m_msdfFtHandle );
}

ImporterDesc VGImporter::GetImporterInfo( ) const
{
    return m_importerInfo;
}

bool VGImporter::CanProcessFileExtension( const InteropString &extension ) const
{
    const InteropString lowerExt = extension.ToLower( );
    for ( size_t i = 0; i < m_importerInfo.SupportedExtensions.NumElements( ); ++i )
    {
        if ( m_importerInfo.SupportedExtensions.GetElement( i ).Equals( lowerExt ) )
        {
            return true;
        }
    }
    return false;
}

ImporterResult VGImporter::Import( const ImportJobDesc &desc )
{
    LOG( INFO ) << "Starting vector graphics import for file: " << desc.SourceFilePath.Get( );

    ImportContext context;
    context.SourceFilePath  = desc.SourceFilePath;
    context.TargetDirectory = desc.TargetDirectory;
    context.AssetNamePrefix = desc.AssetNamePrefix;
    context.Desc            = *reinterpret_cast<VGImportDesc *>( desc.Desc );

    if ( context.Desc.Canvas && !context.SourceFilePath.IsEmpty( ) )
    {
        context.Result.ResultCode   = ImporterResultCode::InvalidParameters;
        context.Result.ErrorMessage = InteropString( "Cannot specify both Canvas and file path - use one or the other" );
        LOG( ERROR ) << context.Result.ErrorMessage.Get( );
        return context.Result;
    }

    if ( !context.Desc.Canvas )
    {
        if ( !FileIO::FileExists( context.SourceFilePath ) )
        {
            context.Result.ResultCode   = ImporterResultCode::FileNotFound;
            context.Result.ErrorMessage = InteropString( "Source file not found: " ).Append( context.SourceFilePath.Get( ) );
            LOG( ERROR ) << context.Result.ErrorMessage.Get( );
            return context.Result;
        }
    }

    if ( !FileIO::FileExists( context.TargetDirectory ) )
    {
        LOG( INFO ) << "Target directory does not exist, attempting to create: " << context.TargetDirectory.Get( );
        if ( !FileIO::CreateDirectories( context.TargetDirectory ) )
        {
            context.Result.ResultCode   = ImporterResultCode::WriteFailed;
            context.Result.ErrorMessage = InteropString( "Failed to create target directory: " ).Append( context.TargetDirectory.Get( ) );
            LOG( ERROR ) << context.Result.ErrorMessage.Get( );
            return context.Result;
        }
    }

    context.Result.ResultCode = ImportVGInternal( context );

    if ( context.Result.ResultCode == ImporterResultCode::Success )
    {
        LOG( INFO ) << "Vector graphics import successful for: " << context.SourceFilePath.Get( );
    }

    return context.Result;
}

bool VGImporter::ValidateFile( const InteropString &filePath ) const
{
    if ( !FileIO::FileExists( filePath ) )
    {
        return false;
    }

    const InteropString extension = AssetPathUtilities::GetFileExtension( filePath );
    return CanProcessFileExtension( extension );
}

ImporterResultCode VGImporter::ImportVGInternal( ImportContext &context )
{
    auto result = ImporterResultCode::Success;

    if ( context.Desc.Canvas )
    {
        if ( context.Desc.RenderMode != VGRenderMode::Raster )
        {
            LOG( WARNING ) << "Canvas input can only be rendered as raster - ignoring specified render mode";
        }
        context.Desc.RenderMode = VGRenderMode::Raster;
        result                  = RenderRasterToTexture( context );
    }
    else
    {
        if ( context.Desc.RenderMode == VGRenderMode::Raster )
        {
            LOG( WARNING ) << "Rendering SVG as raster - consider using MTSDF for better quality";
            result = RenderRasterToTexture( context );
        }
        else
        {
            context.Desc.RenderMode = VGRenderMode::MTSDF;
            result                  = RenderMSDFToTexture( context );
        }
    }

    if ( result != ImporterResultCode::Success )
    {
        return result;
    }

    AssetUri assetUri;
    WriteTextureAsset( context, context.TextureAsset, assetUri );
    RegisterCreatedAsset( context, assetUri );
    return ImporterResultCode::Success;
}

ImporterResultCode VGImporter::RenderRasterToTexture( ImportContext &context )
{
    const ThorVGCanvas *canvas = nullptr;

    if ( context.Desc.Canvas )
    {
        canvas = context.Desc.Canvas;
        if ( !canvas->Draw( ) || !canvas->Sync( ) )
        {
            context.ErrorMessage = InteropString( "Failed to render provided canvas" );
            return ImporterResultCode::ImportFailed;
        }
    }
    else
    {
        ThorVGCanvasDesc canvasDesc;
        canvasDesc.Width  = context.Desc.RenderWidth;
        canvasDesc.Height = context.Desc.RenderHeight;

        const auto tempCanvas = std::make_unique<ThorVGCanvas>( canvasDesc );
        canvas          = tempCanvas.get( );

        const auto picture = std::make_unique<ThorVGPicture>( );

        const std::string resolvedPath = PathResolver::ResolvePath( context.SourceFilePath.Get( ) );
        if ( !picture->Load( resolvedPath.c_str( ) ) )
        {
            context.ErrorMessage = InteropString( "Failed to load SVG file: " ).Append( context.SourceFilePath.Get( ) );
            return ImporterResultCode::ImportFailed;
        }

        const ThorVGBounds bounds = picture->GetBounds( false );
        const float        svgX   = bounds.X;
        const float        svgY   = bounds.Y;
        const float        svgW   = bounds.Width;
        const float        svgH   = bounds.Height;

        const float targetWidth  = static_cast<float>( context.Desc.RenderWidth - 2 * context.Desc.Padding );
        const float targetHeight = static_cast<float>( context.Desc.RenderHeight - 2 * context.Desc.Padding );
        const float scaleX       = targetWidth / svgW;
        const float scaleY       = targetHeight / svgH;
        const float scale        = std::min( scaleX, scaleY ) * context.Desc.Scale;

        const float scaledWidth  = svgW * scale;
        const float scaledHeight = svgH * scale;
        const float offsetX      = ( context.Desc.RenderWidth - scaledWidth ) / 2.0f;
        const float offsetY      = ( context.Desc.RenderHeight - scaledHeight ) / 2.0f;

        picture->Translate( offsetX - svgX * scale, offsetY - svgY * scale );
        picture->Scale( scale );
        if ( !canvas->Push( picture.get( ) ) )
        {
            context.ErrorMessage = InteropString( "Failed to push picture to canvas" );
            return ImporterResultCode::ImportFailed;
        }
        if ( !canvas->Draw( ) || !canvas->Sync( ) )
        {
            context.ErrorMessage = InteropString( "Failed to render SVG" );
            return ImporterResultCode::ImportFailed;
        }
    }

    const InteropArray<uint32_t> &canvasData = canvas->GetData( );

    context.TextureAsset.Width     = context.Desc.RenderWidth;
    context.TextureAsset.Height    = context.Desc.RenderHeight;
    context.TextureAsset.Depth     = 1;
    context.TextureAsset.MipLevels = 1;
    context.TextureAsset.ArraySize = 1;
    context.TextureAsset.Format    = context.Desc.OutputFormat;
    context.TextureAsset.Dimension = TextureDimension::Texture2D;

    // Create mip data
    TextureMip mip;
    mip.Width      = context.Desc.RenderWidth;
    mip.Height     = context.Desc.RenderHeight;
    mip.MipIndex   = 0;
    mip.ArrayIndex = 0;
    mip.RowPitch   = context.Desc.RenderWidth * 4; // 4 bytes per pixel (RGBA)
    mip.NumRows    = context.Desc.RenderHeight;
    mip.SlicePitch = mip.RowPitch * context.Desc.RenderHeight;
    mip.DataOffset = 0;

    context.TextureAsset.Mips.AddElement( mip );

    // Store canvas data for writing
    m_renderBuffer = canvasData;

    return ImporterResultCode::Success;
}

ImporterResultCode VGImporter::RenderMSDFToTexture( ImportContext &context )
{
    // MSDF generation only supports SVG files via msdfgen
    if ( context.SourceFilePath.IsEmpty( ) )
    {
        context.ErrorMessage = InteropString( "MSDF generation requires an SVG file path" );
        return ImporterResultCode::InvalidParameters;
    }

    // Load SVG shape using msdfgen
    msdfgen::Shape shape;
    if ( !msdfgen::loadSvgShape( shape, context.SourceFilePath.Get( ), 0 ) )
    {
        context.ErrorMessage = InteropString( "Failed to load SVG shape from file: " ).Append( context.SourceFilePath.Get( ) );
        return ImporterResultCode::ImportFailed;
    }

    // Normalize shape
    shape.normalize( );

    // Auto-frame shape with padding
    double left, bottom, right, top;
    shape.bound( left, bottom, right, top );

    const double padding = context.Desc.SDFPixelRange;
    const double scale   = std::min( ( context.Desc.RenderWidth - 2.0 * padding ) / ( right - left ), ( context.Desc.RenderHeight - 2.0 * padding ) / ( top - bottom ) );

    // Edge coloring
    msdfgen::edgeColoringByDistance( shape, context.Desc.SDFAngleThreshold );

    // Generate MTSDF
    msdfgen::Bitmap<float, 4> msdf( context.Desc.RenderWidth, context.Desc.RenderHeight );
    const msdfgen::Projection projection( { context.Desc.RenderWidth / 2.0 - scale * ( left + right ) / 2.0, context.Desc.RenderHeight / 2.0 - scale * ( bottom + top ) / 2.0 },
                                          scale );

    msdfgen::MSDFGeneratorConfig config;
    config.overlapSupport = true;

    msdfgen::generateMTSDF( msdf, shape, projection, context.Desc.SDFPixelRange, config );

    // Convert to RGBA format
    m_renderBuffer.Resize( context.Desc.RenderWidth * context.Desc.RenderHeight );
    for ( uint32_t y = 0; y < context.Desc.RenderHeight; ++y )
    {
        for ( uint32_t x = 0; x < context.Desc.RenderWidth; ++x )
        {
            const float *pixel = msdf( x, context.Desc.RenderHeight - 1 - y ); // Flip Y

            // Convert from [-1, 1] to [0, 255]
            const uint8_t  r      = static_cast<uint8_t>( msdfgen::clamp( pixel[ 0 ] * 0.5f + 0.5f, 0.0f, 1.0f ) * 255.0f );
            const uint8_t  g      = static_cast<uint8_t>( msdfgen::clamp( pixel[ 1 ] * 0.5f + 0.5f, 0.0f, 1.0f ) * 255.0f );
            const uint8_t  b      = static_cast<uint8_t>( msdfgen::clamp( pixel[ 2 ] * 0.5f + 0.5f, 0.0f, 1.0f ) * 255.0f );
            const uint8_t  a      = static_cast<uint8_t>( msdfgen::clamp( pixel[ 3 ] * 0.5f + 0.5f, 0.0f, 1.0f ) * 255.0f );
            const uint32_t packed = a << 24 | b << 16 | g << 8 | r;
            m_renderBuffer.SetElement( y * context.Desc.RenderWidth + x, packed );
        }
    }

    // Setup texture asset
    context.TextureAsset.Width     = context.Desc.RenderWidth;
    context.TextureAsset.Height    = context.Desc.RenderHeight;
    context.TextureAsset.Depth     = 1;
    context.TextureAsset.MipLevels = 1;
    context.TextureAsset.ArraySize = 1;
    context.TextureAsset.Format    = context.Desc.OutputFormat;
    context.TextureAsset.Dimension = TextureDimension::Texture2D;

    // Create mip data
    TextureMip mip;
    mip.Width      = context.Desc.RenderWidth;
    mip.Height     = context.Desc.RenderHeight;
    mip.MipIndex   = 0;
    mip.ArrayIndex = 0;
    mip.RowPitch   = context.Desc.RenderWidth * 4; // 4 bytes per pixel (RGBA)
    mip.NumRows    = context.Desc.RenderHeight;
    mip.SlicePitch = mip.RowPitch * context.Desc.RenderHeight;
    mip.DataOffset = 0;

    context.TextureAsset.Mips.AddElement( mip );

    return ImporterResultCode::Success;
}

void VGImporter::WriteTextureAsset( const ImportContext &context, const TextureAsset &textureAsset, AssetUri &outAssetUri ) const
{
    const InteropString assetName     = AssetPathUtilities::GetAssetNameFromFilePath( context.SourceFilePath );
    InteropString       sanitizedName = AssetPathUtilities::SanitizeAssetName( assetName );
    InteropString       suffix;
    switch ( context.Desc.RenderMode )
    {
    case VGRenderMode::SDF:
        suffix = "_sdf";
        break;
    case VGRenderMode::MSDF:
        suffix = "_msdf";
        break;
    case VGRenderMode::MTSDF:
        suffix = "_mtsdf";
        break;
    default:
        suffix = "";
        break;
    }

    const std::filesystem::path targetDirectory = context.TargetDirectory.Get( );
    const InteropString         nameWithSuffix  = sanitizedName.Append( suffix.Get( ) );
    const std::filesystem::path fileName        = AssetPathUtilities::CreateAssetFileName( context.AssetNamePrefix, nameWithSuffix, "dztex" ).Get( );
    const InteropString         filePath        = ( targetDirectory / fileName ).string( ).c_str( );

    BinaryWriter           writer( filePath );
    TextureAssetWriterDesc writerDesc{ };
    writerDesc.Writer = &writer;

    TextureAssetWriter textureWriter( writerDesc );
    textureWriter.Write( textureAsset );

    const uint32_t     pixelCount = context.Desc.RenderWidth * context.Desc.RenderHeight;
    InteropArray<Byte> textureData;
    textureData.Resize( pixelCount * 4 );

    for ( uint32_t i = 0; i < pixelCount; ++i )
    {
        const uint32_t argb = m_renderBuffer.GetElement( i );
        const uint8_t  a    = argb >> 24 & 0xFF;
        const uint8_t  r    = argb >> 16 & 0xFF;
        const uint8_t  g    = argb >> 8 & 0xFF;
        const uint8_t  b    = argb & 0xFF;

        // Store as RGBA
        textureData.SetElement( i * 4 + 0, r );
        textureData.SetElement( i * 4 + 1, g );
        textureData.SetElement( i * 4 + 2, b );
        textureData.SetElement( i * 4 + 3, a );
    }

    textureWriter.AddPixelData( textureData, 0, 0 );
    textureWriter.End( );
    writer.Flush( );

    outAssetUri.Path = filePath;
}

void VGImporter::RegisterCreatedAsset( ImportContext &context, const AssetUri &assetUri ) const
{
    context.Result.CreatedAssets.AddElement( assetUri );
}
