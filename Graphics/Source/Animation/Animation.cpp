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

#include <DenOfIzGraphics/Animation/Animation.h>
#include <DenOfIzGraphics/Animation/Skeleton.h>
#include <glog/logging.h>

#include <ozz/animation/offline/animation_builder.h>
#include <ozz/animation/offline/raw_animation.h>
#include <ozz/animation/runtime/animation.h>
#include <ozz/base/memory/unique_ptr.h>

namespace DenOfIz
{
    class Animation::Impl
    {
    public:
        std::unique_ptr<ozz::animation::Animation, ozz::Deleter<ozz::animation::Animation>> ozzAnimation;
        InteropString                                                                       name;
        Skeleton                                                                           *skeleton;

        static ozz::math::Float3 ToOzzTranslation( const Float_3 &translation )
        {
            return ozz::math::Float3( translation.X, translation.Y, -translation.Z );
        }

        static ozz::math::Quaternion ToOzzRotation( const Float_4 &rotation )
        {
            return ozz::math::Quaternion( -rotation.X, -rotation.Y, rotation.Z, rotation.W );
        }

        static ozz::math::Float3 ToOzzScale( const Float_3 &scale )
        {
            return ozz::math::Float3( scale.X, scale.Y, scale.Z );
        }
    };

    Animation::Animation( const AnimationAsset &animationAsset, Skeleton *skeleton ) : m_impl( new Impl( ) )
    {
        m_impl->skeleton = skeleton;
        if ( animationAsset.Animations.NumElements( ) > 0 )
        {
            const AnimationClip &firstClip = animationAsset.Animations.GetElement( 0 );
            *this                          = Animation( firstClip, skeleton );
        }
        else
        {
            LOG( ERROR ) << "Animation asset has no clips";
        }
    }

    Animation::Animation( const AnimationClip &clip, Skeleton *skeleton ) : m_impl( new Impl( ) )
    {
        m_impl->skeleton = skeleton;
        m_impl->name     = clip.Name;

        if ( m_impl->name.Get( )[ 0 ] == '\0' )
        {
            m_impl->name = InteropString( "Unnamed_Animation" );
        }

        const float duration  = clip.Duration;
        const int   numJoints = skeleton->GetNumJoints( );

        ozz::animation::offline::RawAnimation rawAnimation;
        rawAnimation.duration = duration;
        rawAnimation.tracks.resize( numJoints );

        for ( size_t i = 0; i < clip.Tracks.NumElements( ); ++i )
        {
            const JointAnimTrack &track      = clip.Tracks.GetElement( i );
            const std::string     jointName  = track.JointName.Get( );
            const int             jointIndex = skeleton->GetJointIndex( track.JointName );

            if ( jointIndex < 0 || jointIndex >= numJoints )
            {
                LOG( WARNING ) << "Animation track for joint '" << jointName << "' has no corresponding joint in skeleton (or index " << jointIndex << " is out of range)";
                continue;
            }

            ozz::animation::offline::RawAnimation::JointTrack &rawTrack = rawAnimation.tracks[ jointIndex ];

            // Process position keys
            const size_t numPosKeys = track.PositionKeys.NumElements( );
            for ( size_t j = 0; j < numPosKeys; ++j )
            {
                const PositionKey                                    &key = track.PositionKeys.GetElement( j );
                ozz::animation::offline::RawAnimation::TranslationKey rawKey;
                rawKey.time  = key.Timestamp;
                rawKey.value = Impl::ToOzzTranslation( key.Value );
                rawTrack.translations.push_back( rawKey );
            }

            // Process rotation keys
            const size_t numRotKeys = track.RotationKeys.NumElements( );
            for ( size_t j = 0; j < numRotKeys; ++j )
            {
                const RotationKey                                 &key = track.RotationKeys.GetElement( j );
                ozz::animation::offline::RawAnimation::RotationKey rawKey;
                rawKey.time  = key.Timestamp;
                rawKey.value = Impl::ToOzzRotation( key.Value );
                rawTrack.rotations.push_back( rawKey );
            }

            // Process scale keys
            const size_t numScaleKeys = track.ScaleKeys.NumElements( );
            for ( size_t j = 0; j < numScaleKeys; ++j )
            {
                const ScaleKey                                 &key = track.ScaleKeys.GetElement( j );
                ozz::animation::offline::RawAnimation::ScaleKey rawKey;
                rawKey.time  = key.Timestamp;
                rawKey.value = Impl::ToOzzScale( key.Value );
                rawTrack.scales.push_back( rawKey );
            }
        }

        constexpr ozz::animation::offline::AnimationBuilder builder;
        if ( auto builtAnimation = builder( rawAnimation ) )
        {
            m_impl->ozzAnimation = std::move( builtAnimation );
            LOG( INFO ) << "Successfully created animation '" << m_impl->name.Get( ) << "' with duration " << duration << "s";
        }
        else
        {
            LOG( ERROR ) << "Failed to build ozz animation";
        }
    }

    Animation::~Animation( )
    {
        delete m_impl;
    }

    InteropString Animation::GetName( ) const
    {
        return m_impl->name;
    }

    float Animation::GetDuration( ) const
    {
        return m_impl->ozzAnimation ? m_impl->ozzAnimation->duration( ) : 0.0f;
    }

    Skeleton *Animation::GetSkeleton( ) const
    {
        return m_impl->skeleton;
    }
} // namespace DenOfIz
