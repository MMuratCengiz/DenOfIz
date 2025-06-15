/*
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

#include "DenOfIzGraphics/Assets/Import/AssimpImporter.h"
#include <memory>
#include "DenOfIzGraphics/Assets/Bundle/BundleManager.h"
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphics/Assets/Import/AssetPathUtilities.h"
#include "DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetWriter.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"
#include "DenOfIzGraphicsInternal/Assets/Import/AssimpAnimationProcessor.h"
#include "DenOfIzGraphicsInternal/Assets/Import/AssimpImportContext.h"
#include "DenOfIzGraphicsInternal/Assets/Import/AssimpMaterialProcessor.h"
#include "DenOfIzGraphicsInternal/Assets/Import/AssimpMeshProcessor.h"
#include "DenOfIzGraphicsInternal/Assets/Import/AssimpSceneLoader.h"
#include "DenOfIzGraphicsInternal/Assets/Import/AssimpSkeletonProcessor.h"
#include "DenOfIzGraphicsInternal/Utilities/DZArenaHelper.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

class AssimpImporter::Impl
{
public:
    std::unique_ptr<DZArena> m_mainArena;
    std::unique_ptr<DZArena> m_tempArena;

    std::unique_ptr<AssimpSceneLoader>        m_sceneLoader;
    std::unique_ptr<AssimpMeshProcessor>      m_meshProcessor;
    std::unique_ptr<AssimpMaterialProcessor>  m_materialProcessor;
    std::unique_ptr<AssimpSkeletonProcessor>  m_skeletonProcessor;
    std::unique_ptr<AssimpAnimationProcessor> m_animationProcessor;
    std::vector<InteropString>                m_supportedExtensions;

    explicit Impl( )
    {
        m_sceneLoader        = std::make_unique<AssimpSceneLoader>( );
        m_meshProcessor      = std::make_unique<AssimpMeshProcessor>( );
        m_materialProcessor  = std::make_unique<AssimpMaterialProcessor>( );
        m_skeletonProcessor  = std::make_unique<AssimpSkeletonProcessor>( );
        m_animationProcessor = std::make_unique<AssimpAnimationProcessor>( );

        m_supportedExtensions.resize( 20 );
        m_supportedExtensions[ 0 ]  = ".fbx";
        m_supportedExtensions[ 1 ]  = ".gltf";
        m_supportedExtensions[ 2 ]  = ".glb";
        m_supportedExtensions[ 3 ]  = ".obj";
        m_supportedExtensions[ 4 ]  = ".dae";
        m_supportedExtensions[ 5 ]  = ".blend";
        m_supportedExtensions[ 6 ]  = ".3ds";
        m_supportedExtensions[ 7 ]  = ".ase";
        m_supportedExtensions[ 8 ]  = ".ifc";
        m_supportedExtensions[ 9 ]  = ".xgl";
        m_supportedExtensions[ 10 ] = ".zgl";
        m_supportedExtensions[ 11 ] = ".ply";
        m_supportedExtensions[ 12 ] = ".dxf";
        m_supportedExtensions[ 13 ] = ".lwo";
        m_supportedExtensions[ 14 ] = ".lws";
        m_supportedExtensions[ 15 ] = ".lxo";
        m_supportedExtensions[ 16 ] = ".stl";
        m_supportedExtensions[ 17 ] = ".x";
        m_supportedExtensions[ 18 ] = ".ac";
        m_supportedExtensions[ 19 ] = ".ms3d";
    }

    ~Impl( ) = default;

    ImporterResult Import( const AssimpImportDesc &desc );

private:
    ImporterResultCode ValidateInputs( const AssimpImportDesc &desc, ImporterResult &result ) const;
    ImporterResultCode ProcessScene( AssimpImportContext &context ) const;
    ImporterResultCode WriteMeshAsset( AssimpImportContext &context ) const;
};

AssimpImporter::AssimpImporter( ) : m_pImpl( std::make_unique<Impl>( ) )
{
}

AssimpImporter::~AssimpImporter( ) = default;

InteropString AssimpImporter::GetName( ) const
{
    return "Assimp Importer";
}

InteropStringArray AssimpImporter::GetSupportedExtensions( ) const
{
    return { m_pImpl->m_supportedExtensions.data( ), m_pImpl->m_supportedExtensions.size( ) };
}

bool AssimpImporter::CanProcessFileExtension( const InteropString &extension ) const
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

bool AssimpImporter::ValidateFile( const InteropString &filePath ) const
{
    if ( !FileIO::FileExists( filePath ) )
    {
        return false;
    }

    const InteropString extension = AssetPathUtilities::GetFileExtension( filePath );
    return CanProcessFileExtension( extension );
}

ImporterResult AssimpImporter::Import( const AssimpImportDesc &desc ) const
{
    return m_pImpl->Import( desc );
}

ImporterResult AssimpImporter::Impl::Import( const AssimpImportDesc &desc )
{
    spdlog::info( "Starting Assimp import for file: {}", desc.SourceFilePath.Get( ) );
    ImporterResult result;
    if ( const ImporterResultCode validationResult = ValidateInputs( desc, result ); validationResult != ImporterResultCode::Success )
    {
        return result;
    }

    // Phase 1: Load scene and gather statistics
    spdlog::info( "Phase 1: Loading scene and gathering statistics..." );
    if ( !m_sceneLoader->LoadScene( desc.SourceFilePath, desc ) )
    {
        result.ResultCode   = ImporterResultCode::ImportFailed;
        result.ErrorMessage = InteropString( "Failed to load scene file" );
        return result;
    }

    const AssimpSceneStats &stats = m_sceneLoader->GetStats( );

    // Phase 2: Pre-allocate memory based on statistics
    spdlog::info( "Phase 2: Pre-allocating memory - Main arena: {} bytes, Assets: {} estimated", stats.EstimatedArenaSize, stats.EstimatedAssetsCreated );

    m_mainArena = std::make_unique<DZArena>( stats.EstimatedArenaSize );
    m_tempArena = std::make_unique<DZArena>( stats.EstimatedArenaSize / 4 ); // Temp arena is smaller

    AssimpImportContext context;
    context.Scene           = m_sceneLoader->GetScene( );
    context.SourceFilePath  = desc.SourceFilePath;
    context.TargetDirectory = desc.TargetDirectory;
    context.AssetNamePrefix = desc.AssetNamePrefix;
    context.Desc            = desc;
    context.MainArena       = m_mainArena.get( );
    context.TempArena       = m_tempArena.get( );
    context.CreatedAssets.reserve( stats.EstimatedAssetsCreated );
    context.MeshAsset.Name                       = AssetPathUtilities::GetAssetNameFromFilePath( desc.SourceFilePath );
    context.MeshAsset.MorphTargets.Elements      = nullptr;
    context.MeshAsset.MorphTargets.NumElements   = 0;
    context.MeshAsset.UserProperties.Elements    = nullptr;
    context.MeshAsset.UserProperties.NumElements = 0;

    // Phase 3: Process the scene
    result.ResultCode = ProcessScene( context );

    if ( result.ResultCode == ImporterResultCode::Success )
    {
        DZArenaArrayHelper<AssetUriArray, AssetUri>::AllocateAndConstructArray( *m_mainArena, result.CreatedAssets, context.CreatedAssets.size( ) );
        for ( size_t i = 0; i < context.CreatedAssets.size( ); ++i )
        {
            result.CreatedAssets.Elements[ i ] = context.CreatedAssets[ i ];
        }

        spdlog::info( "Assimp import successful. Created {} assets", result.CreatedAssets.NumElements );
    }
    else
    {
        result.ErrorMessage = context.Result.ErrorMessage;
        spdlog::error( "Assimp import failed: {}", result.ErrorMessage.Get( ) );
    }

    return result;
}

ImporterResultCode AssimpImporter::Impl::ValidateInputs( const AssimpImportDesc &desc, ImporterResult &result ) const
{
    if ( !FileIO::FileExists( desc.SourceFilePath ) )
    {
        result.ResultCode   = ImporterResultCode::FileNotFound;
        result.ErrorMessage = InteropString( "Source file not found: " ).Append( desc.SourceFilePath.Get( ) );
        spdlog::error( "{}", result.ErrorMessage.Get( ) );
        return result.ResultCode;
    }

    if ( !FileIO::FileExists( desc.TargetDirectory ) )
    {
        spdlog::info( "Target directory does not exist, attempting to create: {}", desc.TargetDirectory.Get( ) );
        if ( !FileIO::CreateDirectories( desc.TargetDirectory ) )
        {
            result.ResultCode   = ImporterResultCode::WriteFailed;
            result.ErrorMessage = InteropString( "Failed to create target directory: " ).Append( desc.TargetDirectory.Get( ) );
            spdlog::error( "{}", result.ErrorMessage.Get( ) );
            return result.ResultCode;
        }
    }

    return ImporterResultCode::Success;
}

ImporterResultCode AssimpImporter::Impl::ProcessScene( AssimpImportContext &context ) const
{
    auto result = ImporterResultCode::Success;
    // Phase 3.1: Process materials
    if ( context.Desc.ImportMaterials )
    {
        spdlog::info( "Phase 3.1: Processing materials..." );
        if ( result = m_materialProcessor->ProcessAllMaterials( context ); result != ImporterResultCode::Success )
        {
            return result;
        }
    }

    // Phase 3.2: Process skeleton
    if ( context.Desc.ImportSkeletons )
    {
        spdlog::info( "Phase 3.2: Processing skeleton..." );

        SkeletonBuildStats skelStats;
        if ( result = m_skeletonProcessor->PreprocessSkeleton( context, skelStats ); result != ImporterResultCode::Success )
        {
            return result;
        }

        if ( skelStats.TotalJoints > 0 )
        {
            SkeletonAsset skeletonAsset;
            skeletonAsset.Name = context.MeshAsset.Name;
            if ( result = m_skeletonProcessor->BuildSkeleton( context, skeletonAsset ); result != ImporterResultCode::Success )
            {
                return result;
            }

            if ( result = m_skeletonProcessor->WriteSkeletonAsset( context, skeletonAsset ); result != ImporterResultCode::Success )
            {
                return result;
            }

            context.MeshAsset.SkeletonRef = context.SkeletonAssetUri;
        }
    }

    // Phase 3.3: Collect and process meshes
    spdlog::info( "Phase 3.3: Processing meshes..." );
    if ( result = m_meshProcessor->CollectMeshes( context ); result != ImporterResultCode::Success )
    {
        return result;
    }

    // Phase 3.4: Process animations
    if ( context.Desc.ImportAnimations )
    {
        spdlog::info( "Phase 3.4: Processing animations..." );

        AnimationProcessingStats animStats;
        if ( result = m_animationProcessor->PreprocessAnimations( context, animStats ); result != ImporterResultCode::Success )
        {
            return result;
        }

        if ( animStats.TotalAnimations > 0 )
        {
            if ( result = m_animationProcessor->ProcessAllAnimations( context ); result != ImporterResultCode::Success )
            {
                return result;
            }
        }
    }
    // Phase 3.5: Write mesh asset
    if ( context.MeshAsset.SubMeshes.NumElements > 0 )
    {
        spdlog::info( "Phase 3.5: Writing mesh asset..." );
        if ( result = WriteMeshAsset( context ); result != ImporterResultCode::Success )
        {
            return result;
        }
    }
    else
    {
        spdlog::warn( "No processable meshes found in the scene" );
    }
    return ImporterResultCode::Success;
}

ImporterResultCode AssimpImporter::Impl::WriteMeshAsset( AssimpImportContext &context ) const
{
    const InteropString meshAssetFilename = AssetPathUtilities::CreateAssetFileName( context.AssetNamePrefix, context.MeshAsset.Name, "Mesh", MeshAsset::Extension( ) );
    InteropString       meshTargetPath    = context.TargetDirectory.Append( "/" ).Append( meshAssetFilename.Get( ) );
    meshTargetPath                        = FileIO::GetAbsolutePath( meshTargetPath );
    const AssetUri meshUri                = AssetUri::Create( meshAssetFilename );
    context.MeshAsset.Uri                 = meshUri;
    if ( context.MeshAsset.AttributeConfig.NumUVAttributes > 0 )
    {
        DZArenaArrayHelper<UVChannelArray, UVChannel>::AllocateAndConstructArray( *context.MainArena, context.MeshAsset.AttributeConfig.UVChannels,
                                                                                  context.MeshAsset.AttributeConfig.NumUVAttributes );

        for ( uint32_t i = 0; i < context.MeshAsset.AttributeConfig.NumUVAttributes; ++i )
        {
            UVChannel &channel   = context.MeshAsset.AttributeConfig.UVChannels.Elements[ i ];
            channel.SemanticName = "TEXCOORD";
            channel.Index        = i;
        }
    }

    const uint32_t numColorChannels = context.Scene->mMeshes[ 0 ]->GetNumColorChannels( );
    if ( numColorChannels > 0 )
    {
        DZArenaArrayHelper<ColorFormatArray, ColorFormat>::AllocateAndConstructArray( *context.MainArena, context.MeshAsset.AttributeConfig.ColorFormats, numColorChannels );

        for ( uint32_t i = 0; i < numColorChannels; ++i )
        {
            context.MeshAsset.AttributeConfig.ColorFormats.Elements[ i ] = ColorFormat::RGBA;
        }
    }

    BinaryWriter    binaryWriter( meshTargetPath );
    MeshAssetWriter meshWriter( { &binaryWriter } );
    meshWriter.Write( context.MeshAsset );
    if ( const ImporterResultCode result = m_meshProcessor->ProcessAllMeshes( context, meshWriter ); result != ImporterResultCode::Success )
    {
        return result;
    }

    meshWriter.FinalizeAsset( );
    context.CreatedAssets.push_back( meshUri );

    spdlog::info( "Successfully wrote Mesh asset: {}", meshUri.ToInteropString( ).Get( ) );
    return ImporterResultCode::Success;
}
