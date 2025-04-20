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

// ReSharper disable CppMemberFunctionMayBeStatic
#include <DenOfIzGraphics/Assets/Import/AssimpImporter.h>
#include <DenOfIzGraphics/Assets/Serde/Animation/AnimationAssetWriter.h>
#include <DenOfIzGraphics/Assets/Serde/Material/MaterialAssetWriter.h>
#include <DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetWriter.h>
#include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAssetWriter.h>
#include <DenOfIzGraphics/Assets/Serde/Texture/TextureAssetWriter.h>
#include <DenOfIzGraphics/Assets/Stream/BinaryWriter.h>
#include <DenOfIzGraphics/Data/Texture.h>
#include <DenOfIzGraphics/Utilities/Utilities.h>
#include <DirectXMath.h>
#include <algorithm>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <filesystem>
#include <limits>
#include <ranges>
#include <set>

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
    context.TargetDirectory = desc.TargetDirectory;
    context.AssetNamePrefix = desc.AssetNamePrefix;
    context.Options         = AssimpImportOptions::CreateFromBase( desc.Options );

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
    MeshAsset &meshAsset = context.MeshAsset;
    meshAsset.Name       = GetAssetNameFromFilePath( context.SourceFilePath );

    SkeletonAsset skeletonAsset;
    skeletonAsset.Name = meshAsset.Name;

    LOG( INFO ) << "Starting import for: " << meshAsset.Name.Get( );
    LOG( INFO ) << "Phase 1: Gathering bones and meshes...";

    for ( uint32_t i = 0; i < context.Scene->mNumMeshes; ++i )
    {
        if ( const aiMesh *mesh = context.Scene->mMeshes[ i ]; mesh && mesh->HasBones( ) )
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
        LOG( INFO ) << "Phase 2: Processing " << context.Scene->mNumMaterials << " materials...";
        for ( unsigned int i = 0; i < context.Scene->mNumMaterials; ++i )
        {
            ProcessMaterial( context, context.Scene->mMaterials[ i ] );
        }
    }

    if ( context.Options.ImportSkeletons && !context.BoneNameToIndexMap.empty( ) )
    {
        LOG( INFO ) << "Phase 3: Building skeleton hierarchy...";
        if ( const ImporterResultCode result = ProcessNode( context, context.Scene->mRootNode, nullptr, skeletonAsset ); result != ImporterResultCode::Success )
        {
            LOG( ERROR ) << "Failed to build skeleton hierarchy";
            return result;
        }

        if ( skeletonAsset.Joints.NumElements( ) > 0 )
        {
            WriteSkeletonAsset( context, skeletonAsset );
            meshAsset.SkeletonRef = context.SkeletonAssetUri;
        }
    }

    LOG( INFO ) << "Phase 4: Collecting meshes and preparing submesh metadata...";
    std::vector<const aiMesh *> uniqueMeshes;
    std::set<unsigned int>      processedMeshIndices;

    std::function<void( const aiNode * )> collectMeshes = [ & ]( const aiNode *node )
    {
        if ( !node )
        {
            return;
        }
        for ( unsigned int i = 0; i < node->mNumMeshes; ++i )
        {
            unsigned int meshIndex = node->mMeshes[ i ];
            if ( processedMeshIndices.contains( meshIndex ) )
            {
                continue;
            }

            if ( const aiMesh *mesh = context.Scene->mMeshes[ meshIndex ]; mesh && mesh->HasFaces( ) && mesh->HasPositions( ) )
            {
                uniqueMeshes.push_back( mesh );
                processedMeshIndices.insert( meshIndex );

                SubMeshData subMesh;
                subMesh.Name = mesh->mName.C_Str( );
                if ( subMesh.Name.IsEmpty( ) )
                {
                    subMesh.Name = InteropString( "SubMesh_" ).Append( std::to_string( meshAsset.SubMeshes.NumElements( ) ).c_str( ) );
                }

                subMesh.NumVertices = mesh->mNumVertices;
                subMesh.NumIndices  = mesh->mNumFaces * 3; // Todo support non triangulated faces
                subMesh.Topology    = PrimitiveTopology::Triangle;
                subMesh.IndexType   = IndexType::Uint32;

                Float_3 minBounds{ }, maxBounds{ };
                CalculateMeshBounds( mesh, context.Options.ScaleFactor, minBounds, maxBounds );
                subMesh.MinBounds = minBounds;
                subMesh.MaxBounds = maxBounds;
                subMesh.LODLevel  = 0;
                if ( context.Options.ImportMaterials && mesh->mMaterialIndex < context.Scene->mNumMaterials )
                {
                    const aiMaterial *material = context.Scene->mMaterials[ mesh->mMaterialIndex ];
                    if ( const std::string matNameStr = material->GetName( ).C_Str( ); context.MaterialNameToAssetUriMap.contains( matNameStr ) )
                    {
                        subMesh.MaterialRef = context.MaterialNameToAssetUriMap[ matNameStr ];
                    }
                }
                meshAsset.SubMeshes.AddElement( subMesh );
            }
        }

        for ( unsigned int i = 0; i < node->mNumChildren; ++i )
        {
            collectMeshes( node->mChildren[ i ] );
        }
    };

    collectMeshes( context.Scene->mRootNode );
    if ( !uniqueMeshes.empty( ) )
    {
        const aiMesh           *firstMesh = uniqueMeshes[ 0 ];
        VertexEnabledAttributes attributes{ };
        attributes.Position     = firstMesh->HasPositions( );
        attributes.Normal       = context.Options.GenerateNormals || firstMesh->HasNormals( );
        attributes.Tangent      = context.Options.CalculateTangentSpace || firstMesh->HasTangentsAndBitangents( );
        attributes.Bitangent    = context.Options.CalculateTangentSpace || firstMesh->HasTangentsAndBitangents( );
        attributes.UV           = firstMesh->GetNumUVChannels( ) > 0;
        attributes.Color        = firstMesh->HasVertexColors( 0 );
        attributes.BlendIndices = firstMesh->HasBones( );
        attributes.BlendWeights = firstMesh->HasBones( );

        VertexAttributeConfig attributeConfig{ };
        attributeConfig.NumPositionComponents = 4;
        attributeConfig.NumUVAttributes       = firstMesh->GetNumUVChannels( );
        attributeConfig.MaxBoneInfluences     = context.Options.MaxBoneWeightsPerVertex;

        attributeConfig.UVChannels.Resize( firstMesh->GetNumUVChannels( ) );
        for ( uint32_t i = 0; i < firstMesh->GetNumUVChannels( ); ++i )
        {
            UVChannel config;
            config.SemanticName = "TEXCOORD";
            config.Index        = i;
            attributeConfig.UVChannels.SetElement( i, config );
        }

        attributeConfig.ColorFormats.Resize( firstMesh->GetNumColorChannels( ) );
        for ( uint32_t i = 0; i < firstMesh->GetNumColorChannels( ); ++i )
        {
            attributeConfig.ColorFormats.SetElement( i, ColorFormat::RGBA );
        }

        meshAsset.EnabledAttributes = attributes;
        meshAsset.AttributeConfig   = attributeConfig;
        meshAsset.NumLODs           = 1; // TODO LODs not yet implemented
    }

    LOG( INFO ) << "Found " << uniqueMeshes.size( ) << " unique meshes";
    if ( context.Options.ImportAnimations && context.Scene->HasAnimations( ) )
    {
        LOG( INFO ) << "Phase 5: Processing " << context.Scene->mNumAnimations << " animations...";
        for ( unsigned int i = 0; i < context.Scene->mNumAnimations; ++i )
        {
            AssetUri animUri;
            ProcessAnimation( context, context.Scene->mAnimations[ i ], animUri );
            meshAsset.AnimationRefs.AddElement( animUri );
        }
    }

    if ( !uniqueMeshes.empty( ) )
    {
        LOG( INFO ) << "Phase 6: Writing mesh asset with " << meshAsset.SubMeshes.NumElements( ) << " submeshes...";
        const InteropString meshAssetFilename = CreateAssetFileName( context.AssetNamePrefix, meshAsset.Name, "Mesh", "dzmesh" );
        InteropString       meshTargetPath    = context.TargetDirectory.Append( "/" ).Append( meshAssetFilename.Get( ) );
        meshTargetPath                        = FileIO::GetAbsolutePath( meshTargetPath );

        const AssetUri meshUri = AssetUri::Create( meshAssetFilename );
        meshAsset.Uri          = meshUri;

        BinaryWriter    binaryWriter( meshTargetPath );
        MeshAssetWriter meshWriter( { &binaryWriter } );
        meshWriter.Write( meshAsset );
        context.CurrentSubMeshIndex = 0;

        for ( const aiMesh *mesh : uniqueMeshes )
        {
            if ( const ImporterResultCode result = ProcessMesh( context, mesh, meshWriter ); result != ImporterResultCode::Success )
            {
                LOG( ERROR ) << "Failed to process mesh #" << mesh->mName.C_Str( ) << ": " << context.ErrorMessage.Get( );
                return result;
            }
        }

        meshWriter.FinalizeAsset( );
        RegisterCreatedAsset( context, meshUri );
        LOG( INFO ) << "Successfully wrote Mesh asset: " << meshUri.ToString( ).Get( );
        if ( context.Options.ImportAnimations && context.Scene->HasAnimations( ) )
        {
            LOG( WARNING ) << "No processable meshes found in the scene";
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
    int32_t           currentJointIndex = parentJointIndex;

    if ( const bool isKnownBone = context.BoneNameToIndexMap.contains( nodeNameStr ); meshWriter == nullptr && isKnownBone && context.Options.ImportSkeletons )
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
            joint.Name        = nodeNameStr.c_str( );
            joint.Index       = currentJointIndex;
            joint.ParentIndex = parentJointIndex;

            aiMatrix4x4  localMatrix = node->mTransformation;
            aiVector3D   translation, scale;
            aiQuaternion rotation;
            localMatrix.Decompose( scale, rotation, translation );

            const float scaleFactor = context.Options.ScaleFactor;
            joint.LocalTranslation  = { translation.x * scaleFactor, translation.y * scaleFactor, translation.z * scaleFactor };
            joint.LocalRotationQuat = { rotation.x, rotation.y, rotation.z, rotation.w };
            joint.LocalScale        = { scale.x, scale.y, scale.z };

            if ( context.BoneNameToInverseBindMatrixMap.contains( nodeNameStr ) )
            {
                aiMatrix4x4 scaledMatrix = context.BoneNameToInverseBindMatrixMap[ nodeNameStr ];
                joint.InverseBindMatrix  = ConvertMatrix( scaledMatrix );
                joint.InverseBindMatrix._41 *= context.Options.ScaleFactor;
                joint.InverseBindMatrix._42 *= context.Options.ScaleFactor;
                joint.InverseBindMatrix._43 *= context.Options.ScaleFactor;
            }
            else
            {
                LOG( WARNING ) << "Inverse bind matrix not found in map for joint: " << joint.Name.Get( ) << ". Using identity."; /* Set identity */
            }
            skeletonAsset.Joints.AddElement( joint );
            if ( parentJointIndex >= 0 && parentJointIndex < static_cast<int32_t>( skeletonAsset.Joints.NumElements( ) ) )
            {
                skeletonAsset.Joints.GetElement( parentJointIndex ).ChildIndices.AddElement( currentJointIndex );
            }
            context.BoneNameToIndexMap[ nodeNameStr ]         = currentJointIndex;
            context.IndexToAssimpNodeMap[ currentJointIndex ] = node;
        }
        else
        {
            if ( context.BoneNameToIndexMap[ nodeNameStr ] == -1 )
            {
                context.BoneNameToIndexMap[ nodeNameStr ] = currentJointIndex;
            }
            if ( !context.IndexToAssimpNodeMap.contains( currentJointIndex ) )
            {
                context.IndexToAssimpNodeMap[ currentJointIndex ] = node;
            }
        }
    }
    else if ( meshWriter != nullptr && isKnownBone )
    {
        if ( context.BoneNameToIndexMap[ nodeNameStr ] != -1 )
        {
            currentJointIndex = context.BoneNameToIndexMap[ nodeNameStr ];
        }
        else
        {
            for ( size_t j = 0; j < skeletonAsset.Joints.NumElements( ); ++j )
            {
                if ( skeletonAsset.Joints.GetElement( j ).Name.Equals( nodeNameStr.c_str( ) ) )
                {
                    currentJointIndex                         = skeletonAsset.Joints.GetElement( j ).Index;
                    context.BoneNameToIndexMap[ nodeNameStr ] = currentJointIndex; // Update map
                    if ( !context.IndexToAssimpNodeMap.contains( currentJointIndex ) )
                    {
                        context.IndexToAssimpNodeMap[ currentJointIndex ] = node;
                    }
                    break;
                }
            }
        }
    }

    if ( meshWriter != nullptr )
    {
        for ( unsigned int i = 0; i < node->mNumMeshes; ++i )
        {
            if ( const aiMesh *mesh = context.Scene->mMeshes[ node->mMeshes[ i ] ] )
            {
                if ( const ImporterResultCode meshResult = ProcessMesh( context, mesh, *meshWriter ); meshResult != ImporterResultCode::Success )
                {
                    LOG( ERROR ) << "Failed writing data for mesh #" << node->mMeshes[ i ] << " attached to node " << nodeNameStr;
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

ImporterResultCode AssimpImporter::ProcessMesh( ImportContext &context, const aiMesh *mesh, MeshAssetWriter &assetWriter )
{
    if ( !mesh->HasFaces( ) || !mesh->HasPositions( ) )
    {
        return ImporterResultCode::Success;
    }

    const uint32_t submeshIndex = context.CurrentSubMeshIndex;
    if ( submeshIndex >= context.MeshAsset.SubMeshes.NumElements( ) )
    {
        LOG( ERROR ) << "ProcessMesh: Invalid submesh index " << submeshIndex;
        return ImporterResultCode::InvalidParameters;
    }
    LOG( INFO ) << "Writing data for mesh: " << mesh->mName.C_Str( ) << " (SubMesh: " << submeshIndex << " with " << mesh->mNumVertices << " vertices and " << mesh->mNumFaces * 3
                << " indices)";
    const VertexEnabledAttributes &attributes      = context.MeshAsset.EnabledAttributes;
    const VertexAttributeConfig   &attributeConfig = context.MeshAsset.AttributeConfig;

    std::vector<std::vector<std::pair<int, float>>> boneInfluences;
    if ( attributes.BlendIndices && mesh->HasBones( ) )
    {
        boneInfluences.resize( mesh->mNumVertices );
        for ( unsigned int b = 0; b < mesh->mNumBones; ++b )
        {
            const aiBone     *bone     = mesh->mBones[ b ];
            const std::string boneName = bone->mName.C_Str( );

            if ( !context.BoneNameToIndexMap.contains( boneName ) )
            {
                LOG( WARNING ) << "Bone '" << boneName << "' not found in skeleton - will be ignored";
                continue;
            }

            int boneIndex = context.BoneNameToIndexMap[ boneName ];
            if ( boneIndex < 0 )
            {
                LOG( WARNING ) << "Bone '" << boneName << "' has invalid index " << boneIndex;
                continue;
            }

            for ( unsigned int w = 0; w < bone->mNumWeights; ++w )
            {
                if ( const aiVertexWeight &weight = bone->mWeights[ w ]; weight.mVertexId < mesh->mNumVertices )
                {
                    boneInfluences[ weight.mVertexId ].emplace_back( boneIndex, weight.mWeight );
                }
            }
        }

        for ( auto &influences : boneInfluences )
        {
            std::ranges::sort( influences, []( const auto &a, const auto &b ) { return a.second > b.second; } );
            if ( influences.size( ) > attributeConfig.MaxBoneInfluences )
            {
                influences.resize( attributeConfig.MaxBoneInfluences );
            }

            float totalWeight = 0.0f;
            for ( const auto &weight : influences | std::views::values )
            {
                totalWeight += weight;
            }

            if ( totalWeight > 1e-6f )
            {
                for ( auto &weight : influences | std::views::values )
                {
                    weight /= totalWeight;
                }
            }
        }
    }

    for ( unsigned int i = 0; i < mesh->mNumVertices; ++i )
    {
        MeshVertex vertex{ };

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
        if ( attributes.Tangent && mesh->HasTangentsAndBitangents( ) )
        {
            vertex.Tangent = { mesh->mTangents[ i ].x, mesh->mTangents[ i ].y, mesh->mTangents[ i ].z, 1.0f };
        }
        if ( attributes.Bitangent && mesh->HasTangentsAndBitangents( ) )
        {
            vertex.Bitangent = { mesh->mBitangents[ i ].x, mesh->mBitangents[ i ].y, mesh->mBitangents[ i ].z, 1.0f };
        }

        vertex.UVs.Resize( attributeConfig.NumUVAttributes );
        for ( uint32_t uvChan = 0; uvChan < attributeConfig.NumUVAttributes; ++uvChan )
        {
            if ( mesh->HasTextureCoords( uvChan ) )
            {
                Float_2 uv                      = ConvertVector2( mesh->mTextureCoords[ uvChan ][ i ] );
                vertex.UVs.GetElement( uvChan ) = uv;
            }
            else
            {
                vertex.UVs.GetElement( uvChan ) = { 0.0f, 0.0f };
            }
        }

        vertex.Colors.Resize( attributeConfig.ColorFormats.NumElements( ) );
        for ( uint32_t colChan = 0; colChan < attributeConfig.ColorFormats.NumElements( ); ++colChan )
        {
            if ( mesh->HasVertexColors( colChan ) )
            {
                vertex.Colors.SetElement( colChan, ConvertColor( mesh->mColors[ colChan ][ i ] ) );
            }
            else
            {
                vertex.Colors.SetElement( colChan, { 1.0f, 1.0f, 1.0f, 1.0f } );
            }
        }

        if ( attributes.BlendIndices && !boneInfluences.empty( ) )
        {
            const auto &influences = i < boneInfluences.size( ) ? boneInfluences[ i ] : std::vector<std::pair<int, float>>( );
            vertex.BlendIndices    = { 0, 0, 0, 0 };
            vertex.BoneWeights     = { 0.0f, 0.0f, 0.0f, 0.0f };

            for ( size_t infIdx = 0; infIdx < influences.size( ) && infIdx < 4; ++infIdx )
            {
                const auto &[ boneIdx, weight ] = influences[ infIdx ];
                // ReSharper disable once CppDefaultCaseNotHandledInSwitchStatement, TODO needs modification after supporting more indexes
                switch ( infIdx )
                {
                case 0:
                    vertex.BlendIndices.X = boneIdx;
                    vertex.BoneWeights.X  = weight;
                    break;
                case 1:
                    vertex.BlendIndices.Y = boneIdx;
                    vertex.BoneWeights.Y  = weight;
                    break;
                case 2:
                    vertex.BlendIndices.Z = boneIdx;
                    vertex.BoneWeights.Z  = weight;
                    break;
                case 3:
                    vertex.BlendIndices.W = boneIdx;
                    vertex.BoneWeights.W  = weight;
                    break;
                }
            }
        }

        assetWriter.AddVertex( vertex );
    }

    for ( unsigned int i = 0; i < mesh->mNumFaces; ++i )
    {
        // Currently always triangulated due to aiProcess_Triangulate
        if ( const aiFace &face = mesh->mFaces[ i ]; face.mNumIndices == 3 )
        {
            assetWriter.AddIndex32( face.mIndices[ 0 ] );
            assetWriter.AddIndex32( face.mIndices[ 1 ] );
            assetWriter.AddIndex32( face.mIndices[ 2 ] );
        }
    }

    context.CurrentSubMeshIndex++;
    return ImporterResultCode::Success;
}

void AssimpImporter::ProcessMaterial( ImportContext &context, const aiMaterial *material )
{
    AssetUri          assetUri;
    const std::string matNameStr = material->GetName( ).C_Str( );
    InteropString     matName    = SanitizeAssetName( matNameStr.c_str( ) );
    if ( matName.IsEmpty( ) )
    {
        matName = InteropString( "Material_" ).Append( std::to_string( context.MaterialNameToAssetUriMap.size( ) ).c_str( ) );
    }
    if ( context.MaterialNameToAssetUriMap.contains( matNameStr ) )
    {
        return;
    }
    LOG( INFO ) << "Processing material: " << matName.Get( );
    MaterialAsset matAsset;
    matAsset.Name = matName;
    aiColor4D color;
    if ( material->Get( AI_MATKEY_COLOR_DIFFUSE, color ) == AI_SUCCESS )
    {
        matAsset.BaseColorFactor = ConvertColor( color );
    }
    if ( AssetUri albedoUri; context.Options.ImportTextures && ProcessTexture( context, material, aiTextureType_DIFFUSE, "Albedo", albedoUri ) )
    {
        matAsset.AlbedoMapRef = albedoUri;
    }
    if ( AssetUri normalUri; context.Options.ImportTextures && ( ProcessTexture( context, material, aiTextureType_NORMALS, "Normal", normalUri ) ||
                                                                 ProcessTexture( context, material, aiTextureType_HEIGHT, "Normal", normalUri ) ) )
    {
        matAsset.NormalMapRef = normalUri;
    }
    if ( AssetUri mrUri; context.Options.ImportTextures && ProcessTexture( context, material, aiTextureType_METALNESS, "MetallicRoughness", mrUri ) )
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
    if ( AssetUri emUri; context.Options.ImportTextures && ProcessTexture( context, material, aiTextureType_EMISSIVE, "Emissive", emUri ) )
    {
        matAsset.EmissiveMapRef = emUri;
    }
    if ( material->Get( AI_MATKEY_COLOR_EMISSIVE, color ) == AI_SUCCESS )
    {
        matAsset.EmissiveFactor = { color.r, color.g, color.b };
    }
    if ( AssetUri ocUri; context.Options.ImportTextures && ProcessTexture( context, material, aiTextureType_AMBIENT_OCCLUSION, "Occlusion", ocUri ) )
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
    WriteMaterialAsset( context, matAsset, assetUri );
}

bool AssimpImporter::ProcessTexture( ImportContext &context, const aiMaterial *material, const aiTextureType textureType, const InteropString &semanticName, AssetUri &outAssetUri )
{
    aiString aiPath;
    if ( material->GetTexture( textureType, 0, &aiPath ) != AI_SUCCESS || aiPath.length == 0 )
    {
        return false;
    }
    const std::string texPathStr = aiPath.C_Str( );
    if ( texPathStr[ 0 ] == '*' )
    {
        LOG( INFO ) << "Processing embedded texture for material '" << material->GetName( ).C_Str( ) << "', semantic: " << semanticName.Get( );
        if ( const int textureIndex = std::stoi( texPathStr.substr( 1 ) ); textureIndex >= 0 && textureIndex < static_cast<int>( context.Scene->mNumTextures ) )
        {
            WriteTextureAsset( context, context.Scene->mTextures[ textureIndex ], "", semanticName, outAssetUri );
            return true;
        }
        LOG( ERROR ) << "Invalid embedded texture index " << texPathStr.substr( 1 ) << " for material '" << material->GetName( ).C_Str( ) << "'.";
        return false;
    }
    LOG( INFO ) << "Processing external texture reference: '" << texPathStr << "' for semantic: " << semanticName.Get( );
    if ( context.TexturePathToAssetUriMap.contains( texPathStr ) )
    {
        outAssetUri = context.TexturePathToAssetUriMap[ texPathStr ];
        return false;
    }
    const std::filesystem::path modelPath           = context.SourceFilePath.Get( );
    const std::filesystem::path texturePath         = texPathStr;
    const std::filesystem::path absoluteTexturePath = texturePath.is_absolute( ) ? texturePath : absolute( modelPath.parent_path( ) / texturePath );
    if ( !exists( absoluteTexturePath ) )
    {
        LOG( ERROR ) << "External texture file not found: " << absoluteTexturePath.string( ) << " (referenced by material '" << material->GetName( ).C_Str( ) << "')";
        return false;
    }
    WriteTextureAsset( context, nullptr, absoluteTexturePath.string( ), semanticName, outAssetUri );
    return true;
}

void AssimpImporter::ProcessAnimation( ImportContext &context, const aiAnimation *animation, AssetUri &outAssetUri )
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

            key.Value.X *= context.Options.ScaleFactor;
            key.Value.Y *= context.Options.ScaleFactor;
            key.Value.Z *= context.Options.ScaleFactor;
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
    WriteAnimationAsset( context, animAsset, outAssetUri );
    LOG( INFO ) << "Successfully wrote animation asset: " << outAssetUri.ToString( ).Get( );
}

void AssimpImporter::CalculateMeshBounds( const aiMesh *mesh, const float scaleFactor, Float_3 &outMin, Float_3 &outMax )
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

void AssimpImporter::GenerateMeshLODs( const ImportContext &/*context*/, MeshAssetWriter &/*meshWriter*/ )
{
    // TODO Implement this with MeshOptimizer
    LOG( WARNING ) << "Not yet implemented";
}

void AssimpImporter::ConfigureAssimpImportFlags( const AssimpImportOptions &options, unsigned int &flags, Assimp::Importer &importer )
{
    flags |= aiProcess_ImproveCacheLocality;
    flags |= aiProcess_SortByPType;
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
    if ( options.ConvertToLeftHanded )
    {
        flags |= aiProcess_ConvertToLeftHanded;
    }
    if ( options.RemoveRedundantMaterials )
    {
        flags |= aiProcess_RemoveRedundantMaterials;
    }
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

void AssimpImporter::WriteMaterialAsset( ImportContext &context, const MaterialAsset &materialAsset, AssetUri &outAssetUri )
{
    const InteropString assetFilename   = CreateAssetFileName( context.AssetNamePrefix, materialAsset.Name, "Material", "dzmat" );
    const InteropString targetAssetPath = FileIO::GetAbsolutePath( InteropString( context.TargetDirectory ).Append( "/" ).Append( assetFilename.Get( ) ) );
    outAssetUri                         = AssetUri::Create( assetFilename );
    MaterialAsset mutableAsset          = materialAsset;
    mutableAsset.Uri                    = outAssetUri;
    LOG( INFO ) << "Writing Material asset to: " << targetAssetPath.Get( );
    BinaryWriter              writer( targetAssetPath );
    const MaterialAssetWriter assetWriter( { &writer } );
    assetWriter.Write( mutableAsset );
    RegisterCreatedAsset( context, outAssetUri );
}

TextureExtension GetTextureExtension( const aiTexture *texture, const InteropArray<Byte> &textureData )
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
        texExtension = Texture::IdentifyTextureFormat( textureData );
    }
    return texExtension;
}

void AssimpImporter::WriteTextureAsset( ImportContext &context, const aiTexture *texture, const std::string &path, const InteropString &semanticName, AssetUri &outAssetUri )
{
    InteropString texName;
    if ( texture != nullptr )
    {
        texName = SanitizeAssetName( texture->mFilename.length > 0 ? texture->mFilename.C_Str( ) : semanticName.Get( ) );
        if ( texName.IsEmpty( ) )
        {
            texName = InteropString( semanticName ).Append( "_Tex_" ).Append( std::to_string( context.Result.CreatedAssets.NumElements( ) ).c_str( ) );
        }
    }
    else
    {
        const std::filesystem::path texPath = path;
        texName                             = SanitizeAssetName( texPath.filename( ).stem( ).string( ).c_str( ) );
    }
    const InteropString assetFilename   = CreateAssetFileName( context.AssetNamePrefix, texName, "Texture", "dztex" );
    const InteropString targetAssetPath = FileIO::GetAbsolutePath( InteropString( context.TargetDirectory ).Append( "/" ).Append( assetFilename.Get( ) ) );

    BinaryWriter       writer( targetAssetPath );
    TextureAssetWriter assetWriter( { &writer } );

    outAssetUri = AssetUri::Create( assetFilename );
    LOG( INFO ) << "Writing embedded Texture asset to: " << targetAssetPath.Get( ) << " (Semantic: " << semanticName.Get( ) << ")";
    TextureAsset texAsset;
    texAsset.Name = texName;
    texAsset.Uri  = outAssetUri;

    std::unique_ptr<Texture> sourceTexture = nullptr;
    if ( texture )
    {
        const bool isCompressed = texture->mHeight == 0;

        InteropArray<Byte> textureData( texture->mWidth );
        size_t             numBytes = isCompressed ? texture->mWidth : static_cast<size_t>( texAsset.SlicePitch );
        textureData.MemCpy( texture->pcData, numBytes );

        sourceTexture = std::make_unique<Texture>( textureData, GetTextureExtension( texture, textureData ) );
    }
    else
    {
        sourceTexture = std::make_unique<Texture>( path );
    }

    texAsset.Width        = sourceTexture->Width;
    texAsset.Height       = sourceTexture->Height;
    texAsset.Depth        = sourceTexture->Depth;
    texAsset.Format       = sourceTexture->Format;
    texAsset.Dimension    = sourceTexture->Dimension;
    texAsset.MipLevels    = sourceTexture->MipLevels;
    texAsset.ArraySize    = sourceTexture->ArraySize;
    texAsset.BitsPerPixel = sourceTexture->BitsPerPixel;
    texAsset.BlockSize    = sourceTexture->BlockSize;
    texAsset.RowPitch     = sourceTexture->RowPitch;
    texAsset.NumRows      = sourceTexture->NumRows;
    texAsset.SlicePitch   = sourceTexture->SlicePitch;
    texAsset.Mips.Resize( sourceTexture->MipLevels * sourceTexture->ArraySize );

    size_t mipIndex = 0;
    // Have to double stream to write metadata correctly first
    sourceTexture->StreamMipData( [ & ]( const TextureMip &mipData ) { texAsset.Mips.SetElement( mipIndex++, mipData ); } );
    assetWriter.Write( texAsset );

    sourceTexture->StreamMipData(
        [ & ]( const TextureMip &mipData )
        {
            const size_t mipSize   = mipData.SlicePitch;
            const size_t mipOffset = mipData.DataOffset;

            InteropArray<Byte> mipDataBuffer;
            mipDataBuffer.MemCpy( sourceTexture->Data.data( ) + mipOffset, mipSize );
            assetWriter.AddPixelData( mipDataBuffer, mipData.MipIndex, mipData.ArrayIndex );
        } );

    assetWriter.Finalize( );
    RegisterCreatedAsset( context, outAssetUri );
    context.TexturePathToAssetUriMap[ targetAssetPath.Get( ) ] = outAssetUri;
}

void AssimpImporter::WriteSkeletonAsset( ImportContext &context, const SkeletonAsset &skeletonAsset )
{
    const InteropString assetFilename   = CreateAssetFileName( context.AssetNamePrefix, skeletonAsset.Name, "Skeleton", "dzskel" );
    const InteropString targetAssetPath = FileIO::GetAbsolutePath( InteropString( context.TargetDirectory ).Append( "/" ).Append( assetFilename.Get( ) ) );
    context.SkeletonAssetUri            = AssetUri::Create( assetFilename );
    SkeletonAsset mutableAsset          = skeletonAsset;
    mutableAsset.Uri                    = context.SkeletonAssetUri;
    LOG( INFO ) << "Writing Skeleton asset to: " << targetAssetPath.Get( );
    BinaryWriter              writer( targetAssetPath );
    const SkeletonAssetWriter assetWriter( { &writer } );
    assetWriter.Write( mutableAsset );
    RegisterCreatedAsset( context, context.SkeletonAssetUri );
}

void AssimpImporter::WriteAnimationAsset( ImportContext &context, const AnimationAsset &animationAsset, AssetUri &outAssetUri )
{
    const InteropString assetFilename   = CreateAssetFileName( context.AssetNamePrefix, animationAsset.Name, "Animation", "dzanim" );
    const InteropString targetAssetPath = FileIO::GetAbsolutePath( InteropString( context.TargetDirectory ).Append( "/" ).Append( assetFilename.Get( ) ) );
    outAssetUri                         = AssetUri::Create( assetFilename );
    AnimationAsset mutableAsset         = animationAsset;
    mutableAsset.Uri                    = outAssetUri;
    LOG( INFO ) << "Writing Animation asset to: " << targetAssetPath.Get( );
    BinaryWriter         writer( targetAssetPath );
    AnimationAssetWriter assetWriter( { &writer } );
    assetWriter.Write( mutableAsset );
    RegisterCreatedAsset( context, outAssetUri );
}

Float_4x4 AssimpImporter::ConvertMatrix( const aiMatrix4x4 &matrix )
{
    // Both of these are row major, but their layouts don't quite match, Float_4X4 is analogous to XMFLOAT4X4 and 1=1 layout mapping returns an invalid matrix
    // Decomposing and recomposing seem like the safest bet
    aiVector3f   translation;
    aiQuaternion rotation;
    aiVector3f   scale;
    matrix.Decompose( scale, rotation, translation );

    // clang-format off
    const DirectX::XMMATRIX matrixXM = DirectX::XMMatrixAffineTransformation(
        DirectX::XMVectorSet( scale.x, scale.y, scale.z, 1.0f ),
        DirectX::XMVectorZero(),
        DirectX::XMVectorSet( rotation.x, rotation.y, rotation.z, rotation.w ),
        DirectX::XMVectorSet( translation.x, translation.y, translation.z, 1.0f )
    );
    // clang-format on

    return Float_4X4FromXMMATRIX( matrixXM );
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
    std::ranges::replace_if( s, []( const char c ) { return !std::isalnum( c ) && c != '-' && c != '.'; }, '_' );
    s.erase( 0, std::min( s.find_first_not_of( "_-." ), s.size( ) - 1 ) );
    s.erase( s.find_last_not_of( "_-." ) + 1 );
    if ( s.empty( ) )
    {
        s = "UnnamedAsset";
    }
    return { s.c_str( ) };
}

InteropString AssimpImporter::GetFileExtension( const InteropString &filePath ) const
{
    const std::filesystem::path p = filePath.Get( );
    return InteropString( p.extension( ).string( ).c_str( ) ).ToLower( );
}

InteropString AssimpImporter::GetFileNameWithoutExtension( const InteropString &filePath )
{
    const std::filesystem::path p = filePath.Get( );
    return { p.stem( ).string( ).c_str( ) };
}

void AssimpImporter::RegisterCreatedAsset( ImportContext &context, const AssetUri &assetUri )
{
    context.Result.CreatedAssets.AddElement( assetUri );
}
