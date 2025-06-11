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

#include <assimp/Importer.hpp>
#include <assimp/material.h>
#include <assimp/scene.h>
#include <functional>
#include <typeinfo>
#include <unordered_map>

// ReSharper disable CppMemberFunctionMayBeStatic
#include <algorithm>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>
#include <filesystem>
#include <limits>
#include <ranges>
#include <set>
#include "DenOfIzGraphics/Assets/Bundle/BundleManager.h"
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphics/Assets/Import/AssetPathUtilities.h"
#include "DenOfIzGraphics/Assets/Import/AssimpImporter.h"
#include "DenOfIzGraphics/Assets/Serde/Animation/AnimationAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Animation/AnimationAssetWriter.h"
#include "DenOfIzGraphics/Assets/Serde/Material/MaterialAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Material/MaterialAssetWriter.h"
#include "DenOfIzGraphics/Assets/Serde/Mesh/MeshAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetWriter.h"
#include "DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAssetWriter.h"
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAssetWriter.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"
#include "DenOfIzGraphics/Data/Texture.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"
#include "DenOfIzGraphicsInternal/Utilities/InteropMathConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

// Include last due to DirectXMath breaking NULL on external libraries
#include <DirectXMath.h>

using namespace DenOfIz;

class AssimpImporter::Impl
{
public:
    ImporterDesc             m_importerInfo;
    const AssimpImporterDesc m_desc;

    struct ImportContext
    {
        const aiScene   *Scene = nullptr;
        InteropString    SourceFilePath;
        InteropString    TargetDirectory;
        InteropString    AssetNamePrefix;
        AssimpImportDesc Desc;
        ImporterResult   Result;
        InteropString    ErrorMessage;

        std::unordered_map<std::string, AssetUri>    MaterialNameToAssetUriMap;
        std::unordered_map<std::string, AssetUri>    TexturePathToAssetUriMap;
        std::unordered_map<std::string, uint32_t>    BoneNameToIndexMap;
        std::unordered_map<std::string, aiMatrix4x4> BoneNameToInverseBindMatrixMap;
        std::map<int32_t, const aiNode *>            IndexToAssimpNodeMap;
        std::map<const aiNode *, aiMatrix4x4>        WorldTransformCache;
        AssetUri                                     SkeletonAssetUri;
        uint32_t                                     CurrentSubMeshIndex = 0;
        MeshAsset                                    MeshAsset{ };
    };

    explicit Impl( AssimpImporterDesc desc ) : m_desc( std::move( desc ) )
    {
        m_importerInfo.Name                               = "Assimp Importer";
        m_importerInfo.SupportedExtensions                = InteropStringArray::Create( 20 );
        m_importerInfo.SupportedExtensions.Elements[ 0 ]  = ".fbx";
        m_importerInfo.SupportedExtensions.Elements[ 1 ]  = ".gltf";
        m_importerInfo.SupportedExtensions.Elements[ 2 ]  = ".glb";
        m_importerInfo.SupportedExtensions.Elements[ 3 ]  = ".obj";
        m_importerInfo.SupportedExtensions.Elements[ 4 ]  = ".dae";
        m_importerInfo.SupportedExtensions.Elements[ 5 ]  = ".blend";
        m_importerInfo.SupportedExtensions.Elements[ 6 ]  = ".3ds";
        m_importerInfo.SupportedExtensions.Elements[ 7 ]  = ".ase";
        m_importerInfo.SupportedExtensions.Elements[ 8 ]  = ".ifc";
        m_importerInfo.SupportedExtensions.Elements[ 9 ]  = ".xgl";
        m_importerInfo.SupportedExtensions.Elements[ 10 ] = ".zgl";
        m_importerInfo.SupportedExtensions.Elements[ 11 ] = ".ply";
        m_importerInfo.SupportedExtensions.Elements[ 12 ] = ".dxf";
        m_importerInfo.SupportedExtensions.Elements[ 13 ] = ".lwo";
        m_importerInfo.SupportedExtensions.Elements[ 14 ] = ".lws";
        m_importerInfo.SupportedExtensions.Elements[ 15 ] = ".lxo";
        m_importerInfo.SupportedExtensions.Elements[ 16 ] = ".stl";
        m_importerInfo.SupportedExtensions.Elements[ 17 ] = ".x";
        m_importerInfo.SupportedExtensions.Elements[ 18 ] = ".ac";
        m_importerInfo.SupportedExtensions.Elements[ 19 ] = ".ms3d";
    }

    ~Impl( )
    {
        m_importerInfo.SupportedExtensions.Dispose( );
    }

    ImporterResultCode ImportSceneInternal( ImportContext &context );
    ImporterResultCode ProcessNode( ImportContext &context, const aiNode *node, MeshAssetWriter *meshWriter, SkeletonAsset &skeletonAsset, int32_t parentJointIndex = -1 );
    ImporterResultCode ProcessMesh( ImportContext &context, const aiMesh *mesh, MeshAssetWriter &assetWriter ) const;
    void               ProcessMaterial( ImportContext &context, const aiMaterial *material ) const;
    bool ProcessTexture( ImportContext &context, const aiMaterial *material, aiTextureType textureType, const InteropString &semanticName, AssetUri &outAssetUri ) const;
    void ProcessAnimation( ImportContext &context, const aiAnimation *animation, AssetUri &outAssetUri ) const;

    void ConfigureAssimpImportFlags( const AssimpImportDesc &options, unsigned int &flags, Assimp::Importer &importer ) const;
    void CalculateMeshBounds( const aiMesh *mesh, float scaleFactor, Float_3 &outMin, Float_3 &outMax ) const;

    void WriteMaterialAsset( ImportContext &context, const MaterialAsset &materialAsset, AssetUri &outAssetUri ) const;
    void WriteTextureAsset( ImportContext &context, const aiTexture *texture, const std::string &path, const InteropString &semanticName, AssetUri &outAssetUri ) const;
    void WriteSkeletonAsset( ImportContext &context, const SkeletonAsset &skeletonAsset ) const;
    void WriteAnimationAsset( ImportContext &context, const AnimationAsset &animationAsset, AssetUri &outAssetUri ) const;

    Float_4x4 ConvertMatrix( const aiMatrix4x4 &matrix ) const;
    Float_4   ConvertQuaternion( const aiQuaternion &quat ) const;
    Float_3   ConvertVector3( const aiVector3D &vec ) const;
    Float_2   ConvertVector2( const aiVector3D &vec ) const;
    Float_4   ConvertColor( const aiColor4D &color ) const;

    void RegisterCreatedAsset( ImportContext &context, const AssetUri &assetUri ) const;
    void GenerateMeshLODs( const ImportContext &context, MeshAssetWriter &meshWriter ) const;
};

AssimpImporter::AssimpImporter( AssimpImporterDesc desc ) : m_pImpl( std::make_unique<Impl>( std::move( desc ) ) )
{
}

AssimpImporter::~AssimpImporter( ) = default;

ImporterDesc AssimpImporter::GetImporterInfo( ) const
{
    return m_pImpl->m_importerInfo;
}

bool AssimpImporter::CanProcessFileExtension( const InteropString &extension ) const
{
    const InteropString lowerExt = extension.ToLower( );
    for ( size_t i = 0; i < m_pImpl->m_importerInfo.SupportedExtensions.NumElements; ++i )
    {
        if ( m_pImpl->m_importerInfo.SupportedExtensions.Elements[ i ].Equals( lowerExt ) )
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
    return aiIsExtensionSupported( AssetPathUtilities::GetFileExtension( filePath ).Get( ) );
}

ImporterResult AssimpImporter::Import( const ImportJobDesc &desc )
{
    spdlog::info( "Starting Assimp import for file: {}", desc.SourceFilePath.Get( ) );

    Impl::ImportContext context;
    context.SourceFilePath  = desc.SourceFilePath;
    context.TargetDirectory = desc.TargetDirectory;
    context.AssetNamePrefix = desc.AssetNamePrefix;
    context.Desc            = *static_cast<AssimpImportDesc *>( desc.Desc );

    if ( !FileIO::FileExists( context.SourceFilePath ) )
    {
        context.Result.ResultCode   = ImporterResultCode::FileNotFound;
        context.Result.ErrorMessage = InteropString( "Source file not found: " ).Append( context.SourceFilePath.Get( ) );
        spdlog::error( "{}", context.Result.ErrorMessage.Get( ) );
        return context.Result;
    }

    if ( !FileIO::FileExists( context.TargetDirectory ) )
    {
        spdlog::info( "Target directory does not exist, attempting to create: {}", context.TargetDirectory.Get( ) );
        if ( !FileIO::CreateDirectories( context.TargetDirectory ) )
        {
            context.Result.ResultCode   = ImporterResultCode::WriteFailed;
            context.Result.ErrorMessage = InteropString( "Failed to create target directory: " ).Append( context.TargetDirectory.Get( ) );
            spdlog::error( "{}", context.Result.ErrorMessage.Get( ) );
            return context.Result;
        }
    }

    Assimp::Importer importer;
    unsigned int     flags = 0;
    m_pImpl->ConfigureAssimpImportFlags( context.Desc, flags, importer );

    spdlog::info( "Assimp reading file: {}", context.SourceFilePath.Get( ) );
    context.Scene = importer.ReadFile( FileIO::GetResourcePath( context.SourceFilePath ).Get( ), flags );

    if ( !context.Scene || context.Scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !context.Scene->mRootNode )
    {
        context.Result.ResultCode   = ImporterResultCode::ImportFailed;
        context.Result.ErrorMessage = InteropString( "Assimp importer failed: " ).Append( importer.GetErrorString( ) );
        spdlog::error( "{}", context.Result.ErrorMessage.Get( ) );
        return context.Result;
    }

    context.Result.ResultCode = m_pImpl->ImportSceneInternal( context );

    if ( context.Result.ResultCode == ImporterResultCode::Success )
    {
        spdlog::info( "Assimp import successful for: {}", context.SourceFilePath.Get( ) );
    }
    else
    {
        spdlog::error( "Assimp import failed for: {} Error: {}", context.SourceFilePath.Get( ), context.Result.ErrorMessage.Get( ) );
    }

    return context.Result;
}

ImporterResultCode AssimpImporter::Impl::ImportSceneInternal( ImportContext &context )
{
    MeshAsset &meshAsset = context.MeshAsset;
    meshAsset.Name       = AssetPathUtilities::GetAssetNameFromFilePath( context.SourceFilePath );

    SkeletonAsset skeletonAsset;
    skeletonAsset.Name = meshAsset.Name;

    spdlog::info( "Starting import for: {}", meshAsset.Name.Get( ) );
    spdlog::info( "Phase 1: Gathering bones and meshes..." );

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

    if ( context.Desc.ImportMaterials && context.Scene->HasMaterials( ) )
    {
        spdlog::info( "Phase 2: Processing {} materials...", context.Scene->mNumMaterials );
        for ( unsigned int i = 0; i < context.Scene->mNumMaterials; ++i )
        {
            ProcessMaterial( context, context.Scene->mMaterials[ i ] );
        }
    }

    if ( context.Desc.ImportSkeletons && !context.BoneNameToIndexMap.empty( ) )
    {
        spdlog::info( "Phase 3: Building skeleton hierarchy..." );
        if ( const ImporterResultCode result = ProcessNode( context, context.Scene->mRootNode, nullptr, skeletonAsset ); result != ImporterResultCode::Success )
        {
            spdlog::error( "Failed to build skeleton hierarchy" );
            return result;
        }

        if ( skeletonAsset.Joints.NumElements > 0 )
        {
            WriteSkeletonAsset( context, skeletonAsset );
            meshAsset.SkeletonRef = context.SkeletonAssetUri;
        }
    }

    spdlog::info( "Phase 4: Collecting meshes and preparing submesh metadata..." );
    std::vector<const aiMesh *> uniqueMeshes;
    std::set<unsigned int>      processedMeshIndices;
    std::vector<SubMeshData>    collectedSubMeshes;

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
                    subMesh.Name = InteropString( "SubMesh_" ).Append( std::to_string( collectedSubMeshes.size( ) ).c_str( ) );
                }

                subMesh.NumVertices = mesh->mNumVertices;
                subMesh.NumIndices  = mesh->mNumFaces * 3; // Todo support non triangulated faces
                subMesh.Topology    = PrimitiveTopology::Triangle;
                subMesh.IndexType   = IndexType::Uint32;

                Float_3 minBounds{ }, maxBounds{ };
                CalculateMeshBounds( mesh, context.Desc.ScaleFactor, minBounds, maxBounds );
                subMesh.MinBounds = minBounds;
                subMesh.MaxBounds = maxBounds;
                subMesh.LODLevel  = 0;
                if ( context.Desc.ImportMaterials && mesh->mMaterialIndex < context.Scene->mNumMaterials )
                {
                    const aiMaterial *material = context.Scene->mMaterials[ mesh->mMaterialIndex ];
                    if ( const std::string matNameStr = material->GetName( ).C_Str( ); context.MaterialNameToAssetUriMap.contains( matNameStr ) )
                    {
                        subMesh.MaterialRef = context.MaterialNameToAssetUriMap[ matNameStr ];
                    }
                }
                collectedSubMeshes.push_back( subMesh );
            }
        }

        for ( unsigned int i = 0; i < node->mNumChildren; ++i )
        {
            collectMeshes( node->mChildren[ i ] );
        }
    };

    collectMeshes( context.Scene->mRootNode );
    if ( !collectedSubMeshes.empty( ) )
    {
        meshAsset.SubMeshes = SubMeshDataArray::Create( static_cast<uint32_t>( collectedSubMeshes.size( ) ) );
        for ( size_t i = 0; i < collectedSubMeshes.size( ); ++i )
        {
            meshAsset.SubMeshes.Elements[ i ] = collectedSubMeshes[ i ];
        }
    }

    if ( !uniqueMeshes.empty( ) )
    {
        const aiMesh           *firstMesh = uniqueMeshes[ 0 ];
        VertexEnabledAttributes attributes{ };
        attributes.Position     = firstMesh->HasPositions( );
        attributes.Normal       = context.Desc.GenerateNormals || firstMesh->HasNormals( );
        attributes.Tangent      = context.Desc.CalculateTangentSpace || firstMesh->HasTangentsAndBitangents( );
        attributes.Bitangent    = context.Desc.CalculateTangentSpace || firstMesh->HasTangentsAndBitangents( );
        attributes.UV           = firstMesh->GetNumUVChannels( ) > 0;
        attributes.Color        = firstMesh->HasVertexColors( 0 );
        attributes.BlendIndices = firstMesh->HasBones( );
        attributes.BlendWeights = firstMesh->HasBones( );

        VertexAttributeConfig attributeConfig{ };
        attributeConfig.NumPositionComponents = 4;
        attributeConfig.NumUVAttributes       = firstMesh->GetNumUVChannels( );
        attributeConfig.MaxBoneInfluences     = context.Desc.MaxBoneWeightsPerVertex;

        attributeConfig.UVChannels = UVChannelArray::Create( firstMesh->GetNumUVChannels( ) );
        for ( uint32_t i = 0; i < firstMesh->GetNumUVChannels( ); ++i )
        {
            UVChannel config;
            config.SemanticName                      = "TEXCOORD";
            config.Index                             = i;
            attributeConfig.UVChannels.Elements[ i ] = config;
        }

        attributeConfig.ColorFormats = ColorFormatArray::Create( firstMesh->GetNumColorChannels( ) );
        for ( uint32_t i = 0; i < firstMesh->GetNumColorChannels( ); ++i )
        {
            attributeConfig.ColorFormats.Elements[ i ] = ColorFormat::RGBA;
        }

        meshAsset.EnabledAttributes = attributes;
        meshAsset.AttributeConfig   = attributeConfig;
        meshAsset.NumLODs           = 1; // TODO LODs not yet implemented
    }

    spdlog::info( "Found {} unique meshes", uniqueMeshes.size( ) );
    if ( context.Desc.ImportAnimations && context.Scene->HasAnimations( ) )
    {
        spdlog::info( "Phase 5: Processing {} animations...", context.Scene->mNumAnimations );
        meshAsset.AnimationRefs = AssetUriArray::Create( context.Scene->mNumAnimations );
        for ( unsigned int i = 0; i < context.Scene->mNumAnimations; ++i )
        {
            AssetUri animUri;
            ProcessAnimation( context, context.Scene->mAnimations[ i ], animUri );
            meshAsset.AnimationRefs.Elements[ i ] = animUri;
        }
    }

    if ( !uniqueMeshes.empty( ) )
    {
        spdlog::info( "Phase 6: Writing mesh asset with {} submeshes...", meshAsset.SubMeshes.NumElements );
        const InteropString meshAssetFilename = AssetPathUtilities::CreateAssetFileName( context.AssetNamePrefix, meshAsset.Name, "Mesh", MeshAsset::Extension( ) );
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
                spdlog::error( "Failed to process mesh # {} : {}", mesh->mName.C_Str( ), context.ErrorMessage.Get( ) );
                return result;
            }
        }

        meshWriter.FinalizeAsset( );
        RegisterCreatedAsset( context, meshUri );
        meshAsset.Dispose( );
        spdlog::info( "Successfully wrote Mesh asset: {}", meshUri.ToInteropString( ).Get( ) );
        if ( context.Desc.ImportAnimations && context.Scene->HasAnimations( ) )
        {
            spdlog::warn( "No processable meshes found in the scene" );
        }
    }

    return ImporterResultCode::Success;
}

ImporterResultCode AssimpImporter::Impl::ProcessNode( ImportContext &context, const aiNode *node, MeshAssetWriter *meshWriter, SkeletonAsset &skeletonAsset,
                                                      int32_t parentJointIndex )
{
    if ( !node )
    {
        return ImporterResultCode::Success;
    }
    const std::string nodeNameStr       = node->mName.C_Str( );
    int32_t           currentJointIndex = parentJointIndex;

    if ( const bool isKnownBone = context.BoneNameToIndexMap.contains( nodeNameStr ); meshWriter == nullptr && isKnownBone && context.Desc.ImportSkeletons )
    {
        bool alreadyAdded = false;
        for ( size_t j = 0; j < skeletonAsset.Joints.NumElements; ++j )
        {
            if ( skeletonAsset.Joints.Elements[ j ].Name.Equals( nodeNameStr.c_str( ) ) )
            {
                currentJointIndex = skeletonAsset.Joints.Elements[ j ].Index;
                alreadyAdded      = true;
                break;
            }
        }

        if ( !alreadyAdded )
        {
            currentJointIndex = skeletonAsset.Joints.NumElements;
            Joint joint;
            joint.Name        = nodeNameStr.c_str( );
            joint.Index       = currentJointIndex;
            joint.ParentIndex = parentJointIndex;

            aiMatrix4x4  localMatrix = node->mTransformation;
            aiVector3D   translation, scale;
            aiQuaternion rotation;
            localMatrix.Decompose( scale, rotation, translation );

            const float scaleFactor = context.Desc.ScaleFactor;
            joint.LocalTranslation  = { translation.x * scaleFactor, translation.y * scaleFactor, translation.z * scaleFactor };
            joint.LocalRotationQuat = { rotation.x, rotation.y, rotation.z, rotation.w };
            joint.LocalScale        = { scale.x, scale.y, scale.z };

            if ( context.BoneNameToInverseBindMatrixMap.contains( nodeNameStr ) )
            {
                aiMatrix4x4 scaledMatrix = context.BoneNameToInverseBindMatrixMap[ nodeNameStr ];
                joint.InverseBindMatrix  = ConvertMatrix( scaledMatrix );
                joint.InverseBindMatrix._41 *= context.Desc.ScaleFactor;
                joint.InverseBindMatrix._42 *= context.Desc.ScaleFactor;
                joint.InverseBindMatrix._43 *= context.Desc.ScaleFactor;
            }
            else
            {
                spdlog::warn( "Inverse bind matrix not found in map for joint: {} . Using identity.", joint.Name.Get( ) ); /* Set identity */
            }
            // TODO: Cleaner growing mechanism
            JointArray newJoints = JointArray::Create( skeletonAsset.Joints.NumElements + 1 );
            for ( size_t i = 0; i < skeletonAsset.Joints.NumElements; ++i )
            {
                newJoints.Elements[ i ] = skeletonAsset.Joints.Elements[ i ];
            }
            newJoints.Elements[ skeletonAsset.Joints.NumElements ] = joint;
            skeletonAsset.Joints.Dispose( );
            skeletonAsset.Joints = newJoints;

            if ( parentJointIndex >= 0 && parentJointIndex < static_cast<int32_t>( skeletonAsset.Joints.NumElements ) )
            {
                // TODO: Cleaner growing mechanism
                UInt32Array &parentChildIndices = skeletonAsset.Joints.Elements[ parentJointIndex ].ChildIndices;
                UInt32Array  newChildIndices    = UInt32Array::Create( parentChildIndices.NumElements + 1 );
                for ( size_t i = 0; i < parentChildIndices.NumElements; ++i )
                {
                    newChildIndices.Elements[ i ] = parentChildIndices.Elements[ i ];
                }
                newChildIndices.Elements[ parentChildIndices.NumElements ] = currentJointIndex;
                parentChildIndices.Dispose( );
                parentChildIndices = newChildIndices;
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
            for ( size_t j = 0; j < skeletonAsset.Joints.NumElements; ++j )
            {
                if ( skeletonAsset.Joints.Elements[ j ].Name.Equals( nodeNameStr.c_str( ) ) )
                {
                    currentJointIndex                         = skeletonAsset.Joints.Elements[ j ].Index;
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
                    spdlog::error( "Failed writing data for mesh # {} attached to node {}", node->mMeshes[ i ], nodeNameStr );
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

ImporterResultCode AssimpImporter::Impl::ProcessMesh( ImportContext &context, const aiMesh *mesh, MeshAssetWriter &assetWriter ) const
{
    if ( !mesh->HasFaces( ) || !mesh->HasPositions( ) )
    {
        return ImporterResultCode::Success;
    }

    const uint32_t submeshIndex = context.CurrentSubMeshIndex;
    if ( submeshIndex >= context.MeshAsset.SubMeshes.NumElements )
    {
        spdlog::error( "ProcessMesh: Invalid submesh index {}", submeshIndex );
        return ImporterResultCode::InvalidParameters;
    }
    spdlog::info( "Writing data for mesh: {} (SubMesh: {} with {} vertices and {} indices)", mesh->mName.C_Str( ), submeshIndex, mesh->mNumVertices, mesh->mNumFaces * 3 );
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
                spdlog::warn( "Bone ' {} ' not found in skeleton - will be ignored", boneName );
                continue;
            }

            int boneIndex = context.BoneNameToIndexMap[ boneName ];
            if ( boneIndex < 0 )
            {
                spdlog::warn( "Bone ' {} ' has invalid index {}", boneName, boneIndex );
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
            if ( context.Desc.ScaleFactor != 1.0f )
            {
                vertex.Position.X *= context.Desc.ScaleFactor;
                vertex.Position.Y *= context.Desc.ScaleFactor;
                vertex.Position.Z *= context.Desc.ScaleFactor;
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

        vertex.UVs = Float_2Array::Create( attributeConfig.NumUVAttributes );
        for ( uint32_t uvChan = 0; uvChan < attributeConfig.NumUVAttributes; ++uvChan )
        {
            if ( mesh->HasTextureCoords( uvChan ) )
            {
                Float_2 uv                    = ConvertVector2( mesh->mTextureCoords[ uvChan ][ i ] );
                vertex.UVs.Elements[ uvChan ] = uv;
            }
            else
            {
                vertex.UVs.Elements[ uvChan ] = { 0.0f, 0.0f };
            }
        }

        vertex.Colors = Float_4Array::Create( attributeConfig.ColorFormats.NumElements );
        for ( uint32_t colChan = 0; colChan < attributeConfig.ColorFormats.NumElements; ++colChan )
        {
            if ( mesh->HasVertexColors( colChan ) )
            {
                vertex.Colors.Elements[ colChan ] = ConvertColor( mesh->mColors[ colChan ][ i ] );
            }
            else
            {
                vertex.Colors.Elements[ colChan ] = { 1.0f, 1.0f, 1.0f, 1.0f };
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

void AssimpImporter::Impl::ProcessMaterial( ImportContext &context, const aiMaterial *material ) const
{
    AssetUri          assetUri;
    const std::string matNameStr = material->GetName( ).C_Str( );
    InteropString     matName    = AssetPathUtilities::SanitizeAssetName( matNameStr.c_str( ) );
    if ( matName.IsEmpty( ) )
    {
        matName = InteropString( "Material_" ).Append( std::to_string( context.MaterialNameToAssetUriMap.size( ) ).c_str( ) );
    }
    if ( context.MaterialNameToAssetUriMap.contains( matNameStr ) )
    {
        return;
    }
    spdlog::info( "Processing material: {}", matName.Get( ) );
    MaterialAsset matAsset;
    matAsset.Name = matName;
    aiColor4D color;
    if ( material->Get( AI_MATKEY_COLOR_DIFFUSE, color ) == AI_SUCCESS )
    {
        matAsset.BaseColorFactor = ConvertColor( color );
    }
    if ( AssetUri albedoUri; context.Desc.ImportTextures && ProcessTexture( context, material, aiTextureType_DIFFUSE, "Albedo", albedoUri ) )
    {
        matAsset.AlbedoMapRef = albedoUri;
    }
    if ( AssetUri normalUri; context.Desc.ImportTextures && ( ProcessTexture( context, material, aiTextureType_NORMALS, "Normal", normalUri ) ||
                                                              ProcessTexture( context, material, aiTextureType_HEIGHT, "Normal", normalUri ) ) )
    {
        matAsset.NormalMapRef = normalUri;
    }
    if ( AssetUri mrUri; context.Desc.ImportTextures && ProcessTexture( context, material, aiTextureType_METALNESS, "MetallicRoughness", mrUri ) )
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
    if ( AssetUri emUri; context.Desc.ImportTextures && ProcessTexture( context, material, aiTextureType_EMISSIVE, "Emissive", emUri ) )
    {
        matAsset.EmissiveMapRef = emUri;
    }
    if ( material->Get( AI_MATKEY_COLOR_EMISSIVE, color ) == AI_SUCCESS )
    {
        matAsset.EmissiveFactor = { color.r, color.g, color.b };
    }
    if ( AssetUri ocUri; context.Desc.ImportTextures && ProcessTexture( context, material, aiTextureType_AMBIENT_OCCLUSION, "Occlusion", ocUri ) )
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

bool AssimpImporter::Impl::ProcessTexture( ImportContext &context, const aiMaterial *material, const aiTextureType textureType, const InteropString &semanticName,
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
        spdlog::info( "Processing embedded texture for material ' {} ', semantic: {}", material->GetName( ).C_Str( ), semanticName.Get( ) );
        if ( const int textureIndex = std::stoi( texPathStr.substr( 1 ) ); textureIndex >= 0 && textureIndex < static_cast<int>( context.Scene->mNumTextures ) )
        {
            WriteTextureAsset( context, context.Scene->mTextures[ textureIndex ], "", semanticName, outAssetUri );
            return true;
        }
        spdlog::error( "Invalid embedded texture index {} for material '{} '.", texPathStr.substr( 1 ), material->GetName( ).C_Str( ) );
        return false;
    }
    spdlog::info( "Processing external texture reference: ' {} ' for semantic: {}", texPathStr, semanticName.Get( ) );
    if ( context.TexturePathToAssetUriMap.contains( texPathStr ) )
    {
        outAssetUri = context.TexturePathToAssetUriMap[ texPathStr ];
        return false;
    }
    const std::filesystem::path modelPath           = FileIO::GetResourcePath( context.SourceFilePath ).Get( );
    const std::filesystem::path texturePath         = texPathStr;
    const std::filesystem::path absoluteTexturePath = texturePath.is_absolute( ) ? texturePath : absolute( modelPath.parent_path( ) / texturePath );
    if ( !exists( absoluteTexturePath ) )
    {
        spdlog::error( "External texture file not found: {} (referenced by material '{} ')", absoluteTexturePath.string( ), material->GetName( ).C_Str( ) );
        return false;
    }
    WriteTextureAsset( context, nullptr, absoluteTexturePath.string( ), semanticName, outAssetUri );
    return true;
}

void AssimpImporter::Impl::ProcessAnimation( ImportContext &context, const aiAnimation *animation, AssetUri &outAssetUri ) const
{
    const std::string animNameStr = animation->mName.C_Str( );
    InteropString     animName    = AssetPathUtilities::SanitizeAssetName( animNameStr.c_str( ) );
    if ( animName.IsEmpty( ) )
    {
        animName = InteropString( "Animation_" ).Append( std::to_string( context.Result.CreatedAssets.NumElements ).c_str( ) );
    }
    spdlog::info( "Processing animation: {} Duration: {} Ticks/Sec: {}", animName.Get( ), animation->mDuration, animation->mTicksPerSecond );

    AnimationAsset animAsset;
    animAsset.Name        = animName;
    animAsset.SkeletonRef = context.SkeletonAssetUri;

    AnimationClip clip;
    clip.Name                   = animName;
    const double ticksPerSecond = animation->mTicksPerSecond > 0 ? animation->mTicksPerSecond : 24.0;
    clip.Duration               = static_cast<float>( animation->mDuration / ticksPerSecond );

    spdlog::info( "Processing {} joint animation channels (raw keys).", animation->mNumChannels );
    clip.Tracks = JointAnimTrackArray::Create( animation->mNumChannels );
    for ( unsigned int i = 0; i < animation->mNumChannels; ++i )
    {
        const aiNodeAnim *nodeAnim = animation->mChannels[ i ];
        JointAnimTrack   &track    = clip.Tracks.Elements[ i ];
        track.JointName            = nodeAnim->mNodeName.C_Str( );

        if ( !context.BoneNameToIndexMap.contains( track.JointName.Get( ) ) )
        {
            spdlog::warn( "Animation channel ' {} ' does not correspond to a known skeleton joint. Skipping channel.", track.JointName.Get( ) );
            continue;
        }

        track.PositionKeys = PositionKeyArray::Create( nodeAnim->mNumPositionKeys );
        for ( unsigned int k = 0; k < nodeAnim->mNumPositionKeys; ++k )
        {
            PositionKey &key = track.PositionKeys.Elements[ k ];
            key.Timestamp    = static_cast<float>( nodeAnim->mPositionKeys[ k ].mTime / ticksPerSecond );
            key.Value        = ConvertVector3( nodeAnim->mPositionKeys[ k ].mValue );

            key.Value.X *= context.Desc.ScaleFactor;
            key.Value.Y *= context.Desc.ScaleFactor;
            key.Value.Z *= context.Desc.ScaleFactor;
        }

        track.RotationKeys = RotationKeyArray::Create( nodeAnim->mNumRotationKeys );
        for ( unsigned int k = 0; k < nodeAnim->mNumRotationKeys; ++k )
        {
            RotationKey &key = track.RotationKeys.Elements[ k ];
            key.Timestamp    = static_cast<float>( nodeAnim->mRotationKeys[ k ].mTime / ticksPerSecond );
            key.Value        = ConvertQuaternion( nodeAnim->mRotationKeys[ k ].mValue );
        }

        track.ScaleKeys = ScaleKeyArray::Create( nodeAnim->mNumScalingKeys );
        for ( unsigned int k = 0; k < nodeAnim->mNumScalingKeys; ++k )
        {
            ScaleKey &key = track.ScaleKeys.Elements[ k ];
            key.Timestamp = static_cast<float>( nodeAnim->mScalingKeys[ k ].mTime / ticksPerSecond );
            key.Value     = ConvertVector3( nodeAnim->mScalingKeys[ k ].mValue );
        }
    }

    spdlog::info( "Processing {} morph target animation channels.", animation->mNumMorphMeshChannels );
    clip.MorphTracks = MorphAnimTrackArray::Create( animation->mNumMorphMeshChannels );
    for ( unsigned int i = 0; i < animation->mNumMorphMeshChannels; ++i )
    {
        const aiMeshMorphAnim *morphAnim = animation->mMorphMeshChannels[ i ];
        MorphAnimTrack        &track     = clip.MorphTracks.Elements[ i ];
        track.Name                       = morphAnim->mName.C_Str( );
        track.Keyframes                  = MorphKeyframeArray::Create( morphAnim->mNumKeys );
        for ( unsigned int k = 0; k < morphAnim->mNumKeys; ++k )
        {
            MorphKeyframe        &keyframe = track.Keyframes.Elements[ k ];
            const aiMeshMorphKey &aiKey    = morphAnim->mKeys[ k ];
            keyframe.Timestamp             = static_cast<float>( aiKey.mTime / ticksPerSecond );
            keyframe.Weight                = aiKey.mNumValuesAndWeights > 0 ? static_cast<float>( aiKey.mWeights[ 0 ] ) : 0.0f;
        }
    }

    // Todo this doesn't seem right
    animAsset.Animations.Elements    = &clip;
    animAsset.Animations.NumElements = 1;
    WriteAnimationAsset( context, animAsset, outAssetUri );
    spdlog::info( "Successfully wrote animation asset: {}", outAssetUri.ToInteropString( ).Get( ) );
}

void AssimpImporter::Impl::CalculateMeshBounds( const aiMesh *mesh, const float scaleFactor, Float_3 &outMin, Float_3 &outMax ) const
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

void AssimpImporter::Impl::GenerateMeshLODs( const ImportContext & /*context*/, MeshAssetWriter & /*meshWriter*/ ) const
{
    // TODO Implement this with MeshOptimizer
    spdlog::warn( "Not yet implemented" );
}

void AssimpImporter::Impl::ConfigureAssimpImportFlags( const AssimpImportDesc &options, unsigned int &flags, Assimp::Importer &importer ) const
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
            spdlog::warn( "MergeMeshes and PreTransformVertices may conflict. Disabling PreTransformVertices." );
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

void AssimpImporter::Impl::WriteMaterialAsset( ImportContext &context, const MaterialAsset &materialAsset, AssetUri &outAssetUri ) const
{
    const InteropString assetFilename   = AssetPathUtilities::CreateAssetFileName( context.AssetNamePrefix, materialAsset.Name, "Material", MaterialAsset::Extension( ) );
    const InteropString targetAssetPath = FileIO::GetAbsolutePath( InteropString( context.TargetDirectory ).Append( "/" ).Append( assetFilename.Get( ) ) );
    outAssetUri                         = AssetUri::Create( assetFilename );
    MaterialAsset mutableAsset          = materialAsset;
    mutableAsset.Uri                    = outAssetUri;
    spdlog::info( "Writing Material asset to: {}", targetAssetPath.Get( ) );
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

void AssimpImporter::Impl::WriteTextureAsset( ImportContext &context, const aiTexture *texture, const std::string &path, const InteropString &semanticName,
                                              AssetUri &outAssetUri ) const
{
    InteropString texName;
    if ( texture != nullptr )
    {
        texName = AssetPathUtilities::SanitizeAssetName( texture->mFilename.length > 0 ? texture->mFilename.C_Str( ) : semanticName.Get( ) );
        if ( texName.IsEmpty( ) )
        {
            texName = InteropString( semanticName ).Append( "_Tex_" ).Append( std::to_string( context.Result.CreatedAssets.NumElements ).c_str( ) );
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
    spdlog::info( "Writing embedded Texture asset to: {} (Semantic: {} )", targetAssetPath.Get( ), semanticName.Get( ) );
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
    texAsset.Mips         = TextureMipArray::Create( sourceTexture->GetMipLevels( ) * sourceTexture->GetArraySize( ) );

    // Have to double stream to write metadata correctly unfortunately
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
        mipDataBuffer.Elements    = sourceTexture->GetData( ).Data( ) + mipOffset;
        mipDataBuffer.NumElements = mipSize;
        assetWriter.AddPixelData( mipDataBuffer, mipData.MipIndex, mipData.ArrayIndex );
    }

    assetWriter.End( );
    texAsset.Dispose( );
    RegisterCreatedAsset( context, outAssetUri );
    context.TexturePathToAssetUriMap[ targetAssetPath.Get( ) ] = outAssetUri;
}

void AssimpImporter::Impl::WriteSkeletonAsset( ImportContext &context, const SkeletonAsset &skeletonAsset ) const
{
    const InteropString assetFilename   = AssetPathUtilities::CreateAssetFileName( context.AssetNamePrefix, skeletonAsset.Name, "Skeleton", SkeletonAsset::Extension( ) );
    const InteropString targetAssetPath = FileIO::GetAbsolutePath( InteropString( context.TargetDirectory ).Append( "/" ).Append( assetFilename.Get( ) ) );
    context.SkeletonAssetUri            = AssetUri::Create( assetFilename );
    SkeletonAsset mutableAsset          = skeletonAsset;
    mutableAsset.Uri                    = context.SkeletonAssetUri;
    spdlog::info( "Writing Skeleton asset to: {}", targetAssetPath.Get( ) );
    BinaryWriter              writer( targetAssetPath );
    const SkeletonAssetWriter assetWriter( { &writer } );
    assetWriter.Write( mutableAsset );
    RegisterCreatedAsset( context, context.SkeletonAssetUri );
}

void AssimpImporter::Impl::WriteAnimationAsset( ImportContext &context, const AnimationAsset &animationAsset, AssetUri &outAssetUri ) const
{
    const InteropString assetFilename   = AssetPathUtilities::CreateAssetFileName( context.AssetNamePrefix, animationAsset.Name, "Animation", AnimationAsset::Extension( ) );
    const InteropString targetAssetPath = FileIO::GetAbsolutePath( InteropString( context.TargetDirectory ).Append( "/" ).Append( assetFilename.Get( ) ) );
    outAssetUri                         = AssetUri::Create( assetFilename );
    AnimationAsset mutableAsset         = animationAsset;
    mutableAsset.Uri                    = outAssetUri;
    spdlog::info( "Writing Animation asset to: {}", targetAssetPath.Get( ) );
    BinaryWriter         writer( targetAssetPath );
    AnimationAssetWriter assetWriter( { &writer } );
    assetWriter.Write( mutableAsset );
    RegisterCreatedAsset( context, outAssetUri );
}

Float_4x4 AssimpImporter::Impl::ConvertMatrix( const aiMatrix4x4 &matrix ) const
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

    return InteropMathConverter::Float_4X4FromXMMATRIX( matrixXM );
}

Float_4 AssimpImporter::Impl::ConvertQuaternion( const aiQuaternion &quat ) const
{
    return { quat.x, quat.y, quat.z, quat.w };
}
Float_3 AssimpImporter::Impl::ConvertVector3( const aiVector3D &vec ) const
{
    return { vec.x, vec.y, vec.z };
}
Float_2 AssimpImporter::Impl::ConvertVector2( const aiVector3D &vec ) const
{
    return { vec.x, vec.y };
}
Float_4 AssimpImporter::Impl::ConvertColor( const aiColor4D &color ) const
{
    return { color.r, color.g, color.b, color.a };
}

// File path utility methods moved to FilePathUtilities class

void AssimpImporter::Impl::RegisterCreatedAsset( ImportContext &context, const AssetUri &assetUri ) const
{
    // TODO: Cleaner growing mechanism
    auto newElements = new AssetUri[ context.Result.CreatedAssets.NumElements + 1 ];
    for ( size_t i = 0; i < context.Result.CreatedAssets.NumElements; i++ )
    {
        newElements[ i ] = context.Result.CreatedAssets.Elements[ i ];
    }
    newElements[ context.Result.CreatedAssets.NumElements ] = assetUri;

    delete[] context.Result.CreatedAssets.Elements;
    context.Result.CreatedAssets.Elements = newElements;
    context.Result.CreatedAssets.NumElements++;
}
