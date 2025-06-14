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

#include "DenOfIzGraphicsInternal/Assets/Import/AssimpMaterialProcessor.h"
#include <filesystem>
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphics/Assets/Import/AssetPathUtilities.h"
#include "DenOfIzGraphics/Assets/Serde/Material/MaterialAssetWriter.h"
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAssetWriter.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"
#include "DenOfIzGraphics/Data/Texture.h"
#include "DenOfIzGraphicsInternal/Utilities/DZArenaHelper.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

AssimpMaterialProcessor::AssimpMaterialProcessor( )  = default;
AssimpMaterialProcessor::~AssimpMaterialProcessor( ) = default;

ImporterResultCode AssimpMaterialProcessor::ProcessAllMaterials( AssimpImportContext &context ) const
{
    if ( !context.Desc.ImportMaterials || !context.Scene->HasMaterials( ) )
    {
        return ImporterResultCode::Success;
    }

    spdlog::info( "Processing {} materials", context.Scene->mNumMaterials );

    context.MaterialNameToAssetUriMap.reserve( context.Scene->mNumMaterials );
    for ( unsigned int i = 0; i < context.Scene->mNumMaterials; ++i )
    {
        AssetUri materialUri;
        if ( const ImporterResultCode result = ProcessMaterial( context, context.Scene->mMaterials[ i ], materialUri ); result != ImporterResultCode::Success )
        {
            spdlog::error( "Failed to process material {}", i );
            return result;
        }
    }

    spdlog::info( "Processed {} materials successfully", context.MaterialNameToAssetUriMap.size( ) );
    return ImporterResultCode::Success;
}

ImporterResultCode AssimpMaterialProcessor::ProcessMaterial( AssimpImportContext &context, const aiMaterial *material, AssetUri &outMaterialUri ) const
{
    const std::string matNameStr = material->GetName( ).C_Str( );
    InteropString     matName    = AssetPathUtilities::SanitizeAssetName( matNameStr.c_str( ) );
    if ( matName.IsEmpty( ) )
    {
        matName = InteropString( "Material_" ).Append( std::to_string( context.MaterialNameToAssetUriMap.size( ) ).c_str( ) );
    }

    if ( context.MaterialNameToAssetUriMap.contains( matNameStr ) )
    {
        outMaterialUri = context.MaterialNameToAssetUriMap[ matNameStr ];
        return ImporterResultCode::Success;
    }

    spdlog::info( "Processing material: {}", matName.Get( ) );

    MaterialAsset matAsset;
    matAsset.Name = matName;
    ExtractMaterialProperties( material, matAsset );
    if ( context.Desc.ImportTextures )
    {
        ProcessMaterialTextures( context, material, matAsset );
    }
    return WriteMaterialAsset( context, matAsset, outMaterialUri );
}

ImporterResultCode AssimpMaterialProcessor::ProcessMaterialTextures( AssimpImportContext &context, const aiMaterial *material, MaterialAsset &materialAsset ) const
{
    ProcessTexture( context, material, aiTextureType_DIFFUSE, "Albedo", materialAsset.AlbedoMapRef );

    if ( !ProcessTexture( context, material, aiTextureType_NORMALS, "Normal", materialAsset.NormalMapRef ) )
    {
        ProcessTexture( context, material, aiTextureType_HEIGHT, "Normal", materialAsset.NormalMapRef );
    }

    ProcessTexture( context, material, aiTextureType_METALNESS, "MetallicRoughness", materialAsset.MetallicRoughnessMapRef );
    ProcessTexture( context, material, aiTextureType_EMISSIVE, "Emissive", materialAsset.EmissiveMapRef );
    ProcessTexture( context, material, aiTextureType_AMBIENT_OCCLUSION, "Occlusion", materialAsset.OcclusionMapRef );

    return ImporterResultCode::Success;
}

bool AssimpMaterialProcessor::ProcessTexture( AssimpImportContext &context, const aiMaterial *material, const aiTextureType textureType, const InteropString &semanticName,
                                              AssetUri &outAssetUri ) const
{
    aiString aiPath;
    if ( material->GetTexture( textureType, 0, &aiPath ) != AI_SUCCESS || aiPath.length == 0 )
    {
        return false;
    }

    const std::string texPathStr = aiPath.C_Str( );
    if ( texPathStr[ 0 ] == '*' )
    {
        spdlog::info( "Processing embedded texture for material '{}', semantic: {}", material->GetName( ).C_Str( ), semanticName.Get( ) );

        const int textureIndex = std::stoi( texPathStr.substr( 1 ) );
        if ( textureIndex >= 0 && textureIndex < static_cast<int>( context.Scene->mNumTextures ) )
        {
            WriteTextureAsset( context, context.Scene->mTextures[ textureIndex ], "", semanticName, outAssetUri );
            return true;
        }

        spdlog::error( "Invalid embedded texture index {} for material '{}'", texPathStr.substr( 1 ), material->GetName( ).C_Str( ) );
        return false;
    }

    spdlog::info( "Processing external texture reference: '{}' for semantic: {}", texPathStr, semanticName.Get( ) );
    if ( context.TexturePathToAssetUriMap.contains( texPathStr ) )
    {
        outAssetUri = context.TexturePathToAssetUriMap[ texPathStr ];
        return true;
    }

    const std::filesystem::path modelPath           = FileIO::GetResourcePath( context.SourceFilePath ).Get( );
    const std::filesystem::path texturePath         = texPathStr;
    const std::filesystem::path absoluteTexturePath = texturePath.is_absolute( ) ? texturePath : absolute( modelPath.parent_path( ) / texturePath );

    if ( !exists( absoluteTexturePath ) )
    {
        spdlog::error( "External texture file not found: {} (referenced by material '{}')", absoluteTexturePath.string( ), material->GetName( ).C_Str( ) );
        return false;
    }

    WriteTextureAsset( context, nullptr, absoluteTexturePath.string( ), semanticName, outAssetUri );
    return true;
}

ImporterResultCode AssimpMaterialProcessor::WriteTextureAsset( AssimpImportContext &context, const aiTexture *texture, const std::string &path, const InteropString &semanticName,
                                                               AssetUri &outAssetUri ) const
{
    InteropString texName;
    if ( texture != nullptr )
    {
        texName = AssetPathUtilities::SanitizeAssetName( texture->mFilename.length > 0 ? texture->mFilename.C_Str( ) : semanticName.Get( ) );
        if ( texName.IsEmpty( ) )
        {
            texName = InteropString( semanticName ).Append( "_Tex_" ).Append( std::to_string( context.CreatedAssets.size( ) ).c_str( ) );
        }
    }
    else
    {
        const std::filesystem::path texPath = path;
        texName                             = AssetPathUtilities::SanitizeAssetName( texPath.filename( ).stem( ).string( ).c_str( ) );
    }

    const InteropString assetFilename   = AssetPathUtilities::CreateAssetFileName( context.AssetNamePrefix, texName, "Texture", TextureAsset::Extension( ) );
    const InteropString targetAssetPath = FileIO::GetAbsolutePath( InteropString( context.TargetDirectory ).Append( "/" ).Append( assetFilename.Get( ) ) );

    BinaryWriter       writer( targetAssetPath );
    TextureAssetWriter assetWriter( { &writer } );

    outAssetUri = AssetUri::Create( assetFilename );

    spdlog::info( "Writing Texture asset to: {} (Semantic: {})", targetAssetPath.Get( ), semanticName.Get( ) );

    TextureAsset texAsset;
    texAsset._Arena.EnsureCapacity( 1024 * 1024 * 16 ); // TODO Pre-allocate 16MB for texture, might actually need more for 4k
    texAsset.Name = texName;
    texAsset.Uri  = outAssetUri;

    std::unique_ptr<Texture> sourceTexture = nullptr;
    if ( texture )
    {
        const bool    isCompressed = texture->mHeight == 0;
        size_t        numBytes     = isCompressed ? texture->mWidth : texture->mWidth * texture->mHeight * 4;
        ByteArrayView textureData{ };
        textureData.Elements    = reinterpret_cast<Byte *>( texture->pcData );
        textureData.NumElements = numBytes;
        sourceTexture           = std::make_unique<Texture>( textureData, IdentifyTextureFormat( texture, textureData ) );
    }
    else
    {
        sourceTexture = std::make_unique<Texture>( InteropString( path.c_str( ) ) );
    }

    texAsset.Width        = sourceTexture->GetWidth( );
    texAsset.Height       = sourceTexture->GetHeight( );
    texAsset.Depth        = sourceTexture->GetDepth( );
    texAsset.Format       = sourceTexture->GetFormat( );
    texAsset.Dimension    = sourceTexture->GetDimension( );
    texAsset.MipLevels    = sourceTexture->GetMipLevels( );
    texAsset.ArraySize    = sourceTexture->GetArraySize( );
    texAsset.BitsPerPixel = sourceTexture->GetBitsPerPixel( );
    texAsset.BlockSize    = sourceTexture->GetBlockSize( );
    texAsset.RowPitch     = sourceTexture->GetRowPitch( );
    texAsset.NumRows      = sourceTexture->GetNumRows( );
    texAsset.SlicePitch   = sourceTexture->GetSlicePitch( );
    DZArenaArrayHelper<TextureMipArray, TextureMip>::AllocateAndConstructArray( texAsset._Arena, texAsset.Mips, sourceTexture->GetMipLevels( ) * sourceTexture->GetArraySize( ) );

    const auto mipDataArray = sourceTexture->ReadMipData( );
    for ( uint32_t i = 0; i < mipDataArray.NumElements; ++i )
    {
        texAsset.Mips.Elements[ i ] = mipDataArray.Elements[ i ];
    }

    assetWriter.Write( texAsset );
    for ( uint32_t i = 0; i < mipDataArray.NumElements; ++i )
    {
        const TextureMip &mipData   = mipDataArray.Elements[ i ];
        const size_t      mipSize   = mipData.SlicePitch;
        const size_t      mipOffset = mipData.DataOffset;

        ByteArrayView mipDataBuffer{ };
        mipDataBuffer.Elements    = sourceTexture->GetData( ).Elements + mipOffset;
        mipDataBuffer.NumElements = mipSize;
        assetWriter.AddPixelData( mipDataBuffer, mipData.MipIndex, mipData.ArrayIndex );
    }

    assetWriter.End( );

    context.CreatedAssets.push_back( outAssetUri );
    if ( !path.empty( ) )
    {
        context.TexturePathToAssetUriMap[ path ] = outAssetUri;
    }

    return ImporterResultCode::Success;
}

ImporterResultCode AssimpMaterialProcessor::WriteMaterialAsset( AssimpImportContext &context, MaterialAsset &materialAsset, AssetUri &outAssetUri ) const
{
    const InteropString assetFilename   = AssetPathUtilities::CreateAssetFileName( context.AssetNamePrefix, materialAsset.Name, "Material", MaterialAsset::Extension( ) );
    const InteropString targetAssetPath = FileIO::GetAbsolutePath( InteropString( context.TargetDirectory ).Append( "/" ).Append( assetFilename.Get( ) ) );

    outAssetUri       = AssetUri::Create( assetFilename );
    materialAsset.Uri = outAssetUri;

    spdlog::info( "Writing Material asset to: {}", targetAssetPath.Get( ) );

    BinaryWriter              writer( targetAssetPath );
    const MaterialAssetWriter assetWriter( { &writer } );
    assetWriter.Write( materialAsset );

    context.CreatedAssets.push_back( outAssetUri );

    const std::string matNameStr                    = materialAsset.Name.Get( );
    context.MaterialNameToAssetUriMap[ matNameStr ] = outAssetUri;
    return ImporterResultCode::Success;
}

void AssimpMaterialProcessor::ExtractMaterialProperties( const aiMaterial *material, MaterialAsset &materialAsset ) const
{
    aiColor4D color;
    float     factor;
    int       intValue;

    if ( material->Get( AI_MATKEY_COLOR_DIFFUSE, color ) == AI_SUCCESS )
    {
        materialAsset.BaseColorFactor = ConvertColor( color );
    }
    if ( material->Get( AI_MATKEY_METALLIC_FACTOR, factor ) == AI_SUCCESS )
    {
        materialAsset.MetallicFactor = factor;
    }
    if ( material->Get( AI_MATKEY_ROUGHNESS_FACTOR, factor ) == AI_SUCCESS )
    {
        materialAsset.RoughnessFactor = factor;
    }
    if ( material->Get( AI_MATKEY_COLOR_EMISSIVE, color ) == AI_SUCCESS )
    {
        materialAsset.EmissiveFactor = { color.r, color.g, color.b };
    }
    if ( material->Get( AI_MATKEY_OPACITY, factor ) == AI_SUCCESS )
    {
        materialAsset.AlphaBlend = factor < 1.0f;
        if ( materialAsset.AlphaBlend )
        {
            materialAsset.BaseColorFactor.W = factor;
        }
    }
    if ( material->Get( AI_MATKEY_TWOSIDED, intValue ) == AI_SUCCESS )
    {
        materialAsset.DoubleSided = intValue != 0;
    }
}

Float_4 AssimpMaterialProcessor::ConvertColor( const aiColor4D &color ) const
{
    return { color.r, color.g, color.b, color.a };
}

Float_3 AssimpMaterialProcessor::ConvertColor3( const aiColor3D &color ) const
{
    return { color.r, color.g, color.b };
}

TextureExtension AssimpMaterialProcessor::IdentifyTextureFormat( const aiTexture *texture, const ByteArrayView &data ) const
{
    const std::string formatHint( texture->achFormatHint );
    auto              texExtension = TextureExtension::DDS;

    if ( !formatHint.empty( ) && formatHint != "\0\0\0\0" )
    {
        const std::unordered_map<std::string, TextureExtension> formatMap = { { "jpg", TextureExtension::JPG }, { "jpeg", TextureExtension::JPG }, { "png", TextureExtension::PNG },
                                                                              { "bmp", TextureExtension::BMP }, { "tga", TextureExtension::TGA },  { "hdr", TextureExtension::HDR },
                                                                              { "gif", TextureExtension::GIF }, { "dds", TextureExtension::DDS } };
        if ( const auto it = formatMap.find( formatHint ); it != formatMap.end( ) )
        {
            texExtension = it->second;
        }
    }
    else
    {
        texExtension = Texture::IdentifyTextureFormat( data );
    }

    return texExtension;
}
