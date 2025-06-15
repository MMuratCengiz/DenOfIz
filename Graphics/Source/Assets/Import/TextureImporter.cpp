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
#include "DenOfIzGraphicsInternal/Utilities/DZArenaHelper.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

class TextureImporter::Impl
{
public:
    InteropString              m_name;
    std::vector<InteropString> m_supportedExtensions;
    std::vector<AssetUri>      m_createdAssets;
    std::unique_ptr<Texture>   m_texture = nullptr;

    struct ImportContext
    {
        TextureImportDesc Desc;
        ImporterResult    Result;
        InteropString     ErrorMessage;
        TextureAsset     *TextureAsset = nullptr;
    };

    struct TextureStats
    {
        uint32_t Width              = 0;
        uint32_t Height             = 0;
        uint32_t MipCount           = 0;
        uint32_t ArraySize          = 0;
        size_t   EstimatedArenaSize = 0;
    };

    explicit Impl( ) : m_name( "Texture Importer" )
    {
        m_supportedExtensions.resize( 9 );
        m_supportedExtensions[ 0 ] = ".png";
        m_supportedExtensions[ 1 ] = ".jpg";
        m_supportedExtensions[ 2 ] = ".jpeg";
        m_supportedExtensions[ 3 ] = ".bmp";
        m_supportedExtensions[ 4 ] = ".tga";
        m_supportedExtensions[ 5 ] = ".dds";
        m_supportedExtensions[ 6 ] = ".hdr";
        m_supportedExtensions[ 7 ] = ".gif";
        m_supportedExtensions[ 8 ] = ".psd";
    }

    ~Impl( ) = default;

    ImporterResult Import( const TextureImportDesc &desc );

private:
    ImporterResultCode ImportTextureInternal( ImportContext &context ) const;
    TextureStats       CalculateTextureStats( const TextureImportDesc &desc );
    void               WriteTextureAsset( const ImportContext &context, AssetUri &outAssetUri ) const;
};

TextureImporter::TextureImporter( ) : m_pImpl( std::make_unique<Impl>( ) )
{
}

TextureImporter::~TextureImporter( ) = default;

InteropString TextureImporter::GetName( ) const
{
    return m_pImpl->m_name;
}

InteropStringArray TextureImporter::GetSupportedExtensions( ) const
{
    return { m_pImpl->m_supportedExtensions.data( ), m_pImpl->m_supportedExtensions.size( ) };
}

bool TextureImporter::CanProcessFileExtension( const InteropString &extension ) const
{
    const InteropString lowerExt = extension.ToLower( );
    for ( const auto &m_supportedExtension : m_pImpl->m_supportedExtensions )
    {
        if ( m_supportedExtension.Equals( lowerExt ) )
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

ImporterResult TextureImporter::Import( const TextureImportDesc &desc ) const
{
    return m_pImpl->Import( desc );
}

TextureImporter::Impl::TextureStats TextureImporter::Impl::CalculateTextureStats( const TextureImportDesc &desc )
{
    TextureStats stats;

    m_texture       = std::make_unique<Texture>( desc.SourceFilePath );
    stats.Width     = m_texture->GetWidth( );
    stats.Height    = m_texture->GetHeight( );
    stats.MipCount  = m_texture->GetMipLevels( );
    stats.ArraySize = m_texture->GetArraySize( );

    stats.EstimatedArenaSize = sizeof( TextureMip ) * stats.MipCount * stats.ArraySize;
    stats.EstimatedArenaSize += sizeof( UserProperty ) * 10;
    stats.EstimatedArenaSize += 4096;

    return stats;
}

ImporterResult TextureImporter::Impl::Import( const TextureImportDesc &desc )
{
    spdlog::info( "Starting texture import for file: {}", desc.SourceFilePath.Get( ) );

    const TextureStats stats = CalculateTextureStats( desc );

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

    context.TextureAsset = new TextureAsset( );
    context.TextureAsset->_Arena.EnsureCapacity( stats.EstimatedArenaSize );

    if ( const ImporterResultCode result = ImportTextureInternal( context ); result != ImporterResultCode::Success )
    {
        context.Result.ResultCode = result;
        delete context.TextureAsset;
        return context.Result;
    }

    AssetUri assetUri;
    WriteTextureAsset( context, assetUri );
    m_createdAssets.push_back( assetUri );

    context.Result.CreatedAssets.NumElements = static_cast<uint32_t>( m_createdAssets.size( ) );
    context.Result.CreatedAssets.Elements    = m_createdAssets.data( );

    delete context.TextureAsset;
    spdlog::info( "Texture import successful for: {}", context.Desc.SourceFilePath.Get( ) );
    return context.Result;
}

ImporterResultCode TextureImporter::Impl::ImportTextureInternal( ImportContext &context ) const
{
    context.TextureAsset->Width     = m_texture->GetWidth( );
    context.TextureAsset->Height    = m_texture->GetHeight( );
    context.TextureAsset->Depth     = m_texture->GetDepth( );
    context.TextureAsset->MipLevels = m_texture->GetMipLevels( );
    context.TextureAsset->ArraySize = m_texture->GetArraySize( );
    context.TextureAsset->Format    = m_texture->GetFormat( );
    context.TextureAsset->Dimension = m_texture->GetDimension( );
    context.TextureAsset->Uri.Path  = context.Desc.SourceFilePath;

    const auto sourceMips = m_texture->ReadMipData( );
    DZArenaArrayHelper<TextureMipArray, TextureMip>::AllocateAndCopyArray( context.TextureAsset->_Arena, context.TextureAsset->Mips, sourceMips.Elements, sourceMips.NumElements );

    return ImporterResultCode::Success;
}

void TextureImporter::Impl::WriteTextureAsset( const ImportContext &context, AssetUri &outAssetUri ) const
{
    const InteropString         assetName       = AssetPathUtilities::GetAssetNameFromFilePath( context.Desc.SourceFilePath );
    const InteropString         sanitizedName   = AssetPathUtilities::SanitizeAssetName( assetName );
    const std::filesystem::path targetDirectory = context.Desc.TargetDirectory.Get( );
    const std::filesystem::path fileName        = AssetPathUtilities::CreateAssetFileName( context.Desc.AssetNamePrefix, sanitizedName, "dztex" ).Get( );
    const InteropString         filePath        = ( targetDirectory / fileName ).string( ).c_str( );

    BinaryWriter           writer( filePath );
    TextureAssetWriterDesc writerDesc{ };
    writerDesc.Writer = &writer;

    TextureAssetWriter textureWriter( writerDesc );
    textureWriter.Write( *context.TextureAsset );

    const auto mipDataArray = m_texture->ReadMipData( );
    for ( uint32_t i = 0; i < mipDataArray.NumElements; ++i )
    {
        const TextureMip &mipData   = mipDataArray.Elements[ i ];
        const size_t      mipSize   = mipData.SlicePitch;
        const size_t      mipOffset = mipData.DataOffset;

        ByteArrayView pixelDataArray{ };
        pixelDataArray.Elements    = m_texture->GetData( ).Elements + mipOffset;
        pixelDataArray.NumElements = mipSize;
        textureWriter.AddPixelData( pixelDataArray, mipData.MipIndex, mipData.ArrayIndex );
    }

    textureWriter.End( );
    writer.Flush( );

    outAssetUri.Path = filePath;
    spdlog::info( "Created texture asset: {}", outAssetUri.Path.Get( ) );
}
