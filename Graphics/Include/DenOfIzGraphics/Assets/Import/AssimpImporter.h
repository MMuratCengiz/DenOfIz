/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

// Include first due to DirectXMath breaking NULL on external libraries
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <functional>
#include <typeinfo>
#include <unordered_map>

#include <DenOfIzGraphics/Assets/Bundle/BundleManager.h>
#include <DenOfIzGraphics/Assets/FileSystem/FileIO.h>
#include <DenOfIzGraphics/Assets/Import/IAssetImporter.h>
#include <DenOfIzGraphics/Assets/Serde/Animation/AnimationAsset.h>
#include <DenOfIzGraphics/Assets/Serde/Material/MaterialAsset.h>
#include <DenOfIzGraphics/Assets/Serde/Mesh/MeshAsset.h>
#include <DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetWriter.h>
#include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAsset.h>
#include <DenOfIzGraphics/Utilities/InteropMath.h>

namespace DenOfIz
{
    struct DZ_API AssimpImportDesc : ImportDesc
    {
        bool     OverwriteExisting        = true;
        bool     GenerateLODs             = true;
        uint32_t MaxLODCount              = 3;
        Float_3  LODScreenPercentages     = { 1.0f, 0.5f, 0.25f };
        bool     OptimizeMeshes           = true;
        float    ScaleFactor              = 1.0f;
        bool     JoinIdenticalVertices    = true;
        bool     PreTransformVertices     = false;
        bool     LimitBoneWeights         = true;
        uint32_t MaxBoneWeightsPerVertex  = 4;
        bool     RemoveRedundantMaterials = true;
        bool     MergeMeshes              = false;
        bool     OptimizeGraph            = true;
        bool     GenerateNormals          = true;
        bool     SmoothNormals            = true;
        float    SmoothNormalsAngle       = 80.0f;
        bool     TriangulateMeshes        = true;
        bool     PreservePivots           = true;
        bool     DropNormals              = false;
        bool     ConvertToLeftHanded      = true; // DenOfIz uses a left handed coordinate system, DirectX12 settings
        bool     CalculateTangentSpace    = true;
        AssimpImportDesc( )               = default;
        explicit AssimpImportDesc( const ImportDesc &base ) : ImportDesc( base )
        {
        }
    };

    struct DZ_API AssimpImporterDesc{ };

    class AssimpImporter final : public IAssetImporter
    {
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
            MeshAsset                                    MeshAsset{ }; // MeshAsset being built
        };

    public:
        DZ_API explicit AssimpImporter( AssimpImporterDesc desc );
        DZ_API ~AssimpImporter( ) override;

        DZ_API [[nodiscard]] ImporterDesc GetImporterInfo( ) const override;
        DZ_API [[nodiscard]] bool         CanProcessFileExtension( const InteropString &extension ) const override;
        DZ_API ImporterResult             Import( const ImportJobDesc &desc ) override;
        DZ_API [[nodiscard]] bool         ValidateFile( const InteropString &filePath ) const override;

    private:
        ImporterResultCode ImportSceneInternal( ImportContext &context );
        ImporterResultCode ProcessNode( ImportContext &context, const aiNode *node, MeshAssetWriter *meshWriter, SkeletonAsset &skeletonAsset, int32_t parentJointIndex = -1 );
        ImporterResultCode ProcessMesh( ImportContext &context, const aiMesh *mesh, MeshAssetWriter &assetWriter ); // Changed MeshAsset& to MeshAssetWriter&
        void               ProcessMaterial( ImportContext &context, const aiMaterial *material );
        bool ProcessTexture( ImportContext &context, const aiMaterial *material, aiTextureType textureType, const InteropString &semanticName, AssetUri &outAssetUri );
        void ProcessAnimation( ImportContext &context, const aiAnimation *animation, AssetUri &outAssetUri );

        void ConfigureAssimpImportFlags( const AssimpImportDesc &options, unsigned int &flags, Assimp::Importer &importer );
        void CalculateMeshBounds( const aiMesh *mesh, float scaleFactor, Float_3 &outMin, Float_3 &outMax );

        void WriteMaterialAsset( ImportContext &context, const MaterialAsset &materialAsset, AssetUri &outAssetUri );
        // Either texture(for embedded textures) or path for reference textures
        void WriteTextureAsset( ImportContext &context, const aiTexture *texture, const std::string &path, const InteropString &semanticName, AssetUri &outAssetUri );
        void WriteSkeletonAsset( ImportContext &context, const SkeletonAsset &skeletonAsset );
        void WriteAnimationAsset( ImportContext &context, const AnimationAsset &animationAsset, AssetUri &outAssetUri );

        Float_4x4 ConvertMatrix( const aiMatrix4x4 &matrix ) const;
        Float_4   ConvertQuaternion( const aiQuaternion &quat );
        Float_3   ConvertVector3( const aiVector3D &vec );
        Float_2   ConvertVector2( const aiVector3D &vec );
        Float_4   ConvertColor( const aiColor4D &color );

        InteropString CreateAssetFileName( const InteropString &prefix, const InteropString &name, const InteropString &assetType, const InteropString &extension );
        InteropString GetAssetNameFromFilePath( const InteropString &filePath );
        InteropString SanitizeAssetName( const InteropString &name );
        InteropString GetFileExtension( const InteropString &filePath ) const;
        InteropString GetFileNameWithoutExtension( const InteropString &filePath );

        void RegisterCreatedAsset( ImportContext &context, const AssetUri &assetUri );
        void GenerateMeshLODs( const ImportContext &context, MeshAssetWriter &meshWriter );
    };

} // namespace DenOfIz
