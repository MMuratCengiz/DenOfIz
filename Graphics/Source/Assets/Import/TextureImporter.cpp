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
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAssetWriter.h"
#include "DenOfIzGraphics/Data/Texture.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

TextureImporter::TextureImporter( ) : m_name( "Texture Importer" )
{
    m_supportedExtensions               = InteropStringArray::Create( 9 );
    m_supportedExtensions.Elements[ 0 ] = ".png";
    m_supportedExtensions.Elements[ 1 ] = ".jpg";
    m_supportedExtensions.Elements[ 2 ] = ".jpeg";
    m_supportedExtensions.Elements[ 3 ] = ".bmp";
    m_supportedExtensions.Elements[ 4 ] = ".tga";
    m_supportedExtensions.Elements[ 5 ] = ".dds";
    m_supportedExtensions.Elements[ 6 ] = ".hdr";
    m_supportedExtensions.Elements[ 7 ] = ".gif";
    m_supportedExtensions.Elements[ 8 ] = ".psd";
}

TextureImporter::~TextureImporter( )
{
    m_supportedExtensions.Dispose( );
}

InteropString TextureImporter::GetName( ) const
{
    return m_name;
}

InteropStringArray TextureImporter::GetSupportedExtensions( ) const
{
    const InteropStringArray copy = InteropStringArray::Create( m_supportedExtensions.NumElements );
    for ( size_t i = 0; i < m_supportedExtensions.NumElements; ++i )
    {
        copy.Elements[ i ] = m_supportedExtensions.Elements[ i ];
    }
    return copy;
}

bool TextureImporter::CanProcessFileExtension( const InteropString &extension ) const
{
    const InteropString lowerExt = extension.ToLower( );
    for ( size_t i = 0; i < m_supportedExtensions.NumElements; ++i )
    {
        if ( m_supportedExtensions.Elements[ i ].Equals( lowerExt ) )
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

ImporterResult TextureImporter::Import( const TextureImportDesc &desc )
{
    spdlog::info( "Starting texture import for file: {}", desc.SourceFilePath.Get( ) );

    ImportContext context;
    context.Desc = desc;

    if ( !FileIO::FileExists( context.Desc.SourceFilePath ) )
    {
        context.Result.ResultCode   = ImporterResultCode::FileNotFound;
        context.Result.ErrorMessage = InteropString( "Source file not found: " ).Append( context.Desc.SourceFilePath.Get( ) );
        spdlog::error( "{}", context.Result.ErrorMessage.Get( ) );
        return context.Result;
    }

    if ( !FileIO::FileExists( context.Desc.TargetDirectory ) )
    {
        spdlog::info( "Target directory does not exist, attempting to create: {}", context.Desc.TargetDirectory.Get( ) );
        if ( !FileIO::CreateDirectories( context.Desc.TargetDirectory ) )
        {
            context.Result.ResultCode   = ImporterResultCode::WriteFailed;
            context.Result.ErrorMessage = InteropString( "Failed to create target directory: " ).Append( context.Desc.TargetDirectory.Get( ) );
            spdlog::error( "{}", context.Result.ErrorMessage.Get( ) );
            return context.Result;
        }
    }

    context.Result.ResultCode = ImportTextureInternal( context );
    spdlog::info( "Texture import successful for: {}", context.Desc.SourceFilePath.Get( ) );

    // Copy the created assets to the result
    context.Result.CreatedAssets.NumElements = static_cast<uint32_t>( m_createdAssets.size( ) );
    context.Result.CreatedAssets.Elements    = m_createdAssets.data( );

    return context.Result;
}

ImporterResultCode TextureImporter::ImportTextureInternal( ImportContext &context )
{
    m_texture = std::make_unique<Texture>( context.Desc.SourceFilePath );
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
    const InteropString         assetName       = AssetPathUtilities::GetAssetNameFromFilePath( context.Desc.SourceFilePath );
    const InteropString         sanitizedName   = AssetPathUtilities::SanitizeAssetName( assetName );
    const std::filesystem::path targetDirectory = context.Desc.TargetDirectory.Get( );
    const std::filesystem::path fileName        = AssetPathUtilities::CreateAssetFileName( context.Desc.AssetNamePrefix, sanitizedName, "texture" ).Get( );
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
    m_createdAssets.push_back( assetUri );
    spdlog::info( "Created texture asset: {}", assetUri.Path.Get( ) );
}
