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

#include <ozz/animation/offline/animation_builder.h>
#include <ozz/animation/offline/raw_animation.h>
#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/animation/offline/raw_track.h>
#include <ozz/animation/offline/skeleton_builder.h>
#include <ozz/animation/offline/track_builder.h>

#include <ozz/animation/runtime/animation.h>
#include <ozz/animation/runtime/blending_job.h>
#include <ozz/animation/runtime/ik_aim_job.h>
#include <ozz/animation/runtime/ik_two_bone_job.h>
#include <ozz/animation/runtime/local_to_model_job.h>
#include <ozz/animation/runtime/sampling_job.h>
#include <ozz/animation/runtime/skeleton.h>
#include <ozz/animation/runtime/track.h>
#include <ozz/animation/runtime/track_sampling_job.h>

#include <ozz/base/containers/vector.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/base/maths/vec_float.h>
#include <ozz/base/memory/unique_ptr.h>
#include <ozz/base/span.h>

#include "ozz/animation/runtime/track_triggering_job.h"
#include "ozz/base/maths/simd_quaternion.h"
#include "ozz/geometry/runtime/skinning_job.h"

#include "DenOfIzGraphics/Animation/OzzAnimation.h"
#include "DenOfIzGraphicsInternal/Utilities/InteropMathConverter.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include <ranges>

namespace DenOfIz
{
    namespace OzzUtils
    {
        static ozz::math::Transform  GetJointLocalTransform( const Joint &joint );
        static ozz::math::Float3     ToOzzTranslation( const Float_3 &translation );
        static ozz::math::Quaternion ToOzzRotation( const Float_4 &rotation );
        static ozz::math::Float3     ToOzzScale( const Float_3 &scale );
        static Float_3               FromOzzTranslation( const ozz::math::Float3 &translation );
        static Float_4               FromOzzRotation( const ozz::math::Quaternion &rotation );
        static Float_3               FromOzzScale( const ozz::math::Float3 &scale );
        static void                  CopyArrayToOzzVector( const InteropArray<Float_4x4> &src, ozz::vector<ozz::math::Float4x4> &dst );
        static void                  CopyOzzVectorToArray( const ozz::vector<ozz::math::Float4x4> &src, InteropArray<Float_4x4> &dst );
        static ozz::math::SimdFloat4 ToOzzSimdFloat4( const Float_3 &v );
        static Float_4               FromOzzSimdQuaternion( const ozz::math::SimdQuaternion &q );
        static ozz::math::Float4x4   ToOzzFloat4x4( const Float_4x4 &m );
        static Float_4x4             FromOzzFloat4x4( const ozz::math::Float4x4 &m );
    } // namespace OzzUtils

    struct InternalContext
    {
        ozz::unique_ptr<ozz::animation::Animation>            animation;
        ozz::unique_ptr<ozz::animation::SamplingJob::Context> samplingContext;
        ozz::vector<ozz::math::SoaTransform>                  localTransforms;
        ozz::vector<ozz::math::Float4x4>                      modelTransforms;

        // Track related data
        ozz::vector<ozz::unique_ptr<ozz::animation::FloatTrack>>      floatTracks;
        ozz::vector<ozz::unique_ptr<ozz::animation::Float2Track>>     float2Tracks;
        ozz::vector<ozz::unique_ptr<ozz::animation::Float3Track>>     float3Tracks;
        ozz::vector<ozz::unique_ptr<ozz::animation::Float4Track>>     float4Tracks;
        ozz::vector<ozz::unique_ptr<ozz::animation::QuaternionTrack>> quaternionTracks;
    };

    class OzzAnimation::Impl
    {
    public:
        ozz::unique_ptr<ozz::animation::Skeleton> skeleton;
        ozz::vector<InternalContext *>            contexts;

        explicit Impl( const SkeletonAsset *skeletonAsset )
        {
            BuildSkeleton( skeletonAsset );
        }

        ~Impl( )
        {
            for ( const auto context : contexts )
            {
                delete context;
            }
            contexts.clear( );
        }

        void BuildSkeleton( const SkeletonAsset *skeletonAsset )
        {
            if ( !skeletonAsset )
            {
                spdlog::error("Skeleton is required for OzzAnimation");
                return;
            }

            const auto  &joints    = skeletonAsset->Joints;
            const size_t numJoints = joints.NumElements( );

            ozz::vector<int16_t> parents( numJoints );

            ozz::animation::offline::RawSkeleton rawSkeleton;
            for ( size_t i = 0; i < numJoints; ++i )
            {
                const Joint &joint = joints.GetElement( i );
                parents[ i ]       = joint.ParentIndex;

                if ( joint.ParentIndex == -1 )
                {
                    ozz::animation::offline::RawSkeleton::Joint rootJoint;
                    rootJoint.name      = joint.Name.Get( );
                    rootJoint.transform = OzzUtils::GetJointLocalTransform( joint );

                    rawSkeleton.roots.push_back( rootJoint );
                }
            }

            for ( size_t i = 0; i < numJoints; ++i )
            {
                if ( parents[ i ] < 0 )
                {
                    continue;
                }

                std::function<ozz::animation::offline::RawSkeleton::Joint *( int32_t, ozz::vector<ozz::animation::offline::RawSkeleton::Joint> & )> FindJoint;
                FindJoint = [ & ]( const int32_t targetIdx, ozz::vector<ozz::animation::offline::RawSkeleton::Joint> &children ) -> ozz::animation::offline::RawSkeleton::Joint *
                {
                    for ( auto &child : children )
                    {
                        if ( child.name == joints.GetElement( targetIdx ).Name.Get( ) )
                        {
                            return &child;
                        }

                        if ( ozz::animation::offline::RawSkeleton::Joint *foundInChildren = FindJoint( targetIdx, child.children ) )
                        {
                            return foundInChildren;
                        }
                    }
                    return nullptr;
                };

                ozz::animation::offline::RawSkeleton::Joint *parentJoint = nullptr;
                for ( auto &root : rawSkeleton.roots )
                {
                    if ( root.name == joints.GetElement( parents[ i ] ).Name.Get( ) )
                    {
                        parentJoint = &root;
                        break;
                    }

                    parentJoint = FindJoint( parents[ i ], root.children );
                    if ( parentJoint )
                    {
                        break;
                    }
                }

                if ( parentJoint )
                {
                    const Joint &joint = joints.GetElement( i );

                    ozz::animation::offline::RawSkeleton::Joint childJoint;
                    childJoint.name      = joint.Name.Get( );
                    childJoint.transform = OzzUtils::GetJointLocalTransform( joint );

                    parentJoint->children.push_back( childJoint );
                }
            }

            ozz::animation::offline::SkeletonBuilder builder;
            if ( auto builtSkeleton = builder( rawSkeleton ) )
            {
                skeleton = std::move( builtSkeleton );
            }
            else
            {
                spdlog::error("Failed to build ozz skeleton");
            }
        }

        [[nodiscard]] ozz::unique_ptr<ozz::animation::Animation> ConvertToOzzAnimation( const AnimationClip &clip ) const
        {
            if ( !skeleton )
            {
                spdlog::error("Skeleton not initialized");
                return nullptr;
            }

            const float  duration  = clip.Duration;
            const size_t numJoints = skeleton->num_joints( );

            std::unordered_map<std::string, int> jointNameToIndexMap;
            for ( size_t i = 0; i < numJoints; ++i )
            {
                jointNameToIndexMap[ skeleton->joint_names( )[ i ] ] = i;
            }

            ozz::animation::offline::RawAnimation rawAnimation;
            rawAnimation.duration = duration;
            rawAnimation.tracks.resize( numJoints );

            for ( size_t i = 0; i < clip.Tracks.NumElements( ); ++i )
            {
                const JointAnimTrack &track      = clip.Tracks.GetElement( i );
                const std::string     jointName  = track.JointName.Get( );
                int                   jointIndex = -1;
                if ( auto it = jointNameToIndexMap.find( jointName ); it != jointNameToIndexMap.end( ) )
                {
                    jointIndex = it->second;
                }

                if ( jointIndex < 0 || jointIndex >= static_cast<int>( numJoints ) )
                {
                    spdlog::warn("Animation track for joint ' {} ' has no corresponding joint in skeleton", jointName);
                    continue;
                }

                ozz::animation::offline::RawAnimation::JointTrack &rawTrack = rawAnimation.tracks[ jointIndex ];

                const size_t numPosKeys = track.PositionKeys.NumElements( );
                for ( size_t j = 0; j < numPosKeys; ++j )
                {
                    const PositionKey                                    &key = track.PositionKeys.GetElement( j );
                    ozz::animation::offline::RawAnimation::TranslationKey rawKey;
                    rawKey.time  = key.Timestamp;
                    rawKey.value = OzzUtils::ToOzzTranslation( key.Value );
                    rawTrack.translations.push_back( rawKey );
                }

                const size_t numRotKeys = track.RotationKeys.NumElements( );
                for ( size_t j = 0; j < numRotKeys; ++j )
                {
                    const RotationKey                                 &key = track.RotationKeys.GetElement( j );
                    ozz::animation::offline::RawAnimation::RotationKey rawKey;
                    rawKey.time  = key.Timestamp;
                    rawKey.value = OzzUtils::ToOzzRotation( key.Value );
                    rawTrack.rotations.push_back( rawKey );
                }

                const size_t numScaleKeys = track.ScaleKeys.NumElements( );
                for ( size_t j = 0; j < numScaleKeys; ++j )
                {
                    const ScaleKey                                 &key = track.ScaleKeys.GetElement( j );
                    ozz::animation::offline::RawAnimation::ScaleKey rawKey;
                    rawKey.time  = key.Timestamp;
                    rawKey.value = OzzUtils::ToOzzScale( key.Value );
                    rawTrack.scales.push_back( rawKey );
                }
            }

            constexpr ozz::animation::offline::AnimationBuilder builder;
            auto                                                built = builder( rawAnimation );
            if ( !built )
            {
                spdlog::error("Failed to build ozz animation");
                return nullptr;
            }

            return built;
        }
    };

    namespace OzzUtils
    {
        static ozz::math::Transform GetJointLocalTransform( const Joint &joint )
        {
            ozz::math::Transform transform;
            transform.translation = ToOzzTranslation( joint.LocalTranslation );
            transform.rotation    = ToOzzRotation( joint.LocalRotationQuat );
            transform.scale       = ToOzzScale( joint.LocalScale );
            return transform;
        }

        static ozz::math::Float3 ToOzzTranslation( const Float_3 &translation )
        {
            return { translation.X, translation.Y, -translation.Z };
        }

        static ozz::math::Quaternion ToOzzRotation( const Float_4 &rotation )
        {
            return { -rotation.X, -rotation.Y, rotation.Z, rotation.W };
        }

        static ozz::math::Float3 ToOzzScale( const Float_3 &scale )
        {
            return { scale.X, scale.Y, scale.Z };
        }

        static Float_3 FromOzzTranslation( const ozz::math::Float3 &translation )
        {
            return Float_3{ translation.x, translation.y, -translation.z };
        }

        static Float_4 FromOzzRotation( const ozz::math::Quaternion &rotation )
        {
            return Float_4{ -rotation.x, -rotation.y, rotation.z, rotation.w };
        }

        static Float_3 FromOzzScale( const ozz::math::Float3 &scale )
        {
            return Float_3{ scale.x, scale.y, scale.z };
        }

        static void CopyArrayToOzzVector( const InteropArray<Float_4x4> &src, ozz::vector<ozz::math::Float4x4> &dst )
        {
            dst.resize( src.NumElements( ) );
            for ( size_t i = 0; i < src.NumElements( ); ++i )
            {
                dst[ i ] = ToOzzFloat4x4( src.GetElement( i ) );
            }
        }

        static void CopyOzzVectorToArray( const ozz::vector<ozz::math::Float4x4> &src, InteropArray<Float_4x4> &dst )
        {
            dst.Resize( src.size( ) );
            for ( size_t i = 0; i < src.size( ); ++i )
            {
                dst.GetElement( i ) = FromOzzFloat4x4( src[ i ] );
            }
        }

        static ozz::math::SimdFloat4 ToOzzSimdFloat4( const Float_3 &v )
        {
            return ozz::math::simd_float4::Load3PtrU( &v.X );
        }

        static Float_4 FromOzzSimdQuaternion( const ozz::math::SimdQuaternion &q )
        {
            Float_4 result{ };
            float   values[ 4 ];
            ozz::math::StorePtrU( q.xyzw, values );
            result.X = values[ 0 ];
            result.Y = values[ 1 ];
            result.Z = values[ 2 ];
            result.W = values[ 3 ];
            return result;
        }

        static ozz::math::Float4x4 ToOzzFloat4x4( const Float_4x4 &m )
        {
            ozz::math::Float4x4 result{ };
            result.cols[ 0 ] = ozz::math::simd_float4::LoadPtrU( &m._11 );
            result.cols[ 1 ] = ozz::math::simd_float4::LoadPtrU( &m._21 );
            result.cols[ 2 ] = ozz::math::simd_float4::LoadPtrU( &m._31 );
            result.cols[ 3 ] = ozz::math::simd_float4::LoadPtrU( &m._41 );
            return result;
        }

        static Float_4x4 FromOzzFloat4x4( const ozz::math::Float4x4 &m )
        {
            Float_4x4 result;
            float     col0[ 4 ], col1[ 4 ], col2[ 4 ], col3[ 4 ];

            ozz::math::StorePtrU( m.cols[ 0 ], col0 );
            ozz::math::StorePtrU( m.cols[ 1 ], col1 );
            ozz::math::StorePtrU( m.cols[ 2 ], col2 );
            ozz::math::StorePtrU( m.cols[ 3 ], col3 );

            result._11 = col0[ 0 ];
            result._12 = col0[ 1 ];
            result._13 = col0[ 2 ];
            result._14 = col0[ 3 ];
            result._21 = col1[ 0 ];
            result._22 = col1[ 1 ];
            result._23 = col1[ 2 ];
            result._24 = col1[ 3 ];
            result._31 = col2[ 0 ];
            result._32 = col2[ 1 ];
            result._33 = col2[ 2 ];
            result._34 = col2[ 3 ];
            result._41 = col3[ 0 ];
            result._42 = col3[ 1 ];
            result._43 = col3[ 2 ];
            result._44 = col3[ 3 ];

            return result;
        }
    } // namespace OzzUtils

    OzzAnimation::OzzAnimation( const SkeletonAsset *skeleton ) : m_impl( new Impl( skeleton ) )
    {
    }

    OzzAnimation::~OzzAnimation( )
    {
        delete m_impl;
    }

    OzzContext *OzzAnimation::NewContext( ) const
    {
        const auto context = new InternalContext( );
        if ( m_impl->skeleton )
        {
            context->samplingContext = ozz::make_unique<ozz::animation::SamplingJob::Context>( );
            context->samplingContext->Resize( m_impl->skeleton->num_joints( ) );
            context->localTransforms.resize( m_impl->skeleton->num_soa_joints( ) );
            context->modelTransforms.resize( m_impl->skeleton->num_joints( ) );
        }

        m_impl->contexts.push_back( context );
        return reinterpret_cast<OzzContext *>( context );
    }

    void OzzAnimation::DestroyContext( OzzContext *context ) const
    {
        if ( !context )
        {
            return;
        }

        const auto *internalContext = reinterpret_cast<InternalContext *>( context );
        const auto  it              = std::ranges::find( m_impl->contexts, internalContext );
        if ( it != m_impl->contexts.end( ) )
        {
            m_impl->contexts.erase( it );
            delete internalContext;
        }
    }

    void OzzAnimation::LoadAnimation( const AnimationAsset *animation, OzzContext *context ) const
    {
        if ( !animation || !context )
        {
            spdlog::error("Invalid animation or context");
            return;
        }

        auto *internalContext = reinterpret_cast<InternalContext *>( context );

        if ( animation->Animations.NumElements( ) == 0 )
        {
            spdlog::warn("Animation asset contains no animations");
            return;
        }

        // Use the first animation clip
        const AnimationClip &clip  = animation->Animations.GetElement( 0 );
        internalContext->animation = m_impl->ConvertToOzzAnimation( clip );

        if ( !internalContext->animation )
        {
            spdlog::error("Failed to convert animation");
        }
    }

    void OzzAnimation::UnloadAnimation( OzzContext *context )
    {
        if ( !context )
        {
            return;
        }

        auto *internalContext = reinterpret_cast<InternalContext *>( context );
        internalContext->animation.reset( );
    }

    void OzzAnimation::LoadTrack( const InteropArray<float> &keys, const float duration, OzzContext *context )
    {
        if ( !context || keys.NumElements( ) == 0 )
        {
            spdlog::error("Invalid context or empty keys");
            return;
        }

        auto *internalContext = reinterpret_cast<InternalContext *>( context );

        ozz::animation::offline::RawFloatTrack rawTrack;
        rawTrack.keyframes.resize( keys.NumElements( ) );

        const float step = duration / ( keys.NumElements( ) - 1 );
        for ( size_t i = 0; i < keys.NumElements( ); ++i )
        {
            auto &keyframe         = rawTrack.keyframes[ i ];
            keyframe.ratio         = i * step;
            keyframe.value         = keys.GetElement( i );
            keyframe.interpolation = ozz::animation::offline::RawTrackInterpolation::kLinear;
        }

        constexpr ozz::animation::offline::TrackBuilder builder;
        if ( auto track = builder( rawTrack ) )
        {
            internalContext->floatTracks.push_back( std::move( track ) );
        }
    }

    void OzzAnimation::LoadTrack( const InteropArray<Float_2> &keys, const InteropArray<float> &timestamps, OzzContext *context )
    {
        if ( !context || keys.NumElements( ) == 0 || timestamps.NumElements( ) != keys.NumElements( ) )
        {
            spdlog::error("Invalid context, empty keys, or mismatched timestamps");
            return;
        }

        auto *internalContext = reinterpret_cast<InternalContext *>( context );

        ozz::animation::offline::RawFloat2Track rawTrack;
        rawTrack.keyframes.resize( keys.NumElements( ) );

        for ( size_t i = 0; i < keys.NumElements( ); ++i )
        {
            const Float_2 &key      = keys.GetElement( i );
            auto          &keyframe = rawTrack.keyframes[ i ];
            keyframe.ratio          = timestamps.GetElement( i );
            keyframe.value          = ozz::math::Float2( key.X, key.Y );
            keyframe.interpolation  = ozz::animation::offline::RawTrackInterpolation::kLinear;
        }

        constexpr ozz::animation::offline::TrackBuilder builder;
        if ( auto track = builder( rawTrack ) )
        {
            internalContext->float2Tracks.push_back( std::move( track ) );
        }
    }

    void OzzAnimation::LoadTrack( const InteropArray<Float_3> &keys, const InteropArray<float> &timestamps, OzzContext *context )
    {
        if ( !context || keys.NumElements( ) == 0 || timestamps.NumElements( ) != keys.NumElements( ) )
        {
            spdlog::error("Invalid context, empty keys, or mismatched timestamps");
            return;
        }

        auto *internalContext = reinterpret_cast<InternalContext *>( context );

        ozz::animation::offline::RawFloat3Track rawTrack;
        rawTrack.keyframes.resize( keys.NumElements( ) );

        for ( size_t i = 0; i < keys.NumElements( ); ++i )
        {
            const Float_3 &key      = keys.GetElement( i );
            auto          &keyframe = rawTrack.keyframes[ i ];
            keyframe.ratio          = timestamps.GetElement( i );
            keyframe.value          = OzzUtils::ToOzzTranslation( key );
            keyframe.interpolation  = ozz::animation::offline::RawTrackInterpolation::kLinear;
        }

        constexpr ozz::animation::offline::TrackBuilder builder;
        if ( auto track = builder( rawTrack ) )
        {
            internalContext->float3Tracks.push_back( std::move( track ) );
        }
    }

    void OzzAnimation::LoadTrack( const InteropArray<Float_4> &keys, const InteropArray<float> &timestamps, OzzContext *context )
    {
        if ( !context || keys.NumElements( ) == 0 || timestamps.NumElements( ) != keys.NumElements( ) )
        {
            spdlog::error("Invalid context, empty keys, or mismatched timestamps");
            return;
        }

        auto *internalContext = reinterpret_cast<InternalContext *>( context );

        ozz::animation::offline::RawFloat4Track rawTrack;
        rawTrack.keyframes.resize( keys.NumElements( ) );

        for ( size_t i = 0; i < keys.NumElements( ); ++i )
        {
            const Float_4 &key      = keys.GetElement( i );
            auto          &keyframe = rawTrack.keyframes[ i ];
            keyframe.ratio          = timestamps.GetElement( i );
            keyframe.value          = ozz::math::Float4( key.X, key.Y, key.Z, key.W );
            keyframe.interpolation  = ozz::animation::offline::RawTrackInterpolation::kLinear;
        }

        constexpr ozz::animation::offline::TrackBuilder builder;
        if ( auto track = builder( rawTrack ) )
        {
            internalContext->float4Tracks.push_back( std::move( track ) );
        }
    }

    SamplingJobResult OzzAnimation::RunSamplingJob( const SamplingJobDesc &desc ) const
    {
        SamplingJobResult result{ };
        if ( !desc.Context )
        {
            spdlog::error("Invalid sampling job parameters");
            return result;
        }

        auto *internalContext = reinterpret_cast<InternalContext *>( desc.Context );
        if ( !internalContext->animation || !internalContext->samplingContext )
        {
            spdlog::error("No animation loaded in context or sampling context not initialized");
            return result;
        }

        const float ratio = ozz::math::Clamp( 0.f, desc.Ratio, 1.f );
        for ( size_t i = 0; i < internalContext->localTransforms.size( ); ++i )
        {
            internalContext->localTransforms[ i ] = m_impl->skeleton->joint_rest_poses( )[ i ];
        }

        ozz::animation::SamplingJob samplingJob;
        samplingJob.animation = internalContext->animation.get( );
        samplingJob.context   = internalContext->samplingContext.get( );
        samplingJob.ratio     = ratio;
        samplingJob.output    = ozz::make_span( internalContext->localTransforms );

        if ( !samplingJob.Run( ) )
        {
            spdlog::error("Animation sampling failed");
            return result;
        }

        ozz::animation::LocalToModelJob ltmJob;
        ltmJob.skeleton = m_impl->skeleton.get( );
        ltmJob.input    = ozz::make_span( internalContext->localTransforms );
        ltmJob.output   = ozz::make_span( internalContext->modelTransforms );

        if ( !ltmJob.Run( ) )
        {
            spdlog::error("Local to model transformation failed");
            return result;
        }

        using namespace DirectX;
        result.Transforms.Resize( internalContext->modelTransforms.size( ) );
        static const XMMATRIX correctionMatrix = XMMatrixRotationX( XM_PIDIV2 );
        for ( size_t i = 0; i < internalContext->modelTransforms.size( ); ++i )
        {
            const ozz::math::Float4x4 &ozzMat = internalContext->modelTransforms[ i ];
            Float_4x4                 &out    = result.Transforms.GetElement( i );
            ozz::math::Float3          ozzTranslation;
            ozz::math::Quaternion      ozzQuat;
            ozz::math::Float3          ozzScale;

            if ( ozz::math::ToAffine( ozzMat, &ozzTranslation, &ozzQuat, &ozzScale ) )
            {
                const Float_3 translation = OzzUtils::FromOzzTranslation( ozzTranslation );
                const Float_4 rotation    = OzzUtils::FromOzzRotation( ozzQuat );
                const Float_3 scale       = OzzUtils::FromOzzScale( ozzScale );

                XMMATRIX xmOut =
                    XMMatrixAffineTransformation( XMVectorSet( scale.X, scale.Y, scale.Z, 1.0f ), XMVectorZero( ), XMVectorSet( rotation.X, rotation.Y, rotation.Z, rotation.W ),
                                                  XMVectorSet( translation.X, translation.Y, translation.Z, 1.0f ) );

                xmOut = XMMatrixMultiply( xmOut, correctionMatrix );
                out   = InteropMathConverter::Float_4X4FromXMMATRIX( xmOut );
            }
        }

        result.Success = true;
        return result;
    }

    BlendingJobResult OzzAnimation::RunBlendingJob( const BlendingJobDesc &desc ) const
    {
        BlendingJobResult result{ };
        if ( !desc.Context || desc.Layers.NumElements( ) == 0 )
        {
            spdlog::error("Invalid blending job parameters");
            return result;
        }

        auto *internalContext = reinterpret_cast<InternalContext *>( desc.Context );

        const size_t                                      numLayers = desc.Layers.NumElements( );
        std::vector<ozz::animation::BlendingJob::Layer>   ozzLayers( numLayers );
        std::vector<ozz::vector<ozz::math::SoaTransform>> layerTransforms( numLayers );

        const size_t numSoaJoints = m_impl->skeleton->num_soa_joints( );
        for ( size_t i = 0; i < numLayers; ++i )
        {
            const auto &layer = desc.Layers.GetElement( i );
            if ( layer.Transforms.NumElements( ) == 0 )
            {
                spdlog::error("Invalid transforms in layer {}", i);
                return result;
            }

            layerTransforms[ i ].resize( numSoaJoints );

            // TODO: Convert from Float_4x4 to SoaTransform
            // This is simplified - proper implementation would need to convert the transforms correctly
            for ( size_t j = 0; j < numSoaJoints; ++j )
            {
                layerTransforms[ i ][ j ] = m_impl->skeleton->joint_rest_poses( )[ j ];
            }

            ozzLayers[ i ].transform = ozz::make_span( layerTransforms[ i ] );
            ozzLayers[ i ].weight    = layer.Weight;
        }

        ozz::vector<ozz::math::SoaTransform> output( numSoaJoints );

        ozz::animation::BlendingJob blendingJob;
        blendingJob.threshold = desc.Threshold;
        blendingJob.rest_pose = m_impl->skeleton->joint_rest_poses( );
        blendingJob.layers    = ozz::make_span( ozzLayers );
        blendingJob.output    = ozz::make_span( output );

        if ( !blendingJob.Run( ) )
        {
            spdlog::error("Blending job failed");
            return result;
        }

        ozz::animation::LocalToModelJob ltmJob;
        ltmJob.skeleton = m_impl->skeleton.get( );
        ltmJob.input    = ozz::make_span( output );
        ltmJob.output   = ozz::make_span( internalContext->modelTransforms );

        if ( !ltmJob.Run( ) )
        {
            spdlog::error("Local to model transformation failed after blending");
            return result;
        }

        result.Success = true;
        OzzUtils::CopyOzzVectorToArray( internalContext->modelTransforms, result.Transforms );
        return result;
    }

    LocalToModelJobResult OzzAnimation::RunLocalToModelJob( const LocalToModelJobDesc &desc ) const
    {
        LocalToModelJobResult result{ };
        if ( !desc.Context )
        {
            spdlog::error("Invalid local to model job parameters");
            return result;
        }

        auto *internalContext = reinterpret_cast<InternalContext *>( desc.Context );

        // TODO: This is a simplified implementation
        // A proper implementation would convert from Float_4x4 to SoaTransform and back

        ozz::animation::LocalToModelJob ltmJob;
        ltmJob.skeleton = m_impl->skeleton.get( );

        // Use local transforms directly from the skeleton
        ltmJob.input  = m_impl->skeleton->joint_rest_poses( );
        ltmJob.output = ozz::make_span( internalContext->modelTransforms );

        if ( !ltmJob.Run( ) )
        {
            spdlog::error("Local to model transformation failed");
            return result;
        }

        // Copy to output
        result.Success = true;
        OzzUtils::CopyOzzVectorToArray( internalContext->modelTransforms, result.Transforms );
        return result;
    }

    SkinningJobResult OzzAnimation::RunSkinningJob( const SkinningJobDesc &desc )
    {
        SkinningJobResult result{ };
        if ( !desc.Context || desc.JointTransforms.NumElements( ) == 0 || desc.Vertices.NumElements( ) == 0 || desc.Weights.NumElements( ) == 0 ||
             !desc.Indices.NumElements( ) == 0 || desc.InfluenceCount <= 0 )
        {
            spdlog::error("Invalid skinning job parameters");
            return result;
        }

        const size_t                     numJoints = desc.JointTransforms.NumElements( );
        ozz::vector<ozz::math::Float4x4> jointMatrices( numJoints );

        for ( size_t i = 0; i < numJoints; ++i )
        {
            jointMatrices[ i ] = OzzUtils::ToOzzFloat4x4( desc.JointTransforms.GetElement( i ) );
        }

        const uint32_t numVertices = desc.Vertices.NumElements( );

        ozz::geometry::SkinningJob skinningJob;
        skinningJob.vertex_count     = desc.Vertices.NumElements( );
        skinningJob.influences_count = desc.InfluenceCount;
        skinningJob.joint_matrices   = ozz::make_span( jointMatrices );

        skinningJob.in_positions  = ozz::span( desc.Vertices.Data( ), numVertices * 3 );
        skinningJob.joint_weights = ozz::span( desc.Weights.Data( ), numVertices * desc.InfluenceCount );
        skinningJob.joint_indices = ozz::span( desc.Indices.Data( ), numVertices * desc.InfluenceCount );

        result.Vertices.Resize( numVertices * 3 );
        skinningJob.out_positions = ozz::span( result.Vertices.Data( ), numVertices * 3 ); // Todo validate result

        if ( !skinningJob.Run( ) )
        {
            spdlog::error("Skinning job failed");
            return result;
        }

        result.Success = true;
        return result;
    }

    IkTwoBoneJobResult OzzAnimation::RunIkTwoBoneJob( const IkTwoBoneJobDesc &desc )
    {
        IkTwoBoneJobResult result{ };

        const ozz::math::Float4x4 startMatrix = OzzUtils::ToOzzFloat4x4( desc.StartJointMatrix );
        const ozz::math::Float4x4 midMatrix   = OzzUtils::ToOzzFloat4x4( desc.MidJointMatrix );
        const ozz::math::Float4x4 endMatrix   = OzzUtils::ToOzzFloat4x4( desc.EndJointMatrix );

        ozz::animation::IKTwoBoneJob ozzJob;

        ozzJob.target      = OzzUtils::ToOzzSimdFloat4( desc.Target );
        ozzJob.pole_vector = OzzUtils::ToOzzSimdFloat4( desc.PoleVector );
        ozzJob.mid_axis    = OzzUtils::ToOzzSimdFloat4( desc.MidAxis );
        ozzJob.twist_angle = desc.TwistAngle;
        ozzJob.soften      = desc.Soften;
        ozzJob.weight      = desc.Weight;

        ozzJob.start_joint = &startMatrix;
        ozzJob.mid_joint   = &midMatrix;
        ozzJob.end_joint   = &endMatrix;

        ozz::math::SimdQuaternion startCorrection{ };
        ozz::math::SimdQuaternion midCorrection{ };
        bool                      targetReached = false;

        ozzJob.start_joint_correction = &startCorrection;
        ozzJob.mid_joint_correction   = &midCorrection;
        ozzJob.reached                = &targetReached;

        if ( !ozzJob.Validate( ) )
        {
            spdlog::error("IKTwoBoneJob: Validation failed");
            return result;
        }

        if ( !ozzJob.Run( ) )
        {
            spdlog::error("IKTwoBoneJob: Execution failed");
            return result;
        }

        result.Success              = true;
        result.StartJointCorrection = OzzUtils::FromOzzSimdQuaternion( startCorrection );
        result.MidJointCorrection   = OzzUtils::FromOzzSimdQuaternion( midCorrection );
        result.Reached              = targetReached;

        return result;
    }

    IkAimJobResult OzzAnimation::RunIkAimJob( const IkAimJobDesc &desc ) const
    {
        IkAimJobResult result{ };
        if ( !desc.Context || desc.JointIndex < 0 )
        {
            spdlog::error("Invalid IK aim job parameters");
            return result;
        }

        if ( !m_impl->skeleton || desc.JointIndex >= m_impl->skeleton->num_joints( ) )
        {
            spdlog::error("Invalid joint index or skeleton not initialized");
            return result;
        }

        ozz::animation::IKAimJob ozzJob;
        ozzJob.up      = OzzUtils::ToOzzSimdFloat4( desc.Up );
        ozzJob.forward = OzzUtils::ToOzzSimdFloat4( desc.Forward );
        ozzJob.target  = OzzUtils::ToOzzSimdFloat4( desc.Target );
        ozzJob.weight  = desc.Weight;

        const ozz::math::Float4x4 jointMatrix = ozz::math::Float4x4::identity( );
        ozzJob.joint                          = &jointMatrix;

        ozz::math::SimdQuaternion jointCorrection{ };
        ozzJob.joint_correction = &jointCorrection;

        if ( !ozzJob.Validate( ) )
        {
            spdlog::error("IKAimJob: Validation failed");
            return result;
        }

        if ( !ozzJob.Run( ) )
        {
            spdlog::error("IKAimJob: Execution failed");
            return result;
        }

        result.Success         = true;
        result.JointCorrection = OzzUtils::FromOzzSimdQuaternion( jointCorrection );
        return result;
    }

    TrackSamplingResult OzzAnimation::RunTrackSamplingJob( const TrackSamplingJobDesc &desc )
    {
        TrackSamplingResult result{ };
        result.Type = desc.Type; // Required?
        if ( !desc.Context || desc.TrackIndex < 0 )
        {
            spdlog::error("Invalid track sampling job parameters");
            return result;
        }

        const auto *internalContext = reinterpret_cast<InternalContext *>( desc.Context );
        const float ratio           = ozz::math::Clamp( 0.f, desc.Ratio, 1.f );
        bool        success         = false;

        switch ( desc.Type )
        {
        case TrackSamplingResultType::Float:
            {
                if ( desc.TrackIndex >= static_cast<int>( internalContext->floatTracks.size( ) ) )
                {
                    spdlog::error("Float track index out of range");
                    return result;
                }
                ozz::animation::FloatTrackSamplingJob job;
                job.track  = internalContext->floatTracks[ desc.TrackIndex ].get( );
                job.ratio  = ratio;
                job.result = &result.FloatValue;
                success    = job.Run( );
                break;
            }

        case TrackSamplingResultType::Float2:
            {
                if ( desc.TrackIndex >= static_cast<int>( internalContext->float2Tracks.size( ) ) )
                {
                    spdlog::error("Float2 track index out of range");
                    return result;
                }

                ozz::animation::Float2TrackSamplingJob job;
                job.track = internalContext->float2Tracks[ desc.TrackIndex ].get( );
                job.ratio = ratio;
                ozz::math::Float2 float2Value;
                job.result = &float2Value;
                success    = job.Run( );
                if ( success )
                {
                    result.Float2Value.X = float2Value.x;
                    result.Float2Value.Y = float2Value.y;
                }
                break;
            }

        case TrackSamplingResultType::Float3:
            {
                if ( desc.TrackIndex >= static_cast<int>( internalContext->float3Tracks.size( ) ) )
                {
                    spdlog::error("Float3 track index out of range");
                    return result;
                }

                ozz::animation::Float3TrackSamplingJob job;
                job.track = internalContext->float3Tracks[ desc.TrackIndex ].get( );
                job.ratio = ratio;
                ozz::math::Float3 float3Value;
                job.result = &float3Value;
                success    = job.Run( );
                if ( success )
                {
                    result.Float3Value.X = float3Value.x;
                    result.Float3Value.Y = float3Value.y;
                    result.Float3Value.Z = float3Value.z;
                }
                break;
            }

        case TrackSamplingResultType::Float4:
            {
                if ( desc.TrackIndex >= static_cast<int>( internalContext->float4Tracks.size( ) ) )
                {
                    spdlog::error("Float4 track index out of range");
                    return result;
                }

                ozz::animation::Float4TrackSamplingJob job;
                job.track = internalContext->float4Tracks[ desc.TrackIndex ].get( );
                job.ratio = ratio;
                ozz::math::Float4 float4Value;
                job.result = &float4Value;
                success    = job.Run( );
                if ( success )
                {
                    result.Float4Value.X = float4Value.x;
                    result.Float4Value.Y = float4Value.y;
                    result.Float4Value.Z = float4Value.z;
                    result.Float4Value.W = float4Value.w;
                }
                break;
            }

        case TrackSamplingResultType::Quaternion:
            {
                if ( desc.TrackIndex >= static_cast<int>( internalContext->quaternionTracks.size( ) ) )
                {
                    spdlog::error("Quaternion track index out of range");
                    return result;
                }

                ozz::animation::QuaternionTrackSamplingJob job;
                job.track = internalContext->quaternionTracks[ desc.TrackIndex ].get( );
                job.ratio = ratio;
                ozz::math::Quaternion quaternionResult;
                job.result = &quaternionResult;
                success    = job.Run( );
                if ( success )
                {
                    result.QuaternionValue.X = quaternionResult.x;
                    result.QuaternionValue.Y = quaternionResult.y;
                    result.QuaternionValue.Z = quaternionResult.z;
                    result.QuaternionValue.W = quaternionResult.w;
                }
                break;
            }
        default:
            spdlog::error("Unsupported track value type");
            return result;
        }

        if ( !success )
        {
            spdlog::error("Track sampling failed");
            return result;
        }

        result.Success = true;
        return result;
    }

    TrackTriggeringResult OzzAnimation::RunTrackTriggeringJob( const TrackTriggeringJobDesc &desc )
    {
        TrackTriggeringResult result{ };
        if ( !desc.Context || desc.TrackIndex < 0 )
        {
            spdlog::error("Invalid track triggering job parameters");
            return result;
        }

        const auto *internalContext = reinterpret_cast<InternalContext *>( desc.Context );
        if ( desc.TrackIndex >= static_cast<int>( internalContext->floatTracks.size( ) ) )
        {
            spdlog::error("Track index out of range");
            return result;
        }

        ozz::animation::TrackTriggeringJob job;
        job.track     = internalContext->floatTracks[ desc.TrackIndex ].get( );
        job.from      = desc.PreviousRatio;
        job.to        = desc.Ratio;
        job.threshold = 0.5f; // Todo configure?
        job.iterator  = nullptr;

        if ( !job.Run( ) )
        {
            spdlog::error("Track triggering failed");
            return result;
        }

        size_t edgeCount = 0;
        auto   it        = job.iterator;
        while ( it && *it != job.end( ) )
        {
            ++edgeCount;
            ++it;
        }

        result.Triggered.Resize( edgeCount );
        edgeCount = 0;
        it        = job.iterator;
        while ( it && *it != job.end( ) )
        {
            const auto &edge = *it;
            result.Triggered.SetElement( edgeCount, edge->ratio );
            ++edgeCount;
            ++it;
        }

        delete job.iterator;
        result.Success = true;
        return result;
    }

    void OzzAnimation::GetJointNames( InteropArray<InteropString> &outNames ) const
    {
        if ( !m_impl->skeleton )
        {
            spdlog::error("Skeleton not initialized");
            return;
        }

        const int numJoints = m_impl->skeleton->num_joints( );
        outNames.Resize( numJoints );

        for ( int i = 0; i < numJoints; ++i )
        {
            outNames.GetElement( i ) = InteropString( m_impl->skeleton->joint_names( )[ i ] );
        }
    }

    int OzzAnimation::GetJointCount( ) const
    {
        return m_impl->skeleton ? m_impl->skeleton->num_joints( ) : 0;
    }

    float OzzAnimation::GetAnimationDuration( OzzContext *context )
    {
        if ( !context )
        {
            return 0.0f;
        }

        const auto *internalContext = reinterpret_cast<InternalContext *>( context );
        return internalContext->animation ? internalContext->animation->duration( ) : 0.0f;
    }

} // namespace DenOfIz
