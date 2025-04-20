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

#include "ozz/base/containers/vector.h"
#include "ozz/base/memory/unique_ptr.h"

#include <DenOfIzGraphics/Assets/Serde/Animation/AnimationAsset.h>
#include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAsset.h>
#include <DenOfIzGraphics/DenOfIzGraphics.h>
#include <complex.h>
#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/blending_job.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/animation/runtime/skeleton.h>
#include <string>
#include <unordered_map>

namespace DenOfIz
{
    namespace Internal
    {
        struct AnimationState
        {
            std::string                                                                         Name;
            float                                                                               PlaybackSpeed = 1.0f;
            float                                                                               CurrentTime   = 0.0f;
            float                                                                               Weight        = 1.0f;
            bool                                                                                Loop          = true;
            bool                                                                                Playing       = false;
            std::unique_ptr<ozz::animation::Animation, ozz::Deleter<ozz::animation::Animation>> OzzAnimation;
            std::unique_ptr<ozz::animation::SamplingJob::Context>                               Context;
        };

        struct BlendingState
        {
            std::string SourceAnimation;
            std::string TargetAnimation;
            float       BlendTime        = 0.5f;
            float       CurrentBlendTime = 0.0f;
            bool        InProgress       = false;
        };
    } // namespace Internal
    using namespace Internal;

    // Configuration for the AnimationStateManager
    struct DZ_API AnimationStateManagerDesc
    {
        SkeletonAsset *Skeleton = nullptr;
    };

    class DZ_API AnimationStateManager
    {
        ozz::unique_ptr<ozz::animation::Skeleton>       m_skeleton;
        std::unordered_map<std::string, AnimationState> m_animations;
        std::string                                     m_currentAnimation;
        BlendingState                                   m_blendingState;

        ozz::vector<ozz::math::SoaTransform> m_localTransforms;
        ozz::vector<ozz::math::Float4x4>     m_modelTransforms;

    public:
        DZ_API explicit AnimationStateManager( const AnimationStateManagerDesc &desc );
        DZ_API ~AnimationStateManager( );

        DZ_API void  AddAnimation( const AnimationAsset &animationAsset );
        DZ_API void  Play( const std::string &animationName, bool loop = true );
        DZ_API void  BlendTo( const std::string &animationName, float blendTime = 0.5f );
        DZ_API void  Stop( );
        DZ_API void  Pause( );
        DZ_API void  Resume( );
        DZ_API void  Update( float deltaTime );
        DZ_API bool  HasAnimation( const std::string &animationName ) const;
        DZ_API void  GetModelSpaceTransforms( InteropArray<Float_4x4> &outTransforms ) const;
        DZ_API const std::string &GetCurrentAnimationName( ) const;
        DZ_API int                GetNumJoints( ) const;

    private:
        std::unique_ptr<ozz::animation::Animation, ozz::Deleter<ozz::animation::Animation>> ConvertToOzzAnimation( const AnimationClip &clip );
        void                         SampleAnimation( AnimationState &state, ozz::vector<ozz::math::SoaTransform> &output ) const;
        void                         UpdateBlending( float deltaTime );
        static ozz::math::Transform  GetJointLocalTransform( const Joint &joint );
        static ozz::math::Float3     ToOzzTranslation( const Float_3 &translation );
        static ozz::math::Quaternion ToOzzRotation( const Float_4 &rotation );
        static ozz::math::Float3     ToOzzScale( const Float_3 &scale );
        static Float_3               FromOzzTranslation( const ozz::math::Float3 &translation );
        static Float_4               FromOzzRotation( const ozz::math::Quaternion &rotation );
        static Float_3               FromOzzScale( const ozz::math::Float3 &scale );
    };
} // namespace DenOfIz
