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

#include <DenOfIzGraphics/Assets/Import/AssimpImporter.h>
#include <DenOfIzGraphics/Assets/Serde/Animation/AnimationAssetWriter.h>
#include <DenOfIzGraphics/Assets/Serde/Material/MaterialAssetWriter.h>
#include <DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetWriter.h>
#include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAssetWriter.h>
#include <DenOfIzGraphics/Assets/Serde/Texture/TextureAssetWriter.h>
#include <DenOfIzGraphics/Assets/Stream/BinaryWriter.h>
#include <DenOfIzGraphics/Utilities/Utilities.h>
#include <algorithm>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <cmath>
#include <filesystem>
#include <limits>
#include <ranges>

using namespace DenOfIz;

AssimpImporter::AssimpImporter( AssimpImporterDesc desc ) : m_desc( std::move( desc ) )
{
    m_importerInfo.Name = "Assimp Importer";
    m_importerInfo.SupportedExtensions.AddElement( ".fbx" );
    m_importerInfo.SupportedExtensions.AddElement( ".gltf" );
    m_importerInfo.SupportedExtensions.AddElement( ".glb" );
    m_importerInfo.SupportedExtensions.AddElement( ".obj" );
    m_importerInfo.SupportedExtensions.AddElement( ".dae" );
    m_importerInfo.SupportedExtensions.AddElement( ".blend" );
    m_importerInfo.SupportedExtensions.AddElement( ".3ds" );
    m_importerInfo.SupportedExtensions.AddElement( ".ase" );
    m_importerInfo.SupportedExtensions.AddElement( ".ifc" );
    m_importerInfo.SupportedExtensions.AddElement( ".xgl" );
    m_importerInfo.SupportedExtensions.AddElement( ".zgl" );
    m_importerInfo.SupportedExtensions.AddElement( ".ply" );
    m_importerInfo.SupportedExtensions.AddElement( ".dxf" );
    m_importerInfo.SupportedExtensions.AddElement( ".lwo" );
    m_importerInfo.SupportedExtensions.AddElement( ".lws" );
    m_importerInfo.SupportedExtensions.AddElement( ".lxo" );
    m_importerInfo.SupportedExtensions.AddElement( ".stl" );
    m_importerInfo.SupportedExtensions.AddElement( ".x" );
    m_importerInfo.SupportedExtensions.AddElement( ".ac" );
    m_importerInfo.SupportedExtensions.AddElement( ".ms3d" );
}

AssimpImporter::~AssimpImporter( ) = default;

ImporterDesc AssimpImporter::GetImporterInfo( ) const
{
    return m_importerInfo;
}

bool AssimpImporter::CanProcessFileExtension( const InteropString &extension ) const
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

bool AssimpImporter::ValidateFile( const InteropString &filePath ) const
{
    if ( !FileIO::FileExists( filePath ) )
    {
        return false;
    }
    return aiIsExtensionSupported( GetFileExtension( filePath ).Get( ) );
}

ImporterResult AssimpImporter::Import( const ImportJobDesc &desc )
{
    LOG( INFO ) << "Starting Assimp import for file: " << desc.SourceFilePath.Get( );

    ImportContext context;
    context.SourceFilePath  = desc.SourceFilePath;
    context.AssetNamePrefix = desc.AssetNamePrefix;
    if ( const std::string targetDirectory = desc.TargetDirectory.Get( ); targetDirectory.ends_with( "/" ) )
    {
        context.TargetDirectory = desc.TargetDirectory;
    }
    else
    {
        context.TargetDirectory = ( targetDirectory + "/" ).c_str( );
    }

    try
    {
        context.Options = AssimpImportOptions::FromBase( desc.Options );
    }
    catch ( const std::bad_cast & )
    {
        context.Result.ResultCode   = ImporterResultCode::InvalidParameters;
        context.Result.ErrorMessage = "Invalid options type provided for AssimpImporter.";
        LOG( ERROR ) << context.Result.ErrorMessage.Get( );
        return context.Result;
    }

    if ( !FileIO::FileExists( context.SourceFilePath ) )
    {
        context.Result.ResultCode   = ImporterResultCode::FileNotFound;
        context.Result.ErrorMessage = "Source file not found: ";
        context.Result.ErrorMessage.Append( context.SourceFilePath.Get( ) );
        LOG( ERROR ) << context.Result.ErrorMessage.Get( );
        return context.Result;
    }

    if ( !FileIO::FileExists( context.TargetDirectory ) )
    {
        LOG( INFO ) << "Target directory does not exist, attempting to create: " << context.TargetDirectory.Get( );
        if ( !FileIO::CreateDirectories( context.TargetDirectory ) )
        {
            context.Result.ResultCode   = ImporterResultCode::WriteFailed;
            context.Result.ErrorMessage = "Failed to create target directory: ";
            context.Result.ErrorMessage.Append( context.TargetDirectory.Get( ) );
            LOG( ERROR ) << context.Result.ErrorMessage.Get( );
            return context.Result;
        }
    }

    Assimp::Importer importer;
    unsigned int     flags = 0;
    ConfigureAssimpImportFlags( context.Options, flags, importer );

    LOG( INFO ) << "Assimp reading file: " << context.SourceFilePath.Get( );
    context.Scene = importer.ReadFile( context.SourceFilePath.Get( ), flags );

    if ( !context.Scene || context.Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !context.Scene->mRootNode )
    {
        context.Result.ResultCode   = ImporterResultCode::ImportFailed;
        context.Result.ErrorMessage = InteropString( "Assimp importer failed: " ).Append( importer.GetErrorString( ) );
        LOG( ERROR ) << context.Result.ErrorMessage.Get( );
        return context.Result;
    }

    context.Result.ResultCode = ImportSceneInternal( context );

    if ( context.Result.ResultCode == ImporterResultCode::Success )
    {
        LOG( INFO ) << "Assimp import successful for: " << context.SourceFilePath.Get( );
        if ( m_desc.BundleManager != nullptr && !m_desc.BundleOutputPath.IsEmpty( ) )
        {
            LOG( INFO ) << "Bundling not yet implemented in importer.";
        }
    }
    else
    {
        LOG( ERROR ) << "Assimp import failed for: " << context.SourceFilePath.Get( ) << " Error: " << context.Result.ErrorMessage.Get( );
    }

    return context.Result;
}

ImporterResultCode AssimpImporter::ImportSceneInternal( ImportContext &context )
{
    MeshAsset meshAsset{ };
    meshAsset.Name                   = GetAssetNameFromFilePath( context.SourceFilePath );
    context.CurrentMeshAssetMetadata = &meshAsset;

    SkeletonAsset skeletonAsset;
    skeletonAsset.Name = meshAsset.Name;

    for ( unsigned int i = 0; i < context.Scene->mNumMeshes; ++i )
    {
        if ( const aiMesh *mesh = context.Scene->mMeshes[ i ]; mesh->HasBones( ) )
        {
            for ( unsigned int b = 0; b < mesh->mNumBones; ++b )
            {
                const aiBone *bone     = mesh->mBones[ b ];
                std::string   boneName = bone->mName.C_Str( );
                if ( !context.BoneNameToInverseBindMatrixMap.contains( boneName ) )
                {
                    context.BoneNameToInverseBindMatrixMap[ boneName ] = bone->mOffsetMatrix;
                }
                if ( !context.BoneNameToIndexMap.contains( boneName ) )
                {
                    context.BoneNameToIndexMap[ boneName ] = -1;
                }
            }
        }
    }

    if ( context.Options.ImportMaterials && context.Scene->HasMaterials( ) )
    {
        LOG( INFO ) << "Processing " << context.Scene->mNumMaterials << " materials...";
        for ( unsigned int i = 0; i < context.Scene->mNumMaterials; ++i )
        {
            AssetUri materialUri;
            if ( const ImporterResultCode matResult = ProcessMaterial( context, context.Scene->mMaterials[ i ], materialUri ); matResult != ImporterResultCode::Success )
            {
                return matResult;
            }
        }
    }

    const InteropString meshAssetFilename = CreateAssetFileName( context.AssetNamePrefix, meshAsset.Name, "Mesh", "dzmesh" );
    const InteropString meshTargetPath    = FileIO::GetAbsolutePath( context.TargetDirectory.Append( "/" ).Append( meshAssetFilename.Get( ) ) );
    const AssetUri      meshUri           = AssetUri::Create( meshAssetFilename );
    meshAsset.Uri                         = meshUri;

    BinaryWriter    binaryMeshWriter( meshTargetPath );
    MeshAssetWriter meshWriter( { &binaryMeshWriter } );

    try
    {

        LOG( INFO ) << "Processing scene graph...";
        if ( const ImporterResultCode nodeResult = ProcessNode( context, context.Scene->mRootNode, &meshWriter, skeletonAsset ); nodeResult != ImporterResultCode::Success )
        {
            LOG( ERROR ) << "Failed processing scene graph.";
            return nodeResult;
        }

        meshWriter.Write( *context.CurrentMeshAssetMetadata );

        if ( context.Options.GenerateLODs )
        {
            GenerateMeshLODs( context, meshWriter );
        }

        meshWriter.FinalizeAsset( );
        RegisterCreatedAsset( context, meshUri, AssetType::Mesh );
        LOG( INFO ) << "Successfully wrote Mesh asset: " << meshUri.ToString( ).Get( );
    }
    catch ( const std::exception &e )
    {
        LOG( ERROR ) << "Failed during Mesh asset writing process " << meshTargetPath.Get( ) << ": " << e.what( );
        context.ErrorMessage = InteropString( "Failed writing mesh asset: " ).Append( e.what( ) );
        return ImporterResultCode::WriteFailed;
    }

    context.CurrentMeshAssetMetadata = nullptr;
    if ( context.Options.ImportSkeletons && !skeletonAsset.Joints.NumElements( ) == 0 )
    {
        LOG( INFO ) << "Processing skeleton...";
        if ( const ImporterResultCode skelResult = ProcessSkeleton( skeletonAsset ); skelResult != ImporterResultCode::Success )
        {
            return skelResult;
        }
        if ( const ImporterResultCode writeSkelResult = WriteSkeletonAsset( context, skeletonAsset ); writeSkelResult != ImporterResultCode::Success )
        {
            LOG( ERROR ) << "Failed to write Skeleton asset.";
            return writeSkelResult;
        }
    }

    if ( context.Options.ImportAnimations && context.Scene->HasAnimations( ) )
    {
        LOG( INFO ) << "Processing " << context.Scene->mNumAnimations << " animations...";
        for ( unsigned int i = 0; i < context.Scene->mNumAnimations; ++i )
        {
            AssetUri animUri;
            if ( const ImporterResultCode animResult = ProcessAnimation( context, context.Scene->mAnimations[ i ], animUri ); animResult != ImporterResultCode::Success )
            {
                LOG( WARNING ) << "Failed to process animation #" << i << ": " << context.ErrorMessage.Get( );
                context.ErrorMessage = "";
            }
        }
    }

    return ImporterResultCode::Success;
}

ImporterResultCode AssimpImporter::ProcessNode( ImportContext &context, const aiNode *node, MeshAssetWriter *meshWriter, SkeletonAsset &skeletonAsset, int32_t parentJointIndex )
{
    if ( !node )
    {
        return ImporterResultCode::Success;
    }
    const std::string nodeNameStr       = node->mName.C_Str( );
    int32_t           currentJointIndex = -1;

    if ( const bool isKnownBone = context.BoneNameToIndexMap.contains( nodeNameStr ); isKnownBone && context.Options.ImportSkeletons )
    {
        bool alreadyAdded = false;
        for ( size_t j = 0; j < skeletonAsset.Joints.NumElements( ); ++j )
        {
            if ( skeletonAsset.Joints.GetElement( j ).Name.Equals( nodeNameStr.c_str( ) ) )
            {
                currentJointIndex = skeletonAsset.Joints.GetElement( j ).Index;
                alreadyAdded      = true;
                break;
            }
        }
        if ( !alreadyAdded )
        {
            currentJointIndex = skeletonAsset.Joints.NumElements( );
            Joint joint;
            joint.Name           = nodeNameStr.c_str( );
            joint.Index          = currentJointIndex;
            joint.ParentIndex    = parentJointIndex;
            joint.LocalTransform = ConvertMatrix( node->mTransformation );
            if ( context.BoneNameToInverseBindMatrixMap.contains( nodeNameStr ) )
            {
                joint.InverseBindMatrix = ConvertMatrix( context.BoneNameToInverseBindMatrixMap[ nodeNameStr ] );
            }
            else
            {
                LOG( WARNING ) << "Inverse bind matrix not found in map for joint: " << joint.Name.Get( ) << ". Using identity.";
            }
            skeletonAsset.Joints.AddElement( joint );
            if ( parentJointIndex >= 0 )
            {
                skeletonAsset.Joints.GetElement( parentJointIndex ).ChildIndices.AddElement( currentJointIndex );
            }
            context.BoneNameToIndexMap[ nodeNameStr ] = currentJointIndex;
        }
    }

    if ( meshWriter )
    {
        for ( unsigned int i = 0; i < node->mNumMeshes; ++i )
        {
            if ( const aiMesh *mesh = context.Scene->mMeshes[ node->mMeshes[ i ] ] )
            {
                if ( const ImporterResultCode meshResult = ProcessMesh( context, mesh, node, *meshWriter ); meshResult != ImporterResultCode::Success )
                {
                    LOG( ERROR ) << "Failed processing mesh #" << node->mMeshes[ i ] << " attached to node " << nodeNameStr;
                    return meshResult;
                }
            }
        }
    }

    for ( unsigned int i = 0; i < node->mNumChildren; ++i )
    {
        if ( const ImporterResultCode childResult = ProcessNode( context, node->mChildren[ i ], meshWriter, skeletonAsset, currentJointIndex );
             childResult != ImporterResultCode::Success )
        {
            return childResult;
        }
    }
    return ImporterResultCode::Success;
}

ImporterResultCode AssimpImporter::ProcessMesh( ImportContext &context, const aiMesh *mesh, const aiNode *node, MeshAssetWriter &assetWriter )
{
    if ( !mesh->HasFaces( ) || !mesh->HasPositions( ) )
    {
        LOG( WARNING ) << "Skipping mesh '" << mesh->mName.C_Str( ) << "' - No faces or positions found.";
        return ImporterResultCode::Success;
    }

    LOG( INFO ) << "Processing mesh: " << mesh->mName.C_Str( ) << " with " << mesh->mNumVertices << " vertices and " << mesh->mNumFaces << " faces for writing.";

    SubMeshData subMesh;
    subMesh.Name = SanitizeAssetName( mesh->mName.C_Str( ) );
    if ( subMesh.Name.IsEmpty( ) )
    {
        subMesh.Name = InteropString( "SubMesh_" ).Append( std::to_string( context.CurrentSubMeshIndex ).c_str( ) );
    }
    subMesh.Topology    = PrimitiveTopology::Triangle;
    subMesh.LODLevel    = 0;
    subMesh.NumVertices = mesh->mNumVertices;
    subMesh.NumIndices  = mesh->mNumFaces * 3;
    subMesh.IndexType   = IndexType::Uint32;
    CalculateMeshBounds( mesh, context.Options.ScaleFactor, subMesh.MinBounds, subMesh.MaxBounds );

    if ( context.Options.ImportMaterials && context.Scene->HasMaterials( ) && mesh->mMaterialIndex < context.Scene->mNumMaterials )
    {
        if ( const aiMaterial *material = context.Scene->mMaterials[ mesh->mMaterialIndex ] )
        {
            if ( const std::string matNameStr = material->GetName( ).C_Str( ); context.MaterialNameToAssetUriMap.contains( matNameStr ) )
            {
                subMesh.MaterialRef = context.MaterialNameToAssetUriMap[ matNameStr ];
            }
            else
            {
                LOG( WARNING ) << "Material '" << matNameStr << "' not found in processed map for mesh '" << mesh->mName.C_Str( ) << "'.";
            }
        }
    }

    VertexEnabledAttributes attributes;
    VertexAttributeConfig   attributeConfig;
    attributeConfig.MaxBoneInfluences     = context.Options.LimitBoneWeights ? context.Options.MaxBoneWeightsPerVertex : 4;
    attributes.Position                   = mesh->HasPositions( );
    attributes.Normal                     = mesh->HasNormals( );
    attributes.Tangent                    = mesh->HasTangentsAndBitangents( );
    attributes.Bitangent                  = mesh->HasTangentsAndBitangents( );
    attributes.UV                         = mesh->HasTextureCoords( 0 );
    attributes.Color                      = mesh->HasVertexColors( 0 );
    attributes.BlendIndices               = mesh->HasBones( ) && context.Options.ImportSkeletons;
    attributes.BlendWeights               = mesh->HasBones( ) && context.Options.ImportSkeletons;
    attributeConfig.NumPositionComponents = 3;
    attributeConfig.NumUVAttributes       = 0;
    for ( unsigned int uvChan = 0; uvChan < AI_MAX_NUMBER_OF_TEXTURECOORDS && mesh->HasTextureCoords( uvChan ); ++uvChan )
    {
        attributeConfig.NumUVAttributes++;
        UVChannel channel;
        channel.Index        = uvChan;
        channel.SemanticName = InteropString( "TEXCOORD" ).Append( std::to_string( uvChan ).c_str( ) );
        attributeConfig.UVChannels.AddElement( channel );
    }
    attributeConfig.ColorFormats.Clear( );
    for ( unsigned int colChan = 0; colChan < AI_MAX_NUMBER_OF_COLOR_SETS && mesh->HasVertexColors( colChan ); ++colChan )
    {
        attributeConfig.ColorFormats.AddElement( ColorFormat::RGBA );
    }

    if ( context.CurrentMeshAssetMetadata )
    {
        context.CurrentMeshAssetMetadata->SubMeshes.AddElement( subMesh );
        context.CurrentMeshAssetMetadata->EnabledAttributes = attributes;
        context.CurrentMeshAssetMetadata->AttributeConfig   = attributeConfig;
    }
    else
    {
        LOG( ERROR ) << "CurrentMeshAssetMetadata is null during ProcessMesh for " << mesh->mName.C_Str( );
        return ImporterResultCode::ImportFailed;
    }

    std::vector<std::vector<std::pair<int, float>>> boneInfluences( mesh->mNumVertices );
    if ( attributes.BlendIndices )
    {
        for ( unsigned int b = 0; b < mesh->mNumBones; ++b )
        {
            const aiBone *bone = mesh->mBones[ b ];
            if ( const std::string boneName = bone->mName.C_Str( ); context.BoneNameToIndexMap.contains( boneName ) )
            {
                auto boneIndex = context.BoneNameToIndexMap[ boneName ];
                if ( boneIndex == -1 )
                {
                    LOG( WARNING ) << "Bone '" << boneName << "' has placeholder index -1 during mesh processing for '" << mesh->mName.C_Str( ) << "'.";
                }
                for ( unsigned int w = 0; w < bone->mNumWeights; ++w )
                {
                    if ( const aiVertexWeight &weight = bone->mWeights[ w ]; weight.mVertexId < mesh->mNumVertices )
                        boneInfluences[ weight.mVertexId ].emplace_back( boneIndex, weight.mWeight );
                }
            }
            else
            {
                LOG( ERROR ) << "Bone '" << boneName << "' not found in map during mesh processing for '" << mesh->mName.C_Str( ) << "'!";
            }
        }
        for ( unsigned int v = 0; v < mesh->mNumVertices; ++v )
        {
            auto &influences = boneInfluences[ v ];
            std::ranges::sort( influences, []( const auto &a, const auto &b ) { return a.second > b.second; } );
            if ( influences.size( ) > attributeConfig.MaxBoneInfluences )
            {
                influences.resize( attributeConfig.MaxBoneInfluences );
            }
            float totalWeight = 0.0f;
            for ( const auto &val : influences | std::views::values )
            {
                totalWeight += val;
            }
            if ( totalWeight > 1e-6f )
            {
                for ( auto &val : influences | std::views::values )
                {
                    val /= totalWeight;
                }
            }
        }
    }

    LOG( INFO ) << "Writing " << mesh->mNumVertices << " vertices for submesh " << context.CurrentSubMeshIndex;
    for ( unsigned int i = 0; i < mesh->mNumVertices; ++i )
    {
        MeshVertex vertex;
        if ( attributes.Position )
        {
            vertex.Position = { mesh->mVertices[ i ].x, mesh->mVertices[ i ].y, mesh->mVertices[ i ].z, 1.0f };
            if ( context.Options.ScaleFactor != 1.0f )
            {
                vertex.Position.X *= context.Options.ScaleFactor;
                vertex.Position.Y *= context.Options.ScaleFactor;
                vertex.Position.Z *= context.Options.ScaleFactor;
            }
        }
        if ( attributes.Normal )
        {
            vertex.Normal = { mesh->mNormals[ i ].x, mesh->mNormals[ i ].y, mesh->mNormals[ i ].z, 0.0f };
        }
        if ( attributes.Tangent )
        {
            vertex.Tangent = { mesh->mTangents[ i ].x, mesh->mTangents[ i ].y, mesh->mTangents[ i ].z, 1.0f };
        }
        if ( attributes.Bitangent )
        {
            vertex.Bitangent = { mesh->mBitangents[ i ].x, mesh->mBitangents[ i ].y, mesh->mBitangents[ i ].z, 1.0f };
        }
        if ( attributes.UV )
        {
            vertex.UVs.Resize( attributeConfig.NumUVAttributes );
            for ( uint32_t uvChan = 0; uvChan < attributeConfig.NumUVAttributes; ++uvChan )
            {
                if ( mesh->HasTextureCoords( uvChan ) )
                {
                    vertex.UVs.GetElement( uvChan ) = ConvertVector2( mesh->mTextureCoords[ uvChan ][ i ] );
                    if ( context.Options.FlipUVs )
                    {
                        vertex.UVs.GetElement( uvChan ).Y = 1.0f - vertex.UVs.GetElement( uvChan ).Y;
                    }
                }
            }
        }
        if ( attributes.Color )
        {
            vertex.Colors.Resize( attributeConfig.ColorFormats.NumElements( ) );
            for ( uint32_t colChan = 0; colChan < attributeConfig.ColorFormats.NumElements( ); ++colChan )
            {
                if ( mesh->HasVertexColors( colChan ) )
                {
                    vertex.Colors.GetElement( colChan ) = ConvertColor( mesh->mColors[ colChan ][ i ] );
                }
            }
        }
        if ( attributes.BlendIndices )
        {
            const auto &influences = boneInfluences[ i ];
            for ( size_t infIdx = 0; infIdx < attributeConfig.MaxBoneInfluences; ++infIdx )
            {
                int   boneIdx = 0;
                float weight  = 0.0f;
                if ( infIdx < influences.size( ) )
                {
                    boneIdx = influences[ infIdx ].first;
                    weight  = influences[ infIdx ].second;
                }
                if ( infIdx == 0 )
                {
                    vertex.BoneIndices.X = boneIdx;
                    vertex.BoneWeights.X = weight;
                }
                else if ( infIdx == 1 )
                {
                    vertex.BoneIndices.Y = boneIdx;
                    vertex.BoneWeights.Y = weight;
                }
                else if ( infIdx == 2 )
                {
                    vertex.BoneIndices.Z = boneIdx;
                    vertex.BoneWeights.Z = weight;
                }
                else if ( infIdx == 3 )
                {
                    vertex.BoneIndices.W = boneIdx;
                    vertex.BoneWeights.W = weight;
                }
            }
        }
        assetWriter.AddVertex( vertex );
    }

    LOG( INFO ) << "Writing " << mesh->mNumFaces * 3 << " indices for submesh " << context.CurrentSubMeshIndex;
    if ( mesh->HasFaces( ) )
    {
        for ( unsigned int i = 0; i < mesh->mNumFaces; ++i )
        {
            if ( const aiFace &face = mesh->mFaces[ i ]; face.mNumIndices == 3 )
            {
                assetWriter.AddIndex32( face.mIndices[ 0 ] );
                assetWriter.AddIndex32( face.mIndices[ 1 ] );
                assetWriter.AddIndex32( face.mIndices[ 2 ] );
            }
        }
    }
    context.CurrentSubMeshIndex++;
    return ImporterResultCode::Success;
}

ImporterResultCode AssimpImporter::ProcessMaterial( ImportContext &context, const aiMaterial *material, AssetUri &outAssetUri )
{
    const std::string matNameStr = material->GetName( ).C_Str( );
    InteropString     matName    = SanitizeAssetName( matNameStr.c_str( ) );
    if ( matName.IsEmpty( ) )
    {
        matName = InteropString( "Material_" ).Append( std::to_string( context.MaterialNameToAssetUriMap.size( ) ).c_str( ) );
    }
    LOG( INFO ) << "Processing material: " << matName.Get( );
    if ( context.MaterialNameToAssetUriMap.contains( matNameStr ) )
    {
        outAssetUri = context.MaterialNameToAssetUriMap[ matNameStr ];
        LOG( INFO ) << "Material '" << matName.Get( ) << "' already processed, reusing URI: " << outAssetUri.ToString( ).Get( );
        return ImporterResultCode::Success;
    }
    MaterialAsset matAsset;
    matAsset.Name = matName;
    aiColor4D color;
    if ( material->Get( AI_MATKEY_COLOR_DIFFUSE, color ) == AI_SUCCESS )
    {
        matAsset.BaseColorFactor = ConvertColor( color );
    }
    if ( AssetUri albedoUri; context.Options.ImportTextures && ProcessTexture( context, material, aiTextureType_DIFFUSE, "Albedo", albedoUri ) == ImporterResultCode::Success )
    {
        matAsset.AlbedoMapRef = albedoUri;
    }
    if ( AssetUri normalUri; context.Options.ImportTextures && ( ProcessTexture( context, material, aiTextureType_NORMALS, "Normal", normalUri ) == ImporterResultCode::Success ||
                                                                 ProcessTexture( context, material, aiTextureType_HEIGHT, "Normal", normalUri ) == ImporterResultCode::Success ) )
    {
        matAsset.NormalMapRef = normalUri;
    }
    if ( AssetUri mrUri; context.Options.ImportTextures && ProcessTexture( context, material, aiTextureType_METALNESS, "MetallicRoughness", mrUri ) == ImporterResultCode::Success )
    {
        float factor;
        matAsset.MetallicRoughnessMapRef = mrUri;
        if ( material->Get( AI_MATKEY_METALLIC_FACTOR, factor ) == AI_SUCCESS )
        {
            matAsset.MetallicFactor = factor;
        }
        if ( material->Get( AI_MATKEY_ROUGHNESS_FACTOR, factor ) == AI_SUCCESS )
        {
            matAsset.RoughnessFactor = factor;
        }
    }
    if ( AssetUri emUri; context.Options.ImportTextures && ProcessTexture( context, material, aiTextureType_EMISSIVE, "Emissive", emUri ) == ImporterResultCode::Success )
    {
        matAsset.EmissiveMapRef = emUri;
    }
    if ( material->Get( AI_MATKEY_COLOR_EMISSIVE, color ) == AI_SUCCESS )
    {
        matAsset.EmissiveFactor = { color.r, color.g, color.b };
    }
    if ( AssetUri ocUri; context.Options.ImportTextures && ProcessTexture( context, material, aiTextureType_AMBIENT_OCCLUSION, "Occlusion", ocUri ) == ImporterResultCode::Success )
    {
        matAsset.OcclusionMapRef = ocUri;
    }
    if ( float opacity; material->Get( AI_MATKEY_OPACITY, opacity ) == AI_SUCCESS )
    {
        matAsset.AlphaBlend = opacity < 1.0f;
    }
    if ( int twoSided; material->Get( AI_MATKEY_TWOSIDED, twoSided ) == AI_SUCCESS )
    {
        matAsset.DoubleSided = twoSided != 0;
    }
    if ( const ImporterResultCode writeResult = WriteMaterialAsset( context, matAsset, outAssetUri ); writeResult == ImporterResultCode::Success )
    {
        context.MaterialNameToAssetUriMap[ matNameStr ] = outAssetUri;
        LOG( INFO ) << "Successfully wrote material asset: " << outAssetUri.ToString( ).Get( );
    }
    else
    {
        LOG( ERROR ) << "Failed to write material asset for: " << matName.Get( );
        return writeResult;
    }
    return ImporterResultCode::Success;
}

ImporterResultCode AssimpImporter::ProcessTexture( ImportContext &context, const aiMaterial *material, aiTextureType textureType, const InteropString &semanticName,
                                                   AssetUri &outAssetUri )
{
    aiString aiPath;
    if ( material->GetTexture( textureType, 0, &aiPath ) != AI_SUCCESS )
    {
        return ImporterResultCode::ResourceUnavailable;
    }
    const std::string texPathStr = aiPath.C_Str( );
    if ( texPathStr.empty( ) )
    {
        return ImporterResultCode::ResourceUnavailable;
    }
    if ( texPathStr[ 0 ] == '*' )
    {
        LOG( INFO ) << "Processing embedded texture for material '" << material->GetName( ).C_Str( ) << "', semantic: " << semanticName.Get( );
        if ( int textureIndex = std::stoi( texPathStr.substr( 1 ) ); textureIndex >= 0 && textureIndex < static_cast<int>( context.Scene->mNumTextures ) )
        {
            return WriteTextureAsset( context, context.Scene->mTextures[ textureIndex ], semanticName, outAssetUri );
        }
        LOG( ERROR ) << "Invalid embedded texture index " << texPathStr.substr( 1 ) << " for material '" << material->GetName( ).C_Str( ) << "'.";
        return ImporterResultCode::ImportFailed;
    }
    LOG( INFO ) << "Processing external texture reference: '" << texPathStr << "' for semantic: " << semanticName.Get( );
    if ( context.TexturePathToAssetUriMap.contains( texPathStr ) )
    {
        outAssetUri = context.TexturePathToAssetUriMap[ texPathStr ];
        LOG( INFO ) << "Texture '" << texPathStr << "' already processed, reusing URI: " << outAssetUri.ToString( ).Get( );
        return ImporterResultCode::Success;
    }
    const std::filesystem::path modelPath           = context.SourceFilePath.Get( );
    const std::filesystem::path texturePath         = texPathStr;
    const std::filesystem::path absoluteTexturePath = texturePath.is_absolute( ) ? texturePath : absolute( modelPath.parent_path( ) / texturePath );
    if ( !exists( absoluteTexturePath ) )
    {
        LOG( ERROR ) << "External texture file not found: " << absoluteTexturePath.string( ) << " (referenced by material '" << material->GetName( ).C_Str( ) << "')";
        return ImporterResultCode::FileNotFound;
    }
    TextureAsset texAsset;
    texAsset.Name                       = SanitizeAssetName( texturePath.filename( ).stem( ).string( ).c_str( ) );
    texAsset.SourcePath                 = absoluteTexturePath.string( ).c_str( );
    texAsset.Width                      = 0;
    texAsset.Height                     = 0;
    texAsset.Format                     = Format::Undefined;
    texAsset.Dimension                  = TextureDimension::Texture2D;
    texAsset.MipLevels                  = 1;
    texAsset.ArraySize                  = 1;
    const InteropString assetFilename   = CreateAssetFileName( context.AssetNamePrefix, texAsset.Name, "Texture", "dztex" );
    const InteropString targetAssetPath = FileIO::GetAbsolutePath( context.TargetDirectory.Append( "/" ).Append( assetFilename.Get( ) ) );
    outAssetUri                         = AssetUri::Create( assetFilename );
    texAsset.Uri                        = outAssetUri;
    LOG( INFO ) << "Creating TextureAsset file: " << targetAssetPath.Get( ) << " referencing source: " << texAsset.SourcePath.Get( );
    try
    {
        BinaryWriter       writer( targetAssetPath );
        TextureAssetWriter assetWriter( { &writer } );
        assetWriter.Write( texAsset );
        assetWriter.Finalize( );
    }
    catch ( const std::exception &e )
    {
        LOG( ERROR ) << "Failed to write TextureAsset file " << targetAssetPath.Get( ) << ": " << e.what( );
        return ImporterResultCode::WriteFailed;
    }
    RegisterCreatedAsset( context, outAssetUri, AssetType::Texture );
    context.TexturePathToAssetUriMap[ texPathStr ] = outAssetUri;
    return ImporterResultCode::Success;
}

ImporterResultCode AssimpImporter::ProcessSkeleton( SkeletonAsset &skeletonAsset )
{
    LOG( INFO ) << "Finalizing skeleton with " << skeletonAsset.Joints.NumElements( ) << " joints.";
    std::function<void( int32_t, const Float_4x4 & )> calculateGlobalTransform = [ & ]( const int32_t jointIndex, const Float_4x4 & )
    {
        if ( jointIndex < 0 || jointIndex >= static_cast<int32_t>( skeletonAsset.Joints.NumElements( ) ) )
        {
            return;
        }
        Joint &joint = skeletonAsset.Joints.GetElement( jointIndex );

        joint.GlobalTransform = joint.LocalTransform;
        for ( size_t i = 0; i < joint.ChildIndices.NumElements( ); ++i )
        {
            calculateGlobalTransform( joint.ChildIndices.GetElement( i ), joint.GlobalTransform );
        }
    };
    constexpr Float_4x4 identityMatrix{ };
    for ( size_t i = 0; i < skeletonAsset.Joints.NumElements( ); ++i )
    {
        if ( skeletonAsset.Joints.GetElement( i ).ParentIndex == -1 )
        {
            calculateGlobalTransform( i, identityMatrix );
        }
    }
    return ImporterResultCode::Success;
}

ImporterResultCode AssimpImporter::ProcessAnimation( ImportContext &context, const aiAnimation *animation, AssetUri &outAssetUri )
{
    const std::string animNameStr = animation->mName.C_Str( );
    InteropString     animName    = SanitizeAssetName( animNameStr.c_str( ) );
    if ( animName.IsEmpty( ) )
    {
        animName = InteropString( "Animation_" ).Append( std::to_string( context.Result.CreatedAssets.NumElements( ) ).c_str( ) );
    }
    LOG( INFO ) << "Processing animation: " << animName.Get( ) << " Duration: " << animation->mDuration << " Ticks/Sec: " << animation->mTicksPerSecond;

    AnimationAsset animAsset;
    animAsset.Name        = animName;
    animAsset.SkeletonRef = context.SkeletonAssetUri;

    AnimationClip clip;
    clip.Name                   = animName;
    const double ticksPerSecond = animation->mTicksPerSecond > 0 ? animation->mTicksPerSecond : 24.0;
    clip.Duration               = static_cast<float>( animation->mDuration / ticksPerSecond );

    LOG( INFO ) << "Processing " << animation->mNumChannels << " joint animation channels (raw keys).";
    clip.Tracks.Resize( animation->mNumChannels );
    for ( unsigned int i = 0; i < animation->mNumChannels; ++i )
    {
        const aiNodeAnim *nodeAnim = animation->mChannels[ i ];
        JointAnimTrack   &track    = clip.Tracks.GetElement( i );
        track.JointName            = nodeAnim->mNodeName.C_Str( );

        if ( !context.BoneNameToIndexMap.contains( track.JointName.Get( ) ) )
        {
            LOG( WARNING ) << "Animation channel '" << track.JointName.Get( ) << "' does not correspond to a known skeleton joint. Skipping channel.";
            continue;
        }

        track.PositionKeys.Resize( nodeAnim->mNumPositionKeys );
        for ( unsigned int k = 0; k < nodeAnim->mNumPositionKeys; ++k )
        {
            PositionKey &key = track.PositionKeys.GetElement( k );
            key.Timestamp    = static_cast<float>( nodeAnim->mPositionKeys[ k ].mTime / ticksPerSecond );
            key.Value        = ConvertVector3( nodeAnim->mPositionKeys[ k ].mValue );
        }

        track.RotationKeys.Resize( nodeAnim->mNumRotationKeys );
        for ( unsigned int k = 0; k < nodeAnim->mNumRotationKeys; ++k )
        {
            RotationKey &key = track.RotationKeys.GetElement( k );
            key.Timestamp    = static_cast<float>( nodeAnim->mRotationKeys[ k ].mTime / ticksPerSecond );
            key.Value        = ConvertQuaternion( nodeAnim->mRotationKeys[ k ].mValue );
        }

        track.ScaleKeys.Resize( nodeAnim->mNumScalingKeys );
        for ( unsigned int k = 0; k < nodeAnim->mNumScalingKeys; ++k )
        {
            ScaleKey &key = track.ScaleKeys.GetElement( k );
            key.Timestamp = static_cast<float>( nodeAnim->mScalingKeys[ k ].mTime / ticksPerSecond );
            key.Value     = ConvertVector3( nodeAnim->mScalingKeys[ k ].mValue );
        }
    }

    LOG( INFO ) << "Processing " << animation->mNumMorphMeshChannels << " morph target animation channels.";
    clip.MorphTracks.Resize( animation->mNumMorphMeshChannels );
    for ( unsigned int i = 0; i < animation->mNumMorphMeshChannels; ++i )
    {
        const aiMeshMorphAnim *morphAnim = animation->mMorphMeshChannels[ i ];
        MorphAnimTrack        &track     = clip.MorphTracks.GetElement( i );
        track.Name                       = morphAnim->mName.C_Str( );
        track.Keyframes.Resize( morphAnim->mNumKeys );
        for ( unsigned int k = 0; k < morphAnim->mNumKeys; ++k )
        {
            MorphKeyframe        &keyframe = track.Keyframes.GetElement( k );
            const aiMeshMorphKey &aiKey    = morphAnim->mKeys[ k ];
            keyframe.Timestamp             = static_cast<float>( aiKey.mTime / ticksPerSecond );
            keyframe.Weight                = aiKey.mNumValuesAndWeights > 0 ? static_cast<float>( aiKey.mWeights[ 0 ] ) : 0.0f;
        }
    }

    animAsset.Animations.AddElement( clip );

    if ( const ImporterResultCode writeResult = WriteAnimationAsset( context, animAsset, outAssetUri ); writeResult != ImporterResultCode::Success )
    {
        LOG( ERROR ) << "Failed to write Animation asset for: " << animName.Get( );
        return writeResult;
    }
    LOG( INFO ) << "Successfully wrote animation asset: " << outAssetUri.ToString( ).Get( );
    return ImporterResultCode::Success;
}

void AssimpImporter::CalculateMeshBounds( const aiMesh *mesh, float scaleFactor, Float_3 &outMin, Float_3 &outMax )
{
    if ( !mesh || !mesh->HasPositions( ) || mesh->mNumVertices == 0 )
    {
        outMin = outMax = { 0, 0, 0 };
        return;
    }
    outMin = { std::numeric_limits<float>::max( ), std::numeric_limits<float>::max( ), std::numeric_limits<float>::max( ) };
    outMax = { std::numeric_limits<float>::lowest( ), std::numeric_limits<float>::lowest( ), std::numeric_limits<float>::lowest( ) };
    for ( unsigned int i = 0; i < mesh->mNumVertices; ++i )
    {
        const aiVector3D &pos = mesh->mVertices[ i ];
        outMin.X              = std::min( outMin.X, pos.x );
        outMin.Y              = std::min( outMin.Y, pos.y );
        outMin.Z              = std::min( outMin.Z, pos.z );
        outMax.X              = std::max( outMax.X, pos.x );
        outMax.Y              = std::max( outMax.Y, pos.y );
        outMax.Z              = std::max( outMax.Z, pos.z );
    }
    if ( scaleFactor != 1.0f )
    {
        outMin.X *= scaleFactor;
        outMin.Y *= scaleFactor;
        outMin.Z *= scaleFactor;
        outMax.X *= scaleFactor;
        outMax.Y *= scaleFactor;
        outMax.Z *= scaleFactor;
    }
}

void AssimpImporter::GenerateMeshLODs( const ImportContext &context, MeshAssetWriter &meshWriter )
{
    LOG( WARNING ) << "LOD generation requires integrating a mesh simplification library (like meshoptimizer). Skipping LOD generation.";
}

void AssimpImporter::ConfigureAssimpImportFlags( const AssimpImportOptions &options, unsigned int &flags, Assimp::Importer &importer )
{
    flags = 0;
    if ( options.TriangulateMeshes )
    {
        flags |= aiProcess_Triangulate;
    }
    if ( options.JoinIdenticalVertices )
    {
        flags |= aiProcess_JoinIdenticalVertices;
    }
    if ( options.CalculateTangentSpace )
    {
        flags |= aiProcess_CalcTangentSpace;
    }
    if ( options.LimitBoneWeights )
    {
        flags |= aiProcess_LimitBoneWeights;
        importer.SetPropertyInteger( AI_CONFIG_PP_LBW_MAX_WEIGHTS, options.MaxBoneWeightsPerVertex );
    }
    if ( options.ConvertToRightHanded )
    {
        flags |= aiProcess_MakeLeftHanded | aiProcess_FlipWindingOrder | aiProcess_FlipUVs;
    }
    else if ( options.FlipUVs )
    {
        flags |= aiProcess_FlipUVs;
    }
    if ( options.RemoveRedundantMaterials )
    {
        flags |= aiProcess_RemoveRedundantMaterials;
    }
    flags |= aiProcess_ImproveCacheLocality;
    flags |= aiProcess_SortByPType;
    if ( options.GenerateNormals )
    {
        if ( options.SmoothNormals )
        {
            flags |= aiProcess_GenSmoothNormals;
            importer.SetPropertyFloat( AI_CONFIG_PP_GSN_MAX_SMOOTHING_ANGLE, options.SmoothNormalsAngle );
        }
        else
        {
            flags |= aiProcess_GenNormals;
        }
    }
    if ( options.PreTransformVertices )
    {
        flags |= aiProcess_PreTransformVertices;
    }
    else
    {
        if ( options.OptimizeGraph )
        {
            flags |= aiProcess_OptimizeGraph;
        }
    }
    if ( options.OptimizeMeshes )
    {
        flags |= aiProcess_OptimizeMeshes;
    }
    if ( options.MergeMeshes )
    {
        flags |= aiProcess_OptimizeMeshes | aiProcess_JoinIdenticalVertices | aiProcess_SortByPType;
        if ( flags & aiProcess_PreTransformVertices )
        {
            LOG( WARNING ) << "MergeMeshes and PreTransformVertices may conflict. Disabling PreTransformVertices.";
            flags &= ~aiProcess_PreTransformVertices;
        }
        if ( !( flags & aiProcess_PreTransformVertices ) )
        {
            flags |= aiProcess_OptimizeGraph;
        }
    }
    if ( options.DropNormals )
    {
        flags |= aiProcess_DropNormals;
        flags &= ~( aiProcess_GenNormals | aiProcess_GenSmoothNormals );
    }
}

ImporterResultCode AssimpImporter::WriteMaterialAsset( ImportContext &context, const MaterialAsset &materialAsset, AssetUri &outAssetUri )
{
    const InteropString assetFilename   = CreateAssetFileName( context.AssetNamePrefix, materialAsset.Name, "Material", "dzmat" );
    const InteropString targetAssetPath = FileIO::GetAbsolutePath( context.TargetDirectory.Append( "/" ).Append( assetFilename.Get( ) ) );
    outAssetUri                         = AssetUri::Create( assetFilename );
    MaterialAsset mutableAsset          = materialAsset;
    mutableAsset.Uri                    = outAssetUri;
    LOG( INFO ) << "Writing Material asset to: " << targetAssetPath.Get( );
    try
    {
        BinaryWriter              writer( targetAssetPath );
        const MaterialAssetWriter assetWriter( { &writer } );
        assetWriter.Write( mutableAsset );
        RegisterCreatedAsset( context, outAssetUri, AssetType::Material );
    }
    catch ( const std::exception &e )
    {
        LOG( ERROR ) << "Failed to write Material asset " << targetAssetPath.Get( ) << ": " << e.what( );
        context.ErrorMessage = InteropString( "Failed to write Material asset " ).Append( targetAssetPath.Get( ) ).Append( ": " ).Append( e.what( ) );
        return ImporterResultCode::WriteFailed;
    }
    return ImporterResultCode::Success;
}

ImporterResultCode AssimpImporter::WriteTextureAsset( ImportContext &context, const aiTexture *texture, const InteropString &semanticName, AssetUri &outAssetUri )
{
    InteropString texName = SanitizeAssetName( texture->mFilename.length > 0 ? texture->mFilename.C_Str( ) : semanticName.Get( ) );
    if ( texName.IsEmpty( ) )
    {
        texName = InteropString( semanticName ).Append( "_Tex_" ).Append( std::to_string( context.Result.CreatedAssets.NumElements( ) ).c_str( ) );
    }
    const InteropString assetFilename   = CreateAssetFileName( context.AssetNamePrefix, texName, "Texture", "dztex" );
    const InteropString targetAssetPath = FileIO::GetAbsolutePath( context.TargetDirectory.Append( "/" ).Append( assetFilename.Get( ) ) );
    outAssetUri                         = AssetUri::Create( assetFilename );
    LOG( INFO ) << "Writing embedded Texture asset to: " << targetAssetPath.Get( ) << " (Semantic: " << semanticName.Get( ) << ")";
    TextureAsset texAsset;
    texAsset.Name           = texName;
    texAsset.Uri            = outAssetUri;
    const bool isCompressed = texture->mHeight == 0;
    texAsset.Width          = texture->mWidth;
    texAsset.Height         = isCompressed ? 0 : texture->mHeight;
    texAsset.Depth          = 1;
    texAsset.Dimension      = TextureDimension::Texture2D;
    texAsset.MipLevels      = 1;
    texAsset.ArraySize      = 1;
    texAsset.Format         = Format::Undefined;
    if ( !isCompressed )
    {
        LOG( WARNING ) << "Assuming RGBA8 format for uncompressed embedded texture: " << texName.Get( );
        texAsset.Format       = Format::R8G8B8A8Unorm;
        texAsset.BitsPerPixel = 32;
        texAsset.RowPitch     = texAsset.Width * 4;
        texAsset.NumRows      = texAsset.Height;
        texAsset.SlicePitch   = texAsset.RowPitch * texAsset.Height;
    }
    try
    {
        BinaryWriter       writer( targetAssetPath );
        TextureAssetWriter assetWriter( { &writer } );
        assetWriter.Write( texAsset );
        InteropArray<Byte> textureData;
        size_t             dataSize = isCompressed ? texture->mWidth : static_cast<size_t>( texAsset.SlicePitch );
        textureData.MemCpy( texture->pcData, dataSize );
        assetWriter.AddPixelData( textureData );
        assetWriter.Finalize( );
        RegisterCreatedAsset( context, outAssetUri, AssetType::Texture );
    }
    catch ( const std::exception &e )
    {
        LOG( ERROR ) << "Failed to write embedded Texture asset " << targetAssetPath.Get( ) << ": " << e.what( );
        context.ErrorMessage = InteropString( "Failed to write embedded Texture asset " ).Append( targetAssetPath.Get( ) ).Append( ": " ).Append( e.what( ) );
        return ImporterResultCode::WriteFailed;
    }
    return ImporterResultCode::Success;
}

ImporterResultCode AssimpImporter::WriteSkeletonAsset( ImportContext &context, const SkeletonAsset &skeletonAsset )
{
    const InteropString assetFilename   = CreateAssetFileName( context.AssetNamePrefix, skeletonAsset.Name, "Skeleton", "dzskel" );
    const InteropString targetAssetPath = FileIO::GetAbsolutePath( context.TargetDirectory.Append( "/" ).Append( assetFilename.Get( ) ) );
    context.SkeletonAssetUri            = AssetUri::Create( assetFilename );
    SkeletonAsset mutableAsset          = skeletonAsset;
    mutableAsset.Uri                    = context.SkeletonAssetUri;
    LOG( INFO ) << "Writing Skeleton asset to: " << targetAssetPath.Get( );
    try
    {
        BinaryWriter              writer( targetAssetPath );
        const SkeletonAssetWriter assetWriter( { &writer } );
        assetWriter.Write( mutableAsset );
        RegisterCreatedAsset( context, context.SkeletonAssetUri, AssetType::Skeleton );
    }
    catch ( const std::exception &e )
    {
        LOG( ERROR ) << "Failed to write Skeleton asset " << targetAssetPath.Get( ) << ": " << e.what( );
        context.ErrorMessage = "Failed to write skeleton asset: ";
        context.ErrorMessage.Append( e.what( ) );
        return ImporterResultCode::WriteFailed;
    }
    return ImporterResultCode::Success;
}

ImporterResultCode AssimpImporter::WriteAnimationAsset( ImportContext &context, const AnimationAsset &animationAsset, AssetUri &outAssetUri )
{
    const InteropString assetFilename   = CreateAssetFileName( context.AssetNamePrefix, animationAsset.Name, "Animation", "dzanim" );
    const InteropString targetAssetPath = FileIO::GetAbsolutePath( context.TargetDirectory.Append( "/" ).Append( assetFilename.Get( ) ) );
    outAssetUri                         = AssetUri::Create( assetFilename );
    AnimationAsset mutableAsset         = animationAsset;
    mutableAsset.Uri                    = outAssetUri;
    LOG( INFO ) << "Writing Animation asset to: " << targetAssetPath.Get( );
    try
    {
        BinaryWriter         writer( targetAssetPath );
        AnimationAssetWriter assetWriter( { &writer } );
        assetWriter.Write( mutableAsset );
        RegisterCreatedAsset( context, outAssetUri, AssetType::Animation );
    }
    catch ( const std::exception &e )
    {
        LOG( ERROR ) << "Failed to write Animation asset " << targetAssetPath.Get( ) << ": " << e.what( );
        context.ErrorMessage = InteropString( "Failed to write Animation asset " ).Append( targetAssetPath.Get( ) ).Append( ": " ).Append( e.what( ) );
        return ImporterResultCode::WriteFailed;
    }
    return ImporterResultCode::Success;
}

Float_4x4 AssimpImporter::ConvertMatrix( const aiMatrix4x4 &matrix )
{
    Float_4x4 r;
    r._11 = matrix.a1;
    r._12 = matrix.a2;
    r._13 = matrix.a3;
    r._14 = matrix.a4;
    r._21 = matrix.b1;
    r._22 = matrix.b2;
    r._23 = matrix.b3;
    r._24 = matrix.b4;
    r._31 = matrix.c1;
    r._32 = matrix.c2;
    r._33 = matrix.c3;
    r._34 = matrix.c4;
    r._41 = matrix.d1;
    r._42 = matrix.d2;
    r._43 = matrix.d3;
    r._44 = matrix.d4;
    return r;
}
Float_4 AssimpImporter::ConvertQuaternion( const aiQuaternion &quat )
{
    return { quat.x, quat.y, quat.z, quat.w };
}
Float_3 AssimpImporter::ConvertVector3( const aiVector3D &vec )
{
    return { vec.x, vec.y, vec.z };
}
Float_2 AssimpImporter::ConvertVector2( const aiVector3D &vec )
{
    return { vec.x, vec.y };
}
Float_4 AssimpImporter::ConvertColor( const aiColor4D &color )
{
    return { color.r, color.g, color.b, color.a };
}

InteropString AssimpImporter::CreateAssetFileName( const InteropString &prefix, const InteropString &name, const InteropString &assetType, const InteropString &extension )
{
    auto fn = InteropString( prefix );
    if ( !prefix.IsEmpty( ) && !name.IsEmpty( ) )
    {
        fn = fn.Append( "_" );
    }
    fn = fn.Append( SanitizeAssetName( name ).Get( ) );
    if ( !assetType.IsEmpty( ) )
    {
        fn = fn.Append( "_" ).Append( assetType.Get( ) );
    }
    fn = fn.Append( "." ).Append( extension.Get( ) );
    return fn;
}

InteropString AssimpImporter::GetAssetNameFromFilePath( const InteropString &filePath )
{
    const std::filesystem::path p = filePath.Get( );
    return SanitizeAssetName( p.stem( ).string( ).c_str( ) );
}

InteropString AssimpImporter::SanitizeAssetName( const InteropString &name )
{
    std::string s = name.Get( );
    std::ranges::replace_if( s, []( char c ) { return !std::isalnum( c ) && c != '-' && c != '.'; }, '_' );
    s.erase( 0, std::min( s.find_first_not_of( "_-." ), s.size( ) - 1 ) );
    s.erase( s.find_last_not_of( "_-." ) + 1 );
    if ( s.empty( ) )
    {
        s = "UnnamedAsset";
    }
    return { s.c_str( ) };
}

InteropString AssimpImporter::GetFileExtension( const InteropString &filePath )
{
    const std::filesystem::path p = filePath.Get( );
    return InteropString( p.extension( ).string( ).c_str( ) ).ToLower( );
}

InteropString AssimpImporter::GetFileNameWithoutExtension( const InteropString &filePath )
{
    const std::filesystem::path p = filePath.Get( );
    return { p.stem( ).string( ).c_str( ) };
}

void AssimpImporter::RegisterCreatedAsset( ImportContext &context, const AssetUri &assetUri, AssetType assetType )
{
    context.Result.CreatedAssets.AddElement( assetUri );
}
