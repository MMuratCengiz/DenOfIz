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

#pragma once

#include <assimp/scene.h>
#include <unordered_map>
#include <vector>
#include "AssimpImportContext.h"
#include "DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAsset.h"

namespace DenOfIz
{
    struct SkeletonBuildStats
    {
        uint32_t TotalJoints         = 0;
        uint32_t MaxChildrenPerJoint = 0;
        size_t   RequiredArenaSize   = 0;
    };

    class AssimpSkeletonProcessor
    {
        SkeletonBuildStats m_stats;

    public:
        AssimpSkeletonProcessor( );
        ~AssimpSkeletonProcessor( );

        ImporterResultCode PreprocessSkeleton( AssimpImportContext &context, SkeletonBuildStats &stats );
        ImporterResultCode BuildSkeleton( AssimpImportContext &context, SkeletonAsset &skeletonAsset );
        ImporterResultCode WriteSkeletonAsset( AssimpImportContext &context, SkeletonAsset &skeletonAsset ) const;

    private:
        void CollectBonesFromMeshes( AssimpImportContext &context ) const;
        void CountBonesInHierarchy( const aiNode *node, const std::unordered_map<std::string, uint32_t> &boneMap, uint32_t &numJoints );
        void CalculateMaxChildren( const aiNode *node, const std::unordered_map<std::string, uint32_t> &boneMap, std::unordered_map<const aiNode *, uint32_t> &childrenCount,
                                   uint32_t &maxChildren );

        ImporterResultCode ProcessNodeHierarchy( AssimpImportContext &context, const aiNode *node, SkeletonAsset &skeletonAsset, int32_t parentJointIndex, uint32_t &jointIndex,
                                                 std::vector<std::vector<uint32_t>> &childrenIndices );

        void FinalizeJointChildren( SkeletonAsset &skeletonAsset, const std::vector<std::vector<uint32_t>> &childrenIndices ) const;

        Float_4x4 ConvertMatrix( const aiMatrix4x4 &matrix ) const;
        Float_4   ConvertQuaternion( const aiQuaternion &quat ) const;
        Float_3   ConvertVector3( const aiVector3D &vec, float scaleFactor = 1.0f ) const;
    };

} // namespace DenOfIz
