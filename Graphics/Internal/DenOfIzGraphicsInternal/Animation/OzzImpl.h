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

#include "DenOfIzGraphics/Animation/Ozz.h"

#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/animation/runtime/track.h>
#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/base/memory/unique_ptr.h>

#include <string>
#include <vector>

namespace DenOfIz
{
    struct InternalContext
    {
        ozz::unique_ptr<ozz::animation::Animation>            animation;
        ozz::unique_ptr<ozz::animation::SamplingJob::Context> samplingContext;
        ozz::vector<ozz::math::SoaTransform>                  localTransforms;
        ozz::vector<ozz::math::Float4x4>                      modelTransforms;

        ozz::vector<ozz::unique_ptr<ozz::animation::FloatTrack>>      floatTracks;
        ozz::vector<ozz::unique_ptr<ozz::animation::Float2Track>>     float2Tracks;
        ozz::vector<ozz::unique_ptr<ozz::animation::Float3Track>>     float3Tracks;
        ozz::vector<ozz::unique_ptr<ozz::animation::Float4Track>>     float4Tracks;
        ozz::vector<ozz::unique_ptr<ozz::animation::QuaternionTrack>> quaternionTracks;

        std::string animationName;
    };

    class OzzSkeletonImpl
    {
    public:
        std::vector<std::vector<DenOfIz_TrackTriggeringEdge>> m_trackTriggeringEdges;
        ozz::unique_ptr<ozz::animation::Skeleton>             skeleton;
        ozz::vector<InternalContext *>                        contexts;
        std::vector<std::string>                              m_jointNames;
        std::vector<DenOfIz_StringView>                       m_jointNamesView;
        std::vector<int32_t>                                  m_jointParents;
        std::vector<DenOfIz_OzzJoint>                         m_joints;
        std::vector<DenOfIz_OzzJointTransform>                m_restPoses;
        bool                                                  m_valid = false;

        explicit OzzSkeletonImpl( DenOfIz_StringView skeletonFilePath );
        explicit OzzSkeletonImpl( DenOfIz_BinaryContainer skeletonData );
        ~OzzSkeletonImpl( );

        void LoadSkeleton( DenOfIz_StringView skeletonFilePath );
        void LoadSkeletonFromBinaryContainer( DenOfIz_BinaryContainer skeletonData );
        bool LoadAnimation( DenOfIz_StringView animationFilePath, InternalContext *context ) const;
        bool LoadAnimationFromBinaryContainer( DenOfIz_BinaryContainer animationData, InternalContext *context ) const;
    };
} // namespace DenOfIz

#define OZZ_SKELETON_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::OzzSkeletonImpl, handle )
#define OZZ_CONTEXT_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::InternalContext, handle )
