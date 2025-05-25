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

using namespace DenOfIz;

VGImporter::VGImporter( const VGImporterDesc desc ) : m_desc( desc )
{
    m_importerInfo.Name = "Vector Graphics Importer (Simplified)";
    m_importerInfo.SupportedExtensions.AddElement( ".svg" );
}

VGImporter::~VGImporter( )
{
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
    if ( context.Desc.Canvas )
    {
        const ThorVGCanvas *canvas = context.Desc.Canvas;
        if ( !canvas->Draw( ) || !canvas->Sync( ) )
        {
            context.ErrorMessage = InteropString( "Failed to render provided canvas" );
            return ImporterResultCode::ImportFailed;
        }
        m_renderBuffer = canvas->GetData( );
    }
    else
    {
        ThorVGPicture thorPicture;
        if ( !thorPicture.Load( context.SourceFilePath.Get( ) ) )
        {
            context.ErrorMessage = InteropString( "Failed to import SVG file: " ).Append( context.SourceFilePath.Get( ) );
            return ImporterResultCode::ImportFailed;
        }
        if ( context.Desc.RenderWidth != 0 && context.Desc.RenderHeight != 0 )
        {
            if ( ! thorPicture.Size( context.Desc.RenderWidth, context.Desc.RenderHeight ) )
            {
                LOG( WARNING ) << "Failed to resize SVG to provided size";
            }
        }

        ThorVGCanvasDesc canvasDesc;
        canvasDesc.Width  = context.Desc.RenderWidth;
        canvasDesc.Height = context.Desc.RenderHeight;

        const ThorVGCanvas canvas{ canvasDesc };
        canvas.Push( &thorPicture );
        if (! canvas.Draw( ) )
        {
            context.ErrorMessage = InteropString( "Failed to render provided canvas" );
            return ImporterResultCode::ImportFailed;
        }
        m_renderBuffer = canvas.GetData( );
    }

    context.TextureAsset.Width     = context.Desc.RenderWidth;
    context.TextureAsset.Height    = context.Desc.RenderHeight;
    context.TextureAsset.Depth     = 1;
    context.TextureAsset.MipLevels = 1;
    context.TextureAsset.ArraySize = 1;
    context.TextureAsset.Format    = context.Desc.OutputFormat;
    context.TextureAsset.Dimension = TextureDimension::Texture2D;

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

    AssetUri assetUri;
    WriteTextureAsset( context, context.TextureAsset, assetUri );
    RegisterCreatedAsset( context, assetUri );
    return ImporterResultCode::Success;
}

void VGImporter::WriteTextureAsset( const ImportContext &context, const TextureAsset &textureAsset, AssetUri &outAssetUri ) const
{
    const InteropString assetName     = AssetPathUtilities::GetAssetNameFromFilePath( context.SourceFilePath );
    InteropString       sanitizedName = AssetPathUtilities::SanitizeAssetName( assetName );

    const std::filesystem::path targetDirectory = context.TargetDirectory.Get( );
    const std::filesystem::path fileName        = AssetPathUtilities::CreateAssetFileName( context.AssetNamePrefix, sanitizedName, "dztex" ).Get( );
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
