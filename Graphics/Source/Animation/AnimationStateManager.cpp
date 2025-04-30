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

#include <ozz/base/maths/soa_quaternion.h>
#include <ozz/base/maths/soa_transform.h>
#include <ozz/base/maths/vec_float.h>
#include <ozz/base/span.h>
#include <ozz/animation/offline/animation_builder.h>
#include <ozz/animation/offline/raw_animation.h>
#include <ozz/animation/offline/raw_skeleton.h>
#include <ozz/animation/offline/skeleton_builder.h>
#include <ozz/animation/runtime/local_to_model_job.h>

#include <DenOfIzGraphics/Animation/AnimationStateManager.h>
#include <functional>
#include <glog/logging.h>

#include "DenOfIzGraphics/Utilities/InteropMathConverter.h"

using ozz::make_span;
using ozz::span;

using namespace DenOfIz;
using namespace Internal;

AnimationStateManager::AnimationStateManager( const AnimationStateManagerDesc &desc )
{
    if ( !desc.Skeleton )
    {
        LOG( ERROR ) << "Skeleton is required for AnimationStateManager";
        return;
    }

    m_skeleton = ozz::make_unique<ozz::animation::Skeleton>( );

    const auto  &joints    = desc.Skeleton->Joints;
    const size_t numJoints = joints.NumElements( );

    ozz::vector<int16_t> parents( numJoints );

    ozz::animation::offline::RawSkeleton rawSkeleton;
    for ( size_t i = 0; i < numJoints; ++i )
    {
        const Joint &joint = joints.GetElement( i );
        parents[ i ]       = joint.ParentIndex;
        // if ( strcmp( joint.Name.Get( ), "b_Root_00") == 0 )
        if ( joint.ParentIndex == -1 )
        {
            ozz::animation::offline::RawSkeleton::Joint rootJoint;
            rootJoint.name      = joint.Name.Get( );
            rootJoint.transform = GetJointLocalTransform( joint );

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
            childJoint.transform = GetJointLocalTransform( joint );

            parentJoint->children.push_back( childJoint );
        }
    }
    ozz::animation::offline::SkeletonBuilder builder;
    if ( auto builtSkeleton = builder( rawSkeleton ) )
    {
        m_skeleton.reset( );
        m_skeleton = std::move( builtSkeleton );

        const size_t numSoaJoints = m_skeleton->num_soa_joints( );
        m_localTransforms.resize( numSoaJoints );
        m_modelTransforms.resize( m_skeleton->num_joints( ) );
    }
    else
    {
        LOG( ERROR ) << "Failed to build ozz skeleton";
    }
}

AnimationStateManager::~AnimationStateManager( )
{
    m_animations.clear( );
    m_localTransforms.clear( );
    m_modelTransforms.clear( );
}

void AnimationStateManager::AddAnimation( const AnimationAsset &animationAsset )
{
    for ( size_t i = 0; i < animationAsset.Animations.NumElements( ); ++i )
    {
        const AnimationClip &clip     = animationAsset.Animations.GetElement( i );
        std::string          animName = clip.Name.Get( );

        if ( animName.empty( ) )
        {
            animName = "Animation_" + std::to_string( i );
        }

        auto ozzAnimation = ConvertToOzzAnimation( clip );

        AnimationState state;
        state.Name         = animName;
        state.OzzAnimation = std::move( ozzAnimation );
        state.Loop         = true;
        state.Playing      = false;

        m_animations[ animName ] = std::move( state );

        LOG( INFO ) << "Added animation '" << animName << "' with duration " << clip.Duration << "s";
    }
    if ( m_currentAnimation.empty( ) && !m_animations.empty( ) )
    {
        m_currentAnimation = m_animations.begin( )->first;
        LOG( INFO ) << "Set default animation to '" << m_currentAnimation << "'";
    }
}

void AnimationStateManager::Play( const std::string &animationName, const bool loop )
{
    if ( !HasAnimation( animationName ) )
    {
        LOG( ERROR ) << "Animation '" << animationName << "' not found";
        return;
    }

    m_blendingState.InProgress = false;

    auto &prevAnim   = m_animations[ m_currentAnimation ];
    prevAnim.Playing = false;

    m_currentAnimation = animationName;
    auto &newAnim      = m_animations[ m_currentAnimation ];

    newAnim.Loop        = loop;
    newAnim.Playing     = true;
    newAnim.CurrentTime = 0.0f;

    LOG( INFO ) << "Playing animation '" << animationName << "'" << ( loop ? " (looping)" : "" );
}

void AnimationStateManager::BlendTo( const std::string &animationName, const float blendTime )
{
    if ( !HasAnimation( animationName ) )
    {
        LOG( ERROR ) << "Animation '" << animationName << "' not found";
        return;
    }

    if ( m_currentAnimation == animationName )
    {
        return;
    }

    m_blendingState.SourceAnimation  = m_currentAnimation;
    m_blendingState.TargetAnimation  = animationName;
    m_blendingState.BlendTime        = blendTime;
    m_blendingState.CurrentBlendTime = 0.0f;
    m_blendingState.InProgress       = true;

    auto &targetAnim       = m_animations[ animationName ];
    targetAnim.Weight      = 0.0f;
    targetAnim.Playing     = true;
    targetAnim.CurrentTime = 0.0f;

    LOG( INFO ) << "Blending from '" << m_currentAnimation << "' to '" << animationName << "' over " << blendTime << "s";
}

void AnimationStateManager::Stop( )
{
    if ( !m_currentAnimation.empty( ) )
    {
        auto &anim       = m_animations[ m_currentAnimation ];
        anim.Playing     = false;
        anim.CurrentTime = 0.0f;
    }

    m_blendingState.InProgress = false;
}

void AnimationStateManager::Pause( )
{
    if ( !m_currentAnimation.empty( ) )
    {
        auto &anim   = m_animations[ m_currentAnimation ];
        anim.Playing = false;
    }
}

void AnimationStateManager::Resume( )
{
    if ( !m_currentAnimation.empty( ) )
    {
        auto &anim   = m_animations[ m_currentAnimation ];
        anim.Playing = true;
    }
}

void AnimationStateManager::Update( const float deltaTime )
{
    if ( m_currentAnimation.empty( ) || !m_animations[ m_currentAnimation ].Playing )
    {
        return;
    }

    UpdateBlending( deltaTime );
    if ( auto &anim = m_animations[ m_currentAnimation ]; anim.Playing )
    {
        const ozz::animation::Animation *ozzAnim = anim.OzzAnimation.get( );
        if ( !ozzAnim )
        {
            return;
        }

        anim.CurrentTime += deltaTime * anim.PlaybackSpeed;
        if ( const float duration = ozzAnim->duration( ); anim.CurrentTime > duration )
        {
            if ( anim.Loop )
            {
                anim.CurrentTime = fmodf( anim.CurrentTime, duration );
            }
            else
            {
                anim.CurrentTime = duration;
                anim.Playing     = false;
            }
        }

        m_localTransforms.resize( m_skeleton->num_soa_joints( ) );
        for ( auto &transform : m_localTransforms )
        {
            transform = ozz::math::SoaTransform::identity( );
        }
        SampleAnimation( anim, m_localTransforms );
        ozz::animation::LocalToModelJob ltmJob;
        ltmJob.skeleton = m_skeleton.get( );
        ltmJob.input    = make_span( m_localTransforms );
        ltmJob.output   = make_span( m_modelTransforms );
        if ( !ltmJob.Run( ) )
        {
            LOG( ERROR ) << "Failed to convert local to model space transforms";
        }
    }
}

bool AnimationStateManager::HasAnimation( const std::string &animationName ) const
{
    return m_animations.contains( animationName );
}

void AnimationStateManager::GetModelSpaceTransforms( InteropArray<Float_4x4> &outTransforms ) const
{
    using namespace DirectX;

    outTransforms.Clear( );
    const size_t numTransforms = m_modelTransforms.size( );
    outTransforms.Resize( numTransforms );

    static const XMMATRIX correctionMatrix = XMMatrixRotationX( XM_PIDIV2 );
    for ( size_t i = 0; i < numTransforms; ++i )
    {
        const ozz::math::Float4x4 &ozzMat = m_modelTransforms[ i ];
        Float_4x4                 &out    = outTransforms.GetElement( i );
        ozz::math::Float3          ozzTranslation;
        ozz::math::Quaternion      ozzQuat;
        ozz::math::Float3          ozzScale;
        // Decomposing gives us a sure way of building a matrix due to differences in matrix layout
        if ( ozz::math::ToAffine( ozzMat, &ozzTranslation, &ozzQuat, &ozzScale ) )
        {
            const Float_3 translation = FromOzzTranslation( ozzTranslation );
            const Float_4 rotation    = FromOzzRotation( ozzQuat );
            const Float_3 scale       = FromOzzScale( ozzScale );

            // clang-format off
            XMMATRIX xmOut = XMMatrixAffineTransformation(
                XMVectorSet( scale.X, scale.Y, scale.Z, 1.0f ),
                XMVectorZero(),
                XMVectorSet( rotation.X, rotation.Y, rotation.Z, rotation.W ),
                XMVectorSet( translation.X, translation.Y, translation.Z, 1.0f )
            );
            // clang-format on
            xmOut = XMMatrixMultiply( xmOut, correctionMatrix );
            out   = InteropMathConverter::Float_4X4FromXMMATRIX( xmOut );
        }
    }
}

const std::string &AnimationStateManager::GetCurrentAnimationName( ) const
{
    return m_currentAnimation;
}

int AnimationStateManager::GetNumJoints( ) const
{
    return m_skeleton ? m_skeleton->num_joints( ) : 0;
}

std::unique_ptr<ozz::animation::Animation, ozz::Deleter<ozz::animation::Animation>> AnimationStateManager::ConvertToOzzAnimation( const AnimationClip &clip )
{
    auto ozzAnim = std::make_unique<ozz::animation::Animation>( );

    const float  duration  = clip.Duration;
    const size_t numJoints = m_skeleton->num_joints( );

    std::unordered_map<std::string, int> jointNameToIndexMap;
    for ( size_t i = 0; i < numJoints; ++i )
    {
        jointNameToIndexMap[ m_skeleton->joint_names( )[ i ] ] = i;
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
            LOG( WARNING ) << "Animation track for joint '" << jointName << "' has no corresponding joint in skeleton (or index " << jointIndex << " is out of range)";
            continue;
        }

        ozz::animation::offline::RawAnimation::JointTrack &rawTrack = rawAnimation.tracks[ jointIndex ];

        const size_t numPosKeys = track.PositionKeys.NumElements( );
        for ( size_t j = 0; j < numPosKeys; ++j )
        {
            const PositionKey                                    &key = track.PositionKeys.GetElement( j );
            ozz::animation::offline::RawAnimation::TranslationKey rawKey;
            rawKey.time  = key.Timestamp;
            rawKey.value = ToOzzTranslation( key.Value );
            rawTrack.translations.push_back( rawKey );
        }

        const size_t numRotKeys = track.RotationKeys.NumElements( );
        for ( size_t j = 0; j < numRotKeys; ++j )
        {
            const RotationKey                                 &key = track.RotationKeys.GetElement( j );
            ozz::animation::offline::RawAnimation::RotationKey rawKey;
            rawKey.time  = key.Timestamp;
            rawKey.value = ToOzzRotation( key.Value );
            rawTrack.rotations.push_back( rawKey );
        }

        const size_t numScaleKeys = track.ScaleKeys.NumElements( );
        for ( size_t j = 0; j < numScaleKeys; ++j )
        {
            const ScaleKey                                 &key = track.ScaleKeys.GetElement( j );
            ozz::animation::offline::RawAnimation::ScaleKey rawKey;
            rawKey.time  = key.Timestamp;
            rawKey.value = ToOzzScale( key.Value );
            rawTrack.scales.push_back( rawKey );
        }
    }

    constexpr ozz::animation::offline::AnimationBuilder builder;
    auto                                                built = builder( rawAnimation );
    if ( !built )
    {
        LOG( ERROR ) << "Failed to build ozz animation";
        return nullptr;
    }

    return built;
}

void AnimationStateManager::SampleAnimation( AnimationState &state, ozz::vector<ozz::math::SoaTransform> &output ) const
{
    const ozz::animation::Animation *animation = state.OzzAnimation.get( );
    if ( !animation )
    {
        return;
    }
    if ( output.size( ) != m_skeleton->num_soa_joints( ) )
    {
        output.resize( m_skeleton->num_soa_joints( ) );
    }
    for ( size_t i = 0; i < output.size( ); ++i )
    {
        output[ i ] = m_skeleton->joint_rest_poses( )[ i ];
    }

    float ratio = state.CurrentTime / animation->duration( );
    ratio       = ozz::math::Clamp( 0.f, ratio, 1.f );

    if ( !state.Context )
    {
        state.Context = std::make_unique<ozz::animation::SamplingJob::Context>( );
        state.Context->Resize( m_skeleton->num_joints( ) );
    }

    ozz::animation::SamplingJob samplingJob;
    samplingJob.animation = animation;
    samplingJob.ratio     = ratio;
    samplingJob.output    = make_span( output );
    samplingJob.context   = state.Context.get( );

    if ( !samplingJob.Run( ) )
    {
        LOG( ERROR ) << "Animation sampling failed for animation '" << state.Name << "'";
        for ( size_t i = 0; i < output.size( ); ++i )
        {
            output[ i ] = m_skeleton->joint_rest_poses( )[ i ];
        }
    }
}

void AnimationStateManager::UpdateBlending( const float deltaTime )
{
    if ( !m_blendingState.InProgress )
    {
        return;
    }

    m_blendingState.CurrentBlendTime += deltaTime;

    const float blendFactor = m_blendingState.CurrentBlendTime / m_blendingState.BlendTime;
    if ( blendFactor >= 1.0f )
    {
        m_blendingState.InProgress = false;
        m_currentAnimation         = m_blendingState.TargetAnimation;

        for ( auto &[ name, anim ] : m_animations )
        {
            anim.Weight  = name == m_currentAnimation ? 1.0f : 0.0f;
            anim.Playing = name == m_currentAnimation;
        }
        return;
    }

    auto &sourceAnim = m_animations[ m_blendingState.SourceAnimation ];
    auto &targetAnim = m_animations[ m_blendingState.TargetAnimation ];

    sourceAnim.Weight = 1.0f - blendFactor;
    targetAnim.Weight = blendFactor;

    ozz::vector<ozz::math::SoaTransform> sourceTransforms;
    ozz::vector<ozz::math::SoaTransform> targetTransforms;

    sourceTransforms.resize( m_skeleton->num_soa_joints( ) );
    targetTransforms.resize( m_skeleton->num_soa_joints( ) );
    m_localTransforms.resize( m_skeleton->num_soa_joints( ) );

    SampleAnimation( sourceAnim, sourceTransforms );
    SampleAnimation( targetAnim, targetTransforms );

    ozz::animation::BlendingJob blendingJob;
    blendingJob.threshold = 0.1f;

    ozz::animation::BlendingJob::Layer layers[ 2 ];
    layers[ 0 ].transform = make_span( sourceTransforms );
    layers[ 0 ].weight    = sourceAnim.Weight;
    layers[ 1 ].transform = make_span( targetTransforms );
    layers[ 1 ].weight    = targetAnim.Weight;

    blendingJob.layers    = layers;
    blendingJob.rest_pose = m_skeleton->joint_rest_poses( );
    blendingJob.output    = make_span( m_localTransforms );

    if ( !blendingJob.Run( ) )
    {
        LOG( ERROR ) << "Animation blending failed";
        return;
    }

    ozz::animation::LocalToModelJob localToModelJob;
    localToModelJob.skeleton = m_skeleton.get( );
    localToModelJob.input    = make_span( m_localTransforms );
    localToModelJob.output   = make_span( m_modelTransforms );

    if ( !localToModelJob.Run( ) )
    {
        LOG( ERROR ) << "Failed to convert blended transforms to model space";
    }
}

ozz::math::Transform AnimationStateManager::GetJointLocalTransform( const Joint &joint )
{
    ozz::math::Transform transform;
    transform.translation = ToOzzTranslation( joint.LocalTranslation );
    transform.rotation    = ToOzzRotation( joint.LocalRotationQuat );
    transform.scale       = ToOzzScale( joint.LocalScale );
    return transform;
}

ozz::math::Float3 AnimationStateManager::ToOzzTranslation( const Float_3 &translation )
{
    return ozz::math::Float3( translation.X, translation.Y, -translation.Z );
}

ozz::math::Quaternion AnimationStateManager::ToOzzRotation( const Float_4 &rotation )
{
    return ozz::math::Quaternion( -rotation.X, -rotation.Y, rotation.Z, rotation.W );
}

ozz::math::Float3 AnimationStateManager::ToOzzScale( const Float_3 &scale )
{
    return ozz::math::Float3( scale.X, scale.Y, scale.Z );
}

Float_3 AnimationStateManager::FromOzzTranslation( const ozz::math::Float3 &translation )
{
    return Float_3{ translation.x, translation.y, -translation.z };
}

Float_4 AnimationStateManager::FromOzzRotation( const ozz::math::Quaternion &rotation )
{
    return Float_4{ -rotation.x, -rotation.y, rotation.z, rotation.w };
}
Float_3 AnimationStateManager::FromOzzScale( const ozz::math::Float3 &scale )
{
    return Float_3{ scale.x, scale.y, scale.z };
}
