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
#include "AssimpImportContext.h"
#include "DenOfIzGraphics/Assets/Serde/Animation/AnimationAsset.h"

namespace DenOfIz
{
    struct AnimationProcessingStats
    {
        uint32_t TotalAnimations   = 0;
        uint32_t TotalTracks       = 0;
        uint32_t TotalPositionKeys = 0;
        uint32_t TotalRotationKeys = 0;
        uint32_t TotalScaleKeys    = 0;
        uint32_t TotalMorphKeys    = 0;
        size_t   RequiredArenaSize = 0;
    };

    class AssimpAnimationProcessor
    {
        AnimationProcessingStats m_stats;

    public:
        AssimpAnimationProcessor( );
        ~AssimpAnimationProcessor( );

        ImporterResultCode PreprocessAnimations( const AssimpImportContext &context, AnimationProcessingStats &stats );
        ImporterResultCode ProcessAllAnimations( AssimpImportContext &context ) const;
        ImporterResultCode ProcessAnimation( AssimpImportContext &context, const aiAnimation *animation, AssetUri &outAssetUri ) const;

    private:
        void   CountAnimationData( const aiAnimation *animation, AnimationProcessingStats &stats ) const;
        size_t CalculateAnimationMemoryRequirements( const AnimationProcessingStats &stats ) const;

        ImporterResultCode BuildAnimationClip( const AssimpImportContext &context, const aiAnimation *animation, AnimationClip &clip ) const;
        void               ProcessJointAnimationTrack( const AssimpImportContext &context, const aiNodeAnim *nodeAnim, JointAnimTrack &track, double ticksPerSecond ) const;
        void               ProcessMorphAnimationTrack( const AssimpImportContext &context, const aiMeshMorphAnim *morphAnim, MorphAnimTrack &track, double ticksPerSecond ) const;

        ImporterResultCode WriteAnimationAsset( AssimpImportContext &context, AnimationAsset &animationAsset, AssetUri &outAssetUri ) const;

        Float_4 ConvertQuaternion( const aiQuaternion &quat ) const;
        Float_3 ConvertVector3( const aiVector3D &vec, float scaleFactor = 1.0f ) const;
    };
} // namespace DenOfIz
