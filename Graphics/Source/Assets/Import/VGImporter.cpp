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
#include <filesystem>
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphics/Assets/FileSystem/PathResolver.h"
#include "DenOfIzGraphics/Assets/Import/AssetPathUtilities.h"
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAssetWriter.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"
#include "DenOfIzGraphicsInternal/Utilities/DZArenaHelper.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

class VGImporter::Impl
{
public:
    InteropString              m_name;
    std::vector<InteropString> m_supportedExtensions{ };
    std::vector<AssetUri>      m_createdAssets;

    struct ImportContext
    {
        VGImportDesc   Desc;
        ImporterResult Result;
        InteropString  ErrorMessage;
        TextureAsset  *TextureAsset = nullptr;
        ByteArray      RenderData;
    };

    struct VGStats
    {
        uint32_t Width              = 0;
        uint32_t Height             = 0;
        uint32_t MipCount           = 1;
        size_t   PixelDataSize      = 0;
        size_t   EstimatedArenaSize = 0;
    };

    explicit Impl( ) : m_name( "Vector Graphics Importer (Simplified)" )
    {
        m_supportedExtensions.resize( 1 );
        m_supportedExtensions[ 0 ] = ".svg";
    }

    ~Impl( ) = default;
    ImporterResult Import( const VGImportDesc &desc );

private:
    ImporterResultCode ImportVGInternal( ImportContext &context ) const;
    VGStats            CalculateVGStats( const VGImportDesc &desc ) const;
    void               WriteTextureAsset( const ImportContext &context, AssetUri &outAssetUri ) const;
};

VGImporter::VGImporter( ) : m_pImpl( std::make_unique<Impl>( ) )
{
}

VGImporter::~VGImporter( ) = default;

InteropString VGImporter::GetName( ) const
{
    return m_pImpl->m_name;
}

InteropStringArray VGImporter::GetSupportedExtensions( ) const
{
    return { m_pImpl->m_supportedExtensions.data( ), m_pImpl->m_supportedExtensions.size( ) };
}

bool VGImporter::CanProcessFileExtension( const InteropString &extension ) const
{
    const InteropString lowerExt = extension.ToLower( );
    for ( size_t i = 0; i < m_pImpl->m_supportedExtensions.size( ); ++i )
    {
        if ( m_pImpl->m_supportedExtensions[ i ].Equals( lowerExt ) )
        {
            return true;
        }
    }
    return false;
}

ImporterResult VGImporter::Import( const VGImportDesc &desc ) const
{
    return m_pImpl->Import( desc );
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

VGImporter::Impl::VGStats VGImporter::Impl::CalculateVGStats( const VGImportDesc &desc ) const
{
    VGStats stats;
    stats.Width         = desc.RenderWidth;
    stats.Height        = desc.RenderHeight;
    stats.MipCount      = 1;
    stats.PixelDataSize = stats.Width * stats.Height * 4; // RGBA

    stats.EstimatedArenaSize = sizeof( TextureMip ) * stats.MipCount;
    stats.EstimatedArenaSize += stats.PixelDataSize; // Space for pixel data
    stats.EstimatedArenaSize += sizeof( UserProperty ) * 10;
    stats.EstimatedArenaSize += 4096;

    return stats;
}

ImporterResult VGImporter::Impl::Import( const VGImportDesc &desc )
{
    spdlog::info( "Starting vector graphics import for file: {}", desc.SourceFilePath.Get( ) );

    const VGStats stats = CalculateVGStats( desc );

    ImportContext context;
    context.Desc = desc;

    if ( context.Desc.Canvas && !context.Desc.SourceFilePath.IsEmpty( ) )
    {
        context.Result.ResultCode   = ImporterResultCode::InvalidParameters;
        context.Result.ErrorMessage = InteropString( "Cannot specify both Canvas and file path - use one or the other" );
        spdlog::error( "{}", context.Result.ErrorMessage.Get( ) );
        return context.Result;
    }

    if ( !context.Desc.Canvas )
    {
        if ( !FileIO::FileExists( context.Desc.SourceFilePath ) )
        {
            context.Result.ResultCode   = ImporterResultCode::FileNotFound;
            context.Result.ErrorMessage = InteropString( "Source file not found: " ).Append( context.Desc.SourceFilePath.Get( ) );
            spdlog::error( "{}", context.Result.ErrorMessage.Get( ) );
            return context.Result;
        }
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

    if ( const ImporterResultCode result = ImportVGInternal( context ); result != ImporterResultCode::Success )
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
    spdlog::info( "Vector graphics import successful for: {}", context.Desc.SourceFilePath.Get( ) );
    return context.Result;
}

ImporterResultCode VGImporter::Impl::ImportVGInternal( ImportContext &context ) const
{
    if ( context.Desc.Canvas )
    {
        const ThorVGCanvas *canvas = context.Desc.Canvas;
        canvas->Draw( );
        canvas->Sync( );
        const auto canvasData = canvas->GetData( );
        DZArenaArrayHelper<ByteArray, Byte>::AllocateArray( context.TextureAsset->_Arena, context.RenderData, canvasData.NumElements * 4 );
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
        thorPicture.Load( context.Desc.SourceFilePath.Get( ) );
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

        DZArenaArrayHelper<ByteArray, Byte>::AllocateArray( context.TextureAsset->_Arena, context.RenderData, canvasData.NumElements * 4 );
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

    context.TextureAsset->Width     = context.Desc.RenderWidth;
    context.TextureAsset->Height    = context.Desc.RenderHeight;
    context.TextureAsset->Depth     = 1;
    context.TextureAsset->MipLevels = 1;
    context.TextureAsset->ArraySize = 1;
    context.TextureAsset->Format    = context.Desc.OutputFormat;
    context.TextureAsset->Dimension = TextureDimension::Texture2D;
    context.TextureAsset->Uri.Path  = context.Desc.SourceFilePath;

    context.TextureAsset->BitsPerPixel = 32; // RGBA8
    context.TextureAsset->BlockSize    = 1;
    context.TextureAsset->RowPitch     = context.Desc.RenderWidth * 4;
    context.TextureAsset->NumRows      = context.Desc.RenderHeight;
    context.TextureAsset->SlicePitch   = context.TextureAsset->RowPitch * context.Desc.RenderHeight;

    DZArenaArrayHelper<TextureMipArray, TextureMip>::AllocateAndConstructArray( context.TextureAsset->_Arena, context.TextureAsset->Mips, 1 );

    TextureMip &mip = context.TextureAsset->Mips.Elements[ 0 ];
    mip.Width       = context.Desc.RenderWidth;
    mip.Height      = context.Desc.RenderHeight;
    mip.MipIndex    = 0;
    mip.ArrayIndex  = 0;
    mip.RowPitch    = context.Desc.RenderWidth * 4;
    mip.NumRows     = context.Desc.RenderHeight;
    mip.SlicePitch  = mip.RowPitch * context.Desc.RenderHeight;
    mip.DataOffset  = 0;

    return ImporterResultCode::Success;
}

void VGImporter::Impl::WriteTextureAsset( const ImportContext &context, AssetUri &outAssetUri ) const
{
    const InteropString assetName     = AssetPathUtilities::GetAssetNameFromFilePath( context.Desc.SourceFilePath );
    const InteropString sanitizedName = AssetPathUtilities::SanitizeAssetName( assetName );

    const std::filesystem::path targetDirectory = context.Desc.TargetDirectory.Get( );
    const std::filesystem::path fileName        = AssetPathUtilities::CreateAssetFileName( context.Desc.AssetNamePrefix, sanitizedName, TextureAsset::Extension( ) ).Get( );
    const InteropString         filePath        = ( targetDirectory / fileName ).string( ).c_str( );

    {
        BinaryWriter           writer( filePath );
        TextureAssetWriterDesc writerDesc{ };
        writerDesc.Writer = &writer;

        TextureAssetWriter textureWriter( writerDesc );
        textureWriter.Write( *context.TextureAsset );

        if ( context.RenderData.Elements == nullptr || context.RenderData.NumElements == 0 )
        {
            spdlog::error( "VGImporter: No pixel data to write" );
        }

        ByteArrayView dataView{ };
        dataView.Elements    = context.RenderData.Elements;
        dataView.NumElements = context.RenderData.NumElements;
        textureWriter.AddPixelData( dataView, 0, 0 );
        textureWriter.End( );
        writer.Flush( );
    }

    outAssetUri.Path = filePath;
    spdlog::info( "VGImporter: Wrote texture asset to {} with {} bytes of pixel data", filePath.Get( ), context.RenderData.NumElements );
}
