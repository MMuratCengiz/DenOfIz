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

#include "DenOfIzGraphics/Assets/Import/TextureImporter.h"
#include <filesystem>
#include "DenOfIzGraphics/Assets/Import/AssetPathUtilities.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"

#include "DenOfIzGraphics/Assets/Bundle/BundleManager.h"
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphics/Assets/Import/IAssetImporter.h"
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAssetWriter.h"
#include "DenOfIzGraphics/Data/Texture.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

TextureImporter::TextureImporter( const TextureImporterDesc desc ) : m_desc( desc )
{
    m_importerInfo.Name                              = "Texture Importer";
    m_importerInfo.SupportedExtensions               = InteropStringArray::Create( 9 );
    m_importerInfo.SupportedExtensions.Elements[ 0 ] = ".png";
    m_importerInfo.SupportedExtensions.Elements[ 1 ] = ".jpg";
    m_importerInfo.SupportedExtensions.Elements[ 2 ] = ".jpeg";
    m_importerInfo.SupportedExtensions.Elements[ 3 ] = ".bmp";
    m_importerInfo.SupportedExtensions.Elements[ 4 ] = ".tga";
    m_importerInfo.SupportedExtensions.Elements[ 5 ] = ".dds";
    m_importerInfo.SupportedExtensions.Elements[ 6 ] = ".hdr";
    m_importerInfo.SupportedExtensions.Elements[ 7 ] = ".gif";
    m_importerInfo.SupportedExtensions.Elements[ 8 ] = ".psd";
}

TextureImporter::~TextureImporter( )
{
    m_importerInfo.SupportedExtensions.Dispose( );
}

ImporterDesc TextureImporter::GetImporterInfo( ) const
{
    return m_importerInfo;
}

bool TextureImporter::CanProcessFileExtension( const InteropString &extension ) const
{
    const InteropString lowerExt = extension.ToLower( );
    for ( size_t i = 0; i < m_importerInfo.SupportedExtensions.NumElements; ++i )
    {
        if ( m_importerInfo.SupportedExtensions.Elements[ i ].Equals( lowerExt ) )
        {
            return true;
        }
    }
    return false;
}

bool TextureImporter::ValidateFile( const InteropString &filePath ) const
{
    if ( !FileIO::FileExists( filePath ) )
    {
        return false;
    }

    const InteropString extension = AssetPathUtilities::GetFileExtension( filePath );
    return CanProcessFileExtension( extension );
}

ImporterResult TextureImporter::Import( const ImportJobDesc &desc )
{
    spdlog::info( "Starting texture import for file: {}", desc.SourceFilePath.Get( ) );

    ImportContext context;
    context.SourceFilePath  = desc.SourceFilePath;
    context.TargetDirectory = desc.TargetDirectory;
    context.AssetNamePrefix = desc.AssetNamePrefix;
    context.Desc            = *reinterpret_cast<TextureImportDesc *>( desc.Desc );

    if ( !FileIO::FileExists( context.SourceFilePath ) )
    {
        context.Result.ResultCode   = ImporterResultCode::FileNotFound;
        context.Result.ErrorMessage = InteropString( "Source file not found: " ).Append( context.SourceFilePath.Get( ) );
        spdlog::error( "{}", context.Result.ErrorMessage.Get( ) );
        return context.Result;
    }

    if ( !FileIO::FileExists( context.TargetDirectory ) )
    {
        spdlog::info( "Target directory does not exist, attempting to create: {}", context.TargetDirectory.Get( ) );
        if ( !FileIO::CreateDirectories( context.TargetDirectory ) )
        {
            context.Result.ResultCode   = ImporterResultCode::WriteFailed;
            context.Result.ErrorMessage = InteropString( "Failed to create target directory: " ).Append( context.TargetDirectory.Get( ) );
            spdlog::error( "{}", context.Result.ErrorMessage.Get( ) );
            return context.Result;
        }
    }

    context.Result.ResultCode = ImportTextureInternal( context );
    spdlog::info( "Texture import successful for: {}", context.SourceFilePath.Get( ) );
    return context.Result;
}

ImporterResultCode TextureImporter::ImportTextureInternal( ImportContext &context )
{
    m_texture = std::make_unique<Texture>( context.SourceFilePath );
    TextureAsset textureAsset;
    textureAsset.Width     = m_texture->GetWidth( );
    textureAsset.Height    = m_texture->GetHeight( );
    textureAsset.Depth     = m_texture->GetDepth( );
    textureAsset.MipLevels = m_texture->GetMipLevels( );
    textureAsset.ArraySize = m_texture->GetArraySize( );
    textureAsset.Format    = m_texture->GetFormat( );
    textureAsset.Dimension = m_texture->GetDimension( );
    textureAsset.Mips      = m_texture->ReadMipData( );

    AssetUri assetUri;
    WriteTextureAsset( context, textureAsset, assetUri );
    RegisterCreatedAsset( context, assetUri );
    return ImporterResultCode::Success;
}

void TextureImporter::WriteTextureAsset( const ImportContext &context, const TextureAsset &textureAsset, AssetUri &outAssetUri ) const
{
    const InteropString         assetName       = AssetPathUtilities::GetAssetNameFromFilePath( context.SourceFilePath );
    const InteropString         sanitizedName   = AssetPathUtilities::SanitizeAssetName( assetName );
    const std::filesystem::path targetDirectory = context.TargetDirectory.Get( );
    const std::filesystem::path fileName        = AssetPathUtilities::CreateAssetFileName( context.AssetNamePrefix, sanitizedName, "texture" ).Get( );
    const InteropString         filePath        = ( targetDirectory / fileName ).string( ).c_str( );

    BinaryWriter           writer( filePath );
    TextureAssetWriterDesc writerDesc{ };
    writerDesc.Writer = &writer;

    TextureAssetWriter textureWriter( writerDesc );
    textureWriter.Write( textureAsset );

    const auto mipDataArray = m_texture->ReadMipData( );
    for ( uint32_t i = 0; i < mipDataArray.NumElements; ++i )
    {
        const TextureMip &mipData   = mipDataArray.Elements[ i ];
        const size_t      mipSize   = mipData.SlicePitch;
        const size_t      mipOffset = mipData.DataOffset;

        ByteArrayView pixelDataArray{ };
        pixelDataArray.Elements    = m_texture->GetData( ).Data( ) + mipOffset;
        pixelDataArray.NumElements = mipSize;
        textureWriter.AddPixelData( pixelDataArray, mipData.MipIndex, mipData.ArrayIndex );
    }

    textureWriter.End( );
    writer.Flush( );

    outAssetUri.Path = filePath;
}

void TextureImporter::RegisterCreatedAsset( ImportContext &context, const AssetUri &assetUri )
{
    // TODO: Cleaner growing mechanism
    const AssetUriArray newCreatedAssets = AssetUriArray::Create( context.Result.CreatedAssets.NumElements + 1 );
    for ( size_t i = 0; i < context.Result.CreatedAssets.NumElements; ++i )
    {
        newCreatedAssets.Elements[ i ] = context.Result.CreatedAssets.Elements[ i ];
    }
    newCreatedAssets.Elements[ context.Result.CreatedAssets.NumElements ] = assetUri;
    context.Result.CreatedAssets.Dispose( );
    context.Result.CreatedAssets = newCreatedAssets;

    spdlog::info( "Created texture asset: {}", assetUri.Path.Get( ) );
}
