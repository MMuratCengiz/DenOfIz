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

#include <DenOfIzGraphics/Assets/Import/TextureImporter.h>
#include <DenOfIzGraphics/Assets/Stream/BinaryWriter.h>
#include <DenOfIzGraphics/Assets/Import/AssetPathUtilities.h>
#include <algorithm>
#include <filesystem>

#include "DenOfIzGraphics/Assets/FileSystem/PathResolver.h"

using namespace DenOfIz;

TextureImporter::TextureImporter( const TextureImporterDesc desc ) : m_desc( desc )
{
    m_importerInfo.Name = "Texture Importer";
    m_importerInfo.SupportedExtensions.AddElement( ".png" );
    m_importerInfo.SupportedExtensions.AddElement( ".jpg" );
    m_importerInfo.SupportedExtensions.AddElement( ".jpeg" );
    m_importerInfo.SupportedExtensions.AddElement( ".bmp" );
    m_importerInfo.SupportedExtensions.AddElement( ".tga" );
    m_importerInfo.SupportedExtensions.AddElement( ".dds" );
    m_importerInfo.SupportedExtensions.AddElement( ".hdr" );
    m_importerInfo.SupportedExtensions.AddElement( ".gif" );
    m_importerInfo.SupportedExtensions.AddElement( ".psd" );
}

TextureImporter::~TextureImporter( ) = default;

ImporterDesc TextureImporter::GetImporterInfo( ) const
{
    return m_importerInfo;
}

bool TextureImporter::CanProcessFileExtension( const InteropString &extension ) const
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
    LOG( INFO ) << "Starting texture import for file: " << desc.SourceFilePath.Get( );

    ImportContext context;
    context.SourceFilePath  = desc.SourceFilePath;
    context.TargetDirectory = desc.TargetDirectory;
    context.AssetNamePrefix = desc.AssetNamePrefix;
    context.Desc            = *reinterpret_cast<TextureImportDesc *>( desc.Desc );

    if ( !FileIO::FileExists( context.SourceFilePath ) )
    {
        context.Result.ResultCode   = ImporterResultCode::FileNotFound;
        context.Result.ErrorMessage = InteropString( "Source file not found: " ).Append( context.SourceFilePath.Get( ) );
        LOG( ERROR ) << context.Result.ErrorMessage.Get( );
        return context.Result;
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

    context.Result.ResultCode = ImportTextureInternal( context );
    LOG( INFO ) << "Texture import successful for: " << context.SourceFilePath.Get( );
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

    m_texture->StreamMipData(
        [ & ]( const TextureMip &mipData )
        {
            const size_t mipSize   = mipData.SlicePitch;
            const size_t mipOffset = mipData.DataOffset;

            InteropArray<Byte> mipDataBuffer;
            mipDataBuffer.MemCpy( m_texture->GetData( ).Data( ) + mipOffset, mipSize );
            textureWriter.AddPixelData( mipDataBuffer, mipData.MipIndex, mipData.ArrayIndex );
        } );

    textureWriter.End( );
    writer.Flush( );

    outAssetUri.Path = filePath;
}

void TextureImporter::RegisterCreatedAsset( ImportContext &context, const AssetUri &assetUri )
{
    context.Result.CreatedAssets.AddElement( assetUri );
    LOG( INFO ) << "Created texture asset: " << assetUri.Path.Get( );
}
