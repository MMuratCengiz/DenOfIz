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

#include <DenOfIzGraphics/Assets/Bundle/BundleManager.h>
#include <DenOfIzGraphics/Assets/FileSystem/FileIO.h>
#include <DenOfIzGraphics/Assets/Import/IAssetImporter.h>
#include <DenOfIzGraphics/Assets/Serde/Animation/AnimationAsset.h>
#include <DenOfIzGraphics/Assets/Serde/Material/MaterialAsset.h>
#include <DenOfIzGraphics/Assets/Serde/Mesh/MeshAsset.h>
#include <DenOfIzGraphics/Assets/Serde/Mesh/MeshAssetWriter.h>
#include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAsset.h>
#include <DenOfIzGraphics/Utilities/InteropMath.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <functional>
#include <typeinfo>
#include <unordered_map>

namespace DenOfIz
{
    struct DZ_API AssimpImportOptions : ImportOptions
    {
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
        AssimpImportOptions( )            = default;
        explicit AssimpImportOptions( const ImportOptions &base ) : ImportOptions( base )
        {
        }
        static AssimpImportOptions CreateFromBase( const ImportOptions &base )
        {
            return AssimpImportOptions( base );
        }
    };

    struct DZ_API AssimpImporterDesc
    {
        InteropString  BundleOutputPath;
        BundleManager *BundleManager = nullptr;
    };

    class AssimpImporter final : public IAssetImporter
    {
        ImporterDesc             m_importerInfo;
        const AssimpImporterDesc m_desc;

        struct ImportContext
        {
            const aiScene      *Scene = nullptr;
            InteropString       SourceFilePath;
            InteropString       TargetDirectory;
            InteropString       AssetNamePrefix;
            AssimpImportOptions Options;
            ImporterResult      Result;
            InteropString       ErrorMessage;

            std::unordered_map<std::string, AssetUri>    MaterialNameToAssetUriMap;
            std::unordered_map<std::string, AssetUri>    TexturePathToAssetUriMap;
            std::unordered_map<std::string, uint32_t>    BoneNameToIndexMap;
            std::unordered_map<std::string, aiMatrix4x4> BoneNameToInverseBindMatrixMap;
            AssetUri                                     SkeletonAssetUri;
            uint32_t                                     CurrentSubMeshIndex      = 0;
            MeshAsset                                   *CurrentMeshAssetMetadata = nullptr; // Pointer to the MeshAsset being built
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
        static ImporterResultCode ProcessMesh( ImportContext &context, const aiMesh *mesh, MeshAssetWriter &assetWriter ); // Changed MeshAsset& to MeshAssetWriter&
        ImporterResultCode        ProcessMaterial( ImportContext &context, const aiMaterial *material, AssetUri &outAssetUri );
        ImporterResultCode        ProcessTexture( ImportContext &context, const aiMaterial *material, aiTextureType textureType, const InteropString &semanticName,
                                                  AssetUri &outAssetUri ) const;
        static ImporterResultCode ProcessSkeleton( SkeletonAsset &skeletonAsset );
        ImporterResultCode        ProcessAnimation( ImportContext &context, const aiAnimation *animation, AssetUri &outAssetUri );

        static void        ConfigureAssimpImportFlags( const AssimpImportOptions &options, unsigned int &flags, Assimp::Importer &importer );
        ImporterResultCode WriteMaterialAsset( ImportContext &context, const MaterialAsset &materialAsset, AssetUri &outAssetUri ) const;
        static void        CalculateMeshBounds( const aiMesh *mesh, float scaleFactor, Float_3 &outMin, Float_3 &outMax );

        ImporterResultCode WriteTextureAsset( ImportContext &context, const aiTexture *texture, const InteropString &semanticName, AssetUri &outAssetUri ) const;
        ImporterResultCode WriteSkeletonAsset( ImportContext &context, const SkeletonAsset &skeletonAsset ) const;
        ImporterResultCode WriteAnimationAsset( ImportContext &context, const AnimationAsset &animationAsset, AssetUri &outAssetUri ) const;
        static Float_4x4   ConvertMatrix( const aiMatrix4x4 &matrix );
        static Float_4     ConvertQuaternion( const aiQuaternion &quat );
        static Float_3     ConvertVector3( const aiVector3D &vec );
        static Float_2     ConvertVector2( const aiVector3D &vec );
        static Float_4     ConvertColor( const aiColor4D &color );

        static InteropString CreateAssetFileName( const InteropString &prefix, const InteropString &name, const InteropString &assetType, const InteropString &extension );
        static InteropString GetAssetNameFromFilePath( const InteropString &filePath );
        static InteropString SanitizeAssetName( const InteropString &name );
        static InteropString GetFileExtension( const InteropString &filePath );
        static InteropString GetFileNameWithoutExtension( const InteropString &filePath );

        static void RegisterCreatedAsset( ImportContext &context, const AssetUri &assetUri, AssetType assetType ); // Made const
        static void GenerateMeshLODs( const ImportContext &context, MeshAssetWriter &meshWriter );
    };

} // namespace DenOfIz
