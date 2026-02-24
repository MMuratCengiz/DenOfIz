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

#include "DenOfIzGraphics/Assets/Import/VGImporter.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAssetWriter.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"
#include "DenOfIzGraphicsInternal/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphicsInternal/Assets/Import/AssetPathUtilities.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

namespace
{
    std::string ToLowerCopy( std::string value )
    {
        std::transform( value.begin( ), value.end( ), value.begin( ), []( unsigned char c ) { return static_cast<char>( std::tolower( c ) ); } );
        return value;
    }
} // namespace

class VGImporter::Impl
{
public:
    std::string                             m_name;
    std::vector<std::string>                m_supportedExtensions{ };
    mutable std::vector<DenOfIz_StringView> m_supportedExtensionViews{ };
    std::vector<std::string>                m_createdAssetStorage;
    std::vector<DenOfIz_StringView>         m_createdAssetViews;

    struct ImportContext
    {
        std::string            SourceFilePath;
        std::string            TargetDirectory;
        std::string            AssetNamePrefix;
        VGImportDesc           Desc;
        DenOfIz_ImporterResult Result;
        std::string            ErrorMessage;
        DenOfIz_TextureAsset   TextureAsset = DENOFIZ_NULL_HANDLE;
        DenOfIz_ByteArray      RenderData;
        std::vector<Byte>      RenderDataStorage;
    };

    struct VGStats
    {
        uint32_t Width              = 0;
        uint32_t Height             = 0;
        uint32_t MipCount           = 1;
        size_t   PixelDataSize      = 0;
        size_t   EstimatedArenaSize = 0;
    };

    explicit Impl( ) :
        m_name( "Vector Graphics Importer (Simplified)" ), m_supportedExtensions{ ".svg" }, m_supportedExtensionViews( ), m_createdAssetStorage( ), m_createdAssetViews( )
    {
        m_supportedExtensionViews.reserve( m_supportedExtensions.size( ) );
        for ( const std::string &ext : m_supportedExtensions )
        {
            m_supportedExtensionViews.emplace_back( ext.c_str( ), static_cast<uint32_t>( ext.size( ) ) );
        }
    }

    ~Impl( ) = default;
    DenOfIz_ImporterResult Import( const VGImportDesc &desc );

private:
    DenOfIz_ImporterResultCode ImportVGInternal( ImportContext &context ) const;
    VGStats                    CalculateVGStats( const VGImportDesc &desc ) const;
    void                       WriteTextureAsset( const ImportContext &context, std::string &outAssetUri ) const;
};

VGImporter::VGImporter( ) : m_pImpl( std::make_unique<Impl>( ) )
{
}

VGImporter::~VGImporter( ) = default;

DenOfIz_StringView VGImporter::GetName( ) const
{
    return { m_pImpl->m_name.c_str( ), static_cast<uint32_t>( m_pImpl->m_name.size( ) ) };
}

DenOfIz_StringViewArray VGImporter::GetSupportedExtensions( ) const
{
    return { m_pImpl->m_supportedExtensionViews.data( ), m_pImpl->m_supportedExtensionViews.size( ) };
}

bool VGImporter::CanProcessFileExtension( const DenOfIz_StringView &extension ) const
{
    std::string ext( extension.Chars, extension.NumChars );
    ext = ToLowerCopy( ext );
    for ( const std::string &supportedExtension : m_pImpl->m_supportedExtensions )
    {
        if ( ext == supportedExtension )
        {
            return true;
        }
    }
    return false;
}

DenOfIz_ImporterResult VGImporter::Import( const VGImportDesc &desc ) const
{
    return m_pImpl->Import( desc );
}

bool VGImporter::ValidateFile( const DenOfIz_StringView &filePath ) const
{
    std::string filePathStr( filePath.Chars, filePath.NumChars );
    if ( !FileIO::FileExists( filePath ) )
    {
        return false;
    }

    const std::string extension = AssetPathUtilities::GetFileExtension( filePathStr );
    return CanProcessFileExtension( DENOFIZ_STRING( extension.c_str( ) ) );
}

VGImporter::Impl::VGStats VGImporter::Impl::CalculateVGStats( const VGImportDesc &desc ) const
{
    VGStats stats;
    stats.Width         = desc.RenderWidth;
    stats.Height        = desc.RenderHeight;
    stats.MipCount      = 1;
    stats.PixelDataSize = stats.Width * stats.Height * 4; // RGBA

    stats.EstimatedArenaSize = sizeof( DenOfIz_TextureMip ) * stats.MipCount;
    stats.EstimatedArenaSize += stats.PixelDataSize; // Space for pixel data
    stats.EstimatedArenaSize += 4096;

    return stats;
}

DenOfIz_ImporterResult VGImporter::Impl::Import( const VGImportDesc &desc )
{
    std::string sourceFilePathStr( desc.SourceFilePath.Chars, desc.SourceFilePath.NumChars );
    spdlog::info( "Starting vector graphics import for file: {}", sourceFilePathStr );

    m_createdAssetStorage.clear( );
    m_createdAssetViews.clear( );

    const VGStats stats = CalculateVGStats( desc );

    ImportContext context;
    context.SourceFilePath  = std::string( desc.SourceFilePath.Chars, desc.SourceFilePath.NumChars );
    context.TargetDirectory = std::string( desc.TargetDirectory.Chars, desc.TargetDirectory.NumChars );
    context.AssetNamePrefix = std::string( desc.AssetNamePrefix.Chars, desc.AssetNamePrefix.NumChars );
    context.Desc            = desc;
    context.ErrorMessage.clear( );
    context.Result.ErrorMessage = { nullptr, 0 };

    if ( context.Desc.Canvas && !context.SourceFilePath.empty( ) )
    {
        context.Result.ResultCode   = DENOFIZ_IMPORTER_RESULT_INVALID_PARAMETERS;
        context.ErrorMessage        = "Cannot specify both Canvas and file path - use one or the other";
        char *errorMsgCopy          = strdup( context.ErrorMessage.c_str( ) );
        context.Result.ErrorMessage = { errorMsgCopy, static_cast<uint32_t>( context.ErrorMessage.length( ) ) };
        spdlog::error( "{}", context.ErrorMessage );
        return context.Result;
    }

    if ( !context.Desc.Canvas )
    {
        if ( !FileIO::FileExists( DENOFIZ_STRING( context.SourceFilePath.c_str( ) ) ) )
        {
            context.Result.ResultCode   = DENOFIZ_IMPORTER_RESULT_FILE_NOT_FOUND;
            context.ErrorMessage        = "Source file not found: " + context.SourceFilePath;
            char *errorMsgCopy          = strdup( context.ErrorMessage.c_str( ) );
            context.Result.ErrorMessage = { errorMsgCopy, static_cast<uint32_t>( context.ErrorMessage.length( ) ) };
            spdlog::error( "{}", context.ErrorMessage );
            return context.Result;
        }
    }

    if ( !FileIO::FileExists( DENOFIZ_STRING( context.TargetDirectory.c_str( ) ) ) )
    {
        spdlog::info( "Target directory does not exist, attempting to create: {}", context.TargetDirectory.c_str( ) );
        if ( !FileIO::CreateDirectories( DENOFIZ_STRING( context.TargetDirectory.c_str( ) ) ) )
        {
            context.Result.ResultCode   = DENOFIZ_IMPORTER_RESULT_WRITE_FAILED;
            context.ErrorMessage        = "Failed to create target directory: " + context.TargetDirectory;
            char *errorMsgCopy          = strdup( context.ErrorMessage.c_str( ) );
            context.Result.ErrorMessage = { errorMsgCopy, static_cast<uint32_t>( context.ErrorMessage.length( ) ) };
            spdlog::error( "{}", context.ErrorMessage );
            return context.Result;
        }
    }

    context.TextureAsset = DenOfIz_TextureAsset_Create( );
    if ( const DenOfIz_ImporterResultCode result = ImportVGInternal( context ); result != DENOFIZ_IMPORTER_RESULT_SUCCESS )
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
    spdlog::info( "Vector graphics import successful for: {}", context.SourceFilePath.c_str( ) );
    return context.Result;
}

DenOfIz_ImporterResultCode VGImporter::Impl::ImportVGInternal( ImportContext &context ) const
{
    if ( context.Desc.Canvas )
    {
        const ThorVGCanvas *canvas = context.Desc.Canvas;
        canvas->Draw( );
        canvas->Sync( );
        const auto canvasData = canvas->GetData( );
        context.RenderDataStorage.resize( canvasData.NumElements * 4 );
        context.RenderData.NumElements = context.RenderDataStorage.size( );
        context.RenderData.Elements    = context.RenderDataStorage.data( );
        for ( size_t i = 0; i < canvasData.NumElements; ++i )
        {
            const uint32_t argb = canvasData.Elements[ i ];
            const uint8_t  a    = argb >> 24 & 0xFF;
            const uint8_t  r    = argb >> 16 & 0xFF;
            const uint8_t  g    = argb >> 8 & 0xFF;
            const uint8_t  b    = argb & 0xFF;

            context.RenderData.Elements[ i * 4 + 0 ] = r;
            context.RenderData.Elements[ i * 4 + 1 ] = g;
            context.RenderData.Elements[ i * 4 + 2 ] = b;
            context.RenderData.Elements[ i * 4 + 3 ] = a;
        }
    }
    else
    {
        ThorVGPicture thorPicture;
        thorPicture.Load( DENOFIZ_STRING( context.SourceFilePath.c_str( ) ) );
        if ( context.Desc.RenderWidth != 0 && context.Desc.RenderHeight != 0 )
        {
            thorPicture.SetSize( context.Desc.RenderWidth, context.Desc.RenderHeight );
        }

        ThorVGCanvasDesc canvasDesc{ };
        canvasDesc.Width  = context.Desc.RenderWidth;
        canvasDesc.Height = context.Desc.RenderHeight;

        const ThorVGCanvas canvas{ canvasDesc };
        canvas.Push( &thorPicture );
        canvas.Draw( );
        const auto canvasData = canvas.GetData( );
        context.RenderDataStorage.resize( canvasData.NumElements * 4 );
        context.RenderData.NumElements = context.RenderDataStorage.size( );
        context.RenderData.Elements    = context.RenderDataStorage.data( );
        for ( size_t i = 0; i < canvasData.NumElements; ++i )
        {
            const uint32_t argb = canvasData.Elements[ i ];
            const uint8_t  a    = argb >> 24 & 0xFF;
            const uint8_t  r    = argb >> 16 & 0xFF;
            const uint8_t  g    = argb >> 8 & 0xFF;
            const uint8_t  b    = argb & 0xFF;

            context.RenderData.Elements[ i * 4 + 0 ] = r;
            context.RenderData.Elements[ i * 4 + 1 ] = g;
            context.RenderData.Elements[ i * 4 + 2 ] = b;
            context.RenderData.Elements[ i * 4 + 3 ] = a;
        }
    }

    DenOfIz_TextureAsset_SetWidth( context.TextureAsset, context.Desc.RenderWidth );
    DenOfIz_TextureAsset_SetHeight( context.TextureAsset, context.Desc.RenderHeight );
    DenOfIz_TextureAsset_SetDepth( context.TextureAsset, 1 );
    DenOfIz_TextureAsset_SetMipLevels( context.TextureAsset, 1 );
    DenOfIz_TextureAsset_SetArraySize( context.TextureAsset, 1 );
    DenOfIz_TextureAsset_SetFormat( context.TextureAsset, context.Desc.OutputFormat );
    DenOfIz_TextureAsset_SetDimension( context.TextureAsset, DENOFIZ_TEXTURE_DIMENSION_TEXTURE2D );
    DenOfIz_TextureAsset_SetPath( context.TextureAsset, DenOfIz_StringView( context.SourceFilePath.c_str( ) ) );

    DenOfIz_TextureAsset_SetBitsPerPixel( context.TextureAsset, 32 );
    DenOfIz_TextureAsset_SetBlockSize( context.TextureAsset, 1 );
    DenOfIz_TextureAsset_SetRowPitch( context.TextureAsset, context.Desc.RenderWidth * 4 );
    DenOfIz_TextureAsset_SetNumRows( context.TextureAsset, context.Desc.RenderHeight );
    const uint32_t rowPitch = DenOfIz_TextureAsset_RowPitch( context.TextureAsset );
    DenOfIz_TextureAsset_SetSlicePitch( context.TextureAsset, rowPitch * context.Desc.RenderHeight );

    DenOfIz_TextureMip mip{ };
    mip.Width      = context.Desc.RenderWidth;
    mip.Height     = context.Desc.RenderHeight;
    mip.MipIndex   = 0;
    mip.ArrayIndex = 0;
    mip.RowPitch   = context.Desc.RenderWidth * 4;
    mip.NumRows    = context.Desc.RenderHeight;
    mip.SlicePitch = mip.RowPitch * context.Desc.RenderHeight;
    mip.DataOffset = 0;
    DenOfIz_TextureAsset_AddMip( context.TextureAsset, &mip );

    return DENOFIZ_IMPORTER_RESULT_SUCCESS;
}

void VGImporter::Impl::WriteTextureAsset( const ImportContext &context, std::string &outAssetUri ) const
{
    const DenOfIz_StringView ext = DenOfIz_TextureAsset_Extension( );
    const std::string        extStr( ext.Chars, ext.NumChars );
    const std::string        assetName     = AssetPathUtilities::GetAssetNameFromFilePath( DENOFIZ_STRING( context.SourceFilePath.c_str( ) ) );
    const std::string        sanitizedName = AssetPathUtilities::SanitizeAssetName( assetName );

    const std::filesystem::path targetDirectory = context.TargetDirectory;
    const std::filesystem::path fileName        = AssetPathUtilities::CreateAssetFileName( context.AssetNamePrefix, sanitizedName, extStr );
    auto                        relativePath    = context.TargetDirectory + "/" + fileName.string( );
    const std::string           filePath        = FileIO::GetAbsolutePath( DENOFIZ_STRING( relativePath.c_str( ) ) );

    {
        const DenOfIz_StringView       filePathView = DENOFIZ_STRING( filePath.c_str( ) );
        DenOfIz_BinaryWriter           writer       = DenOfIz_BinaryWriter_CreateFromFile( filePathView );
        DenOfIz_TextureAssetWriterDesc writerDesc{ };
        writerDesc.Writer = writer;

        DenOfIz_TextureAssetWriter textureWriter = DenOfIz_TextureAssetWriter_Create( &writerDesc );
        DenOfIz_TextureAssetWriter_Write( textureWriter, context.TextureAsset );

        if ( context.RenderData.Elements == nullptr || context.RenderData.NumElements == 0 )
        {
            spdlog::error( "VGImporter: No pixel data to write" );
        }

        DenOfIz_ByteArrayView dataView{ };
        dataView.Elements    = context.RenderData.Elements;
        dataView.NumElements = context.RenderData.NumElements;
        DenOfIz_TextureAssetWriter_AddPixelData( textureWriter, &dataView, 0, 0 );
        DenOfIz_TextureAssetWriter_End( textureWriter );
        DenOfIz_TextureAssetWriter_Destroy( textureWriter );
        DenOfIz_BinaryWriter_Flush( writer );
        DenOfIz_BinaryWriter_Destroy( writer );
    }

    outAssetUri = filePath;
}
