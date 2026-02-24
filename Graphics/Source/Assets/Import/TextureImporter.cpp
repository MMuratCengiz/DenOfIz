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
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAssetWriter.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"
#include "DenOfIzGraphics/Data/TextureData.h"
#include "DenOfIzGraphicsInternal/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphicsInternal/Assets/Import/AssetPathUtilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

#include <algorithm>
#include <cctype>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

using namespace DenOfIz;

#define TEXTURE_IMPORTER_IMPL( handle ) DENOFIZ_FROM_HANDLE( TextureImporter, handle )

namespace
{
    std::string ToLowerCopy( std::string value )
    {
        std::transform( value.begin( ), value.end( ), value.begin( ), []( unsigned char c ) { return static_cast<char>( std::tolower( c ) ); } );
        return value;
    }
} // namespace

namespace DenOfIz
{
    class TextureImporter
    {
    public:
        std::string                     m_name;
        std::vector<std::string>        m_supportedExtensions;
        std::vector<DenOfIz_StringView> m_supportedExtensionViews;
        std::vector<std::string>        m_createdAssetStorage;
        std::vector<DenOfIz_StringView> m_createdAssetViews;
        DenOfIz_TextureData             m_texture;
        std::string                     m_errorMessage;

        struct ImportContext
        {
            std::string               SourceFilePath;
            std::string               TargetDirectory;
            std::string               AssetNamePrefix;
            DenOfIz_TextureImportDesc Desc;
            DenOfIz_ImporterResult    Result;
            std::string               ErrorMessage;
            DenOfIz_TextureAsset      TextureAsset;
        };

        TextureImporter( )
        {
            m_name                = "Texture Importer";
            m_supportedExtensions = { ".png", ".jpg", ".jpeg", ".bmp", ".tga", ".dds", ".hdr", ".gif", ".psd" };
            m_texture             = DENOFIZ_NULL_HANDLE;

            m_supportedExtensionViews.reserve( m_supportedExtensions.size( ) );
            for ( const std::string &ext : m_supportedExtensions )
            {
                m_supportedExtensionViews.emplace_back( ext.c_str( ), static_cast<uint32_t>( ext.size( ) ) );
            }
        }

        ~TextureImporter( )
        {
            if ( DENOFIZ_HANDLE_IS_VALID( m_texture ) )
            {
                DenOfIz_TextureData_Destroy( m_texture );
            }
        }

        DenOfIz_StringView GetName( ) const
        {
            return { m_name.c_str( ), static_cast<uint32_t>( m_name.size( ) ) };
        }

        DenOfIz_StringViewArray GetSupportedExtensions( )
        {
            return { m_supportedExtensionViews.data( ), m_supportedExtensionViews.size( ) };
        }

        bool CanProcessFileExtension( const DenOfIz_StringView &extension ) const
        {
            std::string ext( extension.Chars, extension.NumChars );
            ext = ToLowerCopy( ext );
            for ( const std::string &supportedExtension : m_supportedExtensions )
            {
                if ( ext == supportedExtension )
                {
                    return true;
                }
            }
            return false;
        }

        bool ValidateFile( const DenOfIz_StringView &filePath ) const
        {
            const std::string pathStr( filePath.Chars, filePath.NumChars );
            if ( !FileIO::FileExists( filePath ) )
            {
                return false;
            }

            const std::string extension = AssetPathUtilities::GetFileExtension( pathStr );
            return CanProcessFileExtension( DENOFIZ_STRING( extension.c_str( ) ) );
        }

        DenOfIz_ImporterResultCode ImportTextureInternal( const ImportContext &context ) const
        {
            DenOfIz_TextureAsset_SetWidth( context.TextureAsset, DenOfIz_TextureData_GetWidth( m_texture ) );
            DenOfIz_TextureAsset_SetHeight( context.TextureAsset, DenOfIz_TextureData_GetHeight( m_texture ) );
            DenOfIz_TextureAsset_SetDepth( context.TextureAsset, DenOfIz_TextureData_GetDepth( m_texture ) );
            DenOfIz_TextureAsset_SetMipLevels( context.TextureAsset, DenOfIz_TextureData_GetMipLevels( m_texture ) );
            DenOfIz_TextureAsset_SetArraySize( context.TextureAsset, DenOfIz_TextureData_GetArraySize( m_texture ) );
            DenOfIz_TextureAsset_SetFormat( context.TextureAsset, DenOfIz_TextureData_GetFormat( m_texture ) );
            DenOfIz_TextureAsset_SetDimension( context.TextureAsset, DenOfIz_TextureData_GetDimension( m_texture ) );
            DenOfIz_TextureAsset_SetPath( context.TextureAsset, DenOfIz_StringView( context.SourceFilePath.c_str( ) ) );

            const DenOfIz_TextureMipArray sourceMips = DenOfIz_TextureData_ReadMipData( m_texture );
            DenOfIz_TextureAsset_SetMips( context.TextureAsset, sourceMips.Elements, sourceMips.NumElements );

            return DENOFIZ_IMPORTER_RESULT_SUCCESS;
        }

        void WriteTextureAsset( const ImportContext &context, std::string &outAssetUri ) const
        {
            const auto                  assetName       = AssetPathUtilities::GetAssetNameFromFilePath( DENOFIZ_STRING( context.SourceFilePath.c_str( ) ) );
            const auto                  sanitizedName   = AssetPathUtilities::SanitizeAssetName( assetName );
            const std::filesystem::path targetDirectory = context.TargetDirectory.c_str( );
            const std::filesystem::path fileName        = context.AssetNamePrefix.empty( )
                ? AssetPathUtilities::CreateAssetFileName( "", sanitizedName, "dztex" ).c_str( )
                : AssetPathUtilities::CreateAssetFileName( "", context.AssetNamePrefix, "dztex" ).c_str( );
            const auto                  relativePath    = context.TargetDirectory + "/" + fileName.string( ).c_str( );
            const auto                  filePath        = FileIO::GetAbsolutePath( DENOFIZ_STRING( relativePath.c_str( ) ) );

            DenOfIz_BinaryWriter           writer = DenOfIz_BinaryWriter_CreateFromFile( DENOFIZ_STRING( filePath.c_str( ) ) );
            DenOfIz_TextureAssetWriterDesc writerDesc{ };
            writerDesc.Writer = writer;

            DenOfIz_TextureAssetWriter textureWriter = DenOfIz_TextureAssetWriter_Create( &writerDesc );
            DenOfIz_TextureAssetWriter_Write( textureWriter, context.TextureAsset );

            const DenOfIz_TextureMipArray mipDataArray = DenOfIz_TextureData_ReadMipData( m_texture );
            for ( uint32_t i = 0; i < mipDataArray.NumElements; ++i )
            {
                const DenOfIz_TextureMip &mipData   = mipDataArray.Elements[ i ];
                const size_t              mipSize   = mipData.SlicePitch;
                const size_t              mipOffset = mipData.DataOffset;

                DenOfIz_ByteArrayView pixelDataArray{ };
                pixelDataArray.Elements    = DenOfIz_TextureData_GetData( m_texture ).Elements + mipOffset;
                pixelDataArray.NumElements = mipSize;
                DenOfIz_TextureAssetWriter_AddPixelData( textureWriter, &pixelDataArray, mipData.MipIndex, mipData.ArrayIndex );
            }

            DenOfIz_TextureAssetWriter_End( textureWriter );
            DenOfIz_TextureAssetWriter_Destroy( textureWriter );
            DenOfIz_BinaryWriter_Flush( writer );
            DenOfIz_BinaryWriter_Destroy( writer );

            outAssetUri = filePath;
            spdlog::info( "Created texture asset: {}", outAssetUri );
        }

        DenOfIz_ImporterResult Import( const DenOfIz_TextureImportDesc &desc )
        {
            m_createdAssetStorage.clear( );
            m_createdAssetViews.clear( );

            ImportContext context{ };
            context.SourceFilePath  = std::string( desc.SourceFilePath.Chars, desc.SourceFilePath.NumChars );
            context.TargetDirectory = std::string( desc.TargetDirectory.Chars, desc.TargetDirectory.NumChars );
            context.AssetNamePrefix = std::string( desc.AssetNamePrefix.Chars, desc.AssetNamePrefix.NumChars );
            context.Desc            = desc;
            context.ErrorMessage.clear( );
            context.Result.ErrorMessage = { nullptr, 0 };
            spdlog::info( "Starting texture import for file: {}", context.SourceFilePath );

            if ( !FileIO::FileExists( DENOFIZ_STRING( context.SourceFilePath.c_str( ) ) ) )
            {
                context.Result.ResultCode   = DENOFIZ_IMPORTER_RESULT_FILE_NOT_FOUND;
                m_errorMessage              = "Source file not found: " + context.SourceFilePath;
                char *errorMsgCopy          = strdup( m_errorMessage.c_str( ) );
                context.Result.ErrorMessage = { errorMsgCopy, static_cast<uint32_t>( m_errorMessage.length( ) ) };
                spdlog::error( "{}", m_errorMessage );
                return context.Result;
            }

            if ( !FileIO::FileExists( DENOFIZ_STRING( context.TargetDirectory.c_str( ) ) ) )
            {
                spdlog::info( "Target directory does not exist, attempting to create: {}", context.TargetDirectory.c_str( ) );
                if ( !FileIO::CreateDirectories( DENOFIZ_STRING( context.TargetDirectory.c_str( ) ) ) )
                {
                    context.Result.ResultCode   = DENOFIZ_IMPORTER_RESULT_WRITE_FAILED;
                    m_errorMessage              = "Failed to create target directory: " + context.TargetDirectory;
                    char *errorMsgCopy          = strdup( m_errorMessage.c_str( ) );
                    context.Result.ErrorMessage = { errorMsgCopy, static_cast<uint32_t>( m_errorMessage.length( ) ) };
                    spdlog::error( "{}", m_errorMessage );
                    return context.Result;
                }
            }

            context.TextureAsset = DenOfIz_TextureAsset_Create( );
            if ( DENOFIZ_HANDLE_IS_VALID( m_texture ) )
            {
                DenOfIz_TextureData_Destroy( m_texture );
            }
            DenOfIz_TextureCreateFromPathDesc textureDesc{ };
            textureDesc.Path = DENOFIZ_STRING( context.SourceFilePath.c_str( ) );
            m_texture        = DenOfIz_TextureData_CreateFromPath( &textureDesc );
            if ( const DenOfIz_ImporterResultCode result = ImportTextureInternal( context ); result != DENOFIZ_IMPORTER_RESULT_SUCCESS )
            {
                context.Result.ResultCode = result;
                DenOfIz_TextureAsset_Destroy( context.TextureAsset );
                return context.Result;
            }

            std::string assetUri;
            WriteTextureAsset( context, assetUri );
            m_createdAssetStorage.push_back( assetUri );

            if ( !m_createdAssetStorage.empty( ) )
            {
                auto *createdAssetsArray = static_cast<DenOfIz_StringView *>( malloc( m_createdAssetStorage.size( ) * sizeof( DenOfIz_StringView ) ) );
                for ( size_t i = 0; i < m_createdAssetStorage.size( ); ++i )
                {
                    char *assetPathCopy              = strdup( m_createdAssetStorage[ i ].c_str( ) );
                    createdAssetsArray[ i ].Chars    = assetPathCopy;
                    createdAssetsArray[ i ].NumChars = static_cast<uint32_t>( m_createdAssetStorage[ i ].length( ) );
                }
                context.Result.CreatedAssets.NumElements = static_cast<uint32_t>( m_createdAssetStorage.size( ) );
                context.Result.CreatedAssets.Elements    = createdAssetsArray;
            }
            else
            {
                context.Result.CreatedAssets.NumElements = 0;
                context.Result.CreatedAssets.Elements    = nullptr;
            }

            DenOfIz_TextureAsset_Destroy( context.TextureAsset );
            spdlog::info( "Texture import successful for: {}", context.SourceFilePath.c_str( ) );
            return context.Result;
        }
    };
} // namespace DenOfIz

extern "C"
{

    DenOfIz_TextureImporter DenOfIz_TextureImporter_Create( )
    {
        auto *impl = new TextureImporter( );
        return DENOFIZ_TO_HANDLE( impl );
    }

    void DenOfIz_TextureImporter_Destroy( DenOfIz_TextureImporter importer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( importer ) )
        {
            return;
        }
        delete TEXTURE_IMPORTER_IMPL( importer );
    }

    DenOfIz_StringView DenOfIz_TextureImporter_GetName( DenOfIz_TextureImporter importer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( importer ) )
        {
            return { NULL, 0 };
        }
        return TEXTURE_IMPORTER_IMPL( importer )->GetName( );
    }

    DenOfIz_StringViewArray DenOfIz_TextureImporter_GetSupportedExtensions( DenOfIz_TextureImporter importer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( importer ) )
        {
            return { NULL, 0 };
        }
        return TEXTURE_IMPORTER_IMPL( importer )->GetSupportedExtensions( );
    }

    bool DenOfIz_TextureImporter_CanProcessFileExtension( DenOfIz_TextureImporter importer, DenOfIz_StringView extension )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( importer ) )
        {
            return false;
        }
        return TEXTURE_IMPORTER_IMPL( importer )->CanProcessFileExtension( extension );
    }

    bool DenOfIz_TextureImporter_ValidateFile( DenOfIz_TextureImporter importer, DenOfIz_StringView filePath )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( importer ) )
        {
            return false;
        }
        return TEXTURE_IMPORTER_IMPL( importer )->ValidateFile( filePath );
    }

    DenOfIz_ImporterResult DenOfIz_TextureImporter_Import( DenOfIz_TextureImporter importer, const DenOfIz_TextureImportDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( importer ) || desc == NULL )
        {
            DenOfIz_ImporterResult result{ };
            result.ResultCode = DENOFIZ_IMPORTER_RESULT_INVALID_PARAMETERS;
            return result;
        }
        return TEXTURE_IMPORTER_IMPL( importer )->Import( *desc );
    }
}
