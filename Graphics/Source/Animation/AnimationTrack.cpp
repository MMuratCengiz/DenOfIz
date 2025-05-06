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

#include <DenOfIzGraphics/Animation/AnimationTrack.h>
#include <glog/logging.h>

#include <ozz/animation/offline/raw_track.h>
#include <ozz/animation/runtime/track.h>
#include <ozz/animation/runtime/track_builder.h>

namespace DenOfIz
{
    class AnimationTrack::Impl
    {
    public:
        TrackValueType                                        valueType;
        ozz::animation::offline::RawTrackInterpolation::Value interpolation = ozz::animation::offline::RawTrackInterpolation::kLinear;

        ozz::unique_ptr<ozz::animation::FloatTrack>      floatTrack;
        ozz::unique_ptr<ozz::animation::Float2Track>     float2Track;
        ozz::unique_ptr<ozz::animation::Float3Track>     float3Track;
        ozz::unique_ptr<ozz::animation::Float4Track>     float4Track;
        ozz::unique_ptr<ozz::animation::QuaternionTrack> quaternionTrack;

        ozz::animation::offline::RawFloatTrack      rawFloatTrack;
        ozz::animation::offline::RawFloat2Track     rawFloat2Track;
        ozz::animation::offline::RawFloat3Track     rawFloat3Track;
        ozz::animation::offline::RawFloat4Track     rawFloat4Track;
        ozz::animation::offline::RawQuaternionTrack rawQuaternionTrack;

        ozz::animation::offline::TrackBuilder builder;

        void BuildTrack( )
        {
            switch ( valueType )
            {
            case TrackValueType::Float:
                floatTrack = builder( rawFloatTrack );
                break;
            case TrackValueType::Float2:
                float2Track = builder( rawFloat2Track );
                break;
            case TrackValueType::Float3:
                float3Track = builder( rawFloat3Track );
                break;
            case TrackValueType::Float4:
                float4Track = builder( rawFloat4Track );
                break;
            case TrackValueType::Quaternion:
                quaternionTrack = builder( rawQuaternionTrack );
                break;
            }
        }

        float GetDuration( ) const
        {
            switch ( valueType )
            {
            case TrackValueType::Float:
                return floatTrack ? floatTrack->duration( ) : 0.0f;
            case TrackValueType::Float2:
                return float2Track ? float2Track->duration( ) : 0.0f;
            case TrackValueType::Float3:
                return float3Track ? float3Track->duration( ) : 0.0f;
            case TrackValueType::Float4:
                return float4Track ? float4Track->duration( ) : 0.0f;
            case TrackValueType::Quaternion:
                return quaternionTrack ? quaternionTrack->duration( ) : 0.0f;
            }
            return 0.0f;
        }
    };

    AnimationTrack::AnimationTrack( TrackValueType valueType ) : m_impl( new Impl( ) )
    {
        m_impl->valueType = valueType;
    }

    AnimationTrack::~AnimationTrack( )
    {
        delete m_impl;
    }

    void AnimationTrack::AddKey( float time, const float *values, int numValues )
    {
        switch ( m_impl->valueType )
        {
        case TrackValueType::Float:
            if ( numValues >= 1 )
            {
                ozz::animation::offline::RawFloatTrack::Keyframe key;
                key.time          = time;
                key.value         = values[ 0 ];
                key.interpolation = m_impl->interpolation;
                m_impl->rawFloatTrack.keyframes.push_back( key );
            }
            break;

        case TrackValueType::Float2:
            if ( numValues >= 2 )
            {
                ozz::animation::offline::RawFloat2Track::Keyframe key;
                key.time          = time;
                key.value         = ozz::math::Float2( values[ 0 ], values[ 1 ] );
                key.interpolation = m_impl->interpolation;
                m_impl->rawFloat2Track.keyframes.push_back( key );
            }
            break;

        case TrackValueType::Float3:
            if ( numValues >= 3 )
            {
                ozz::animation::offline::RawFloat3Track::Keyframe key;
                key.time          = time;
                key.value         = ozz::math::Float3( values[ 0 ], values[ 1 ], values[ 2 ] );
                key.interpolation = m_impl->interpolation;
                m_impl->rawFloat3Track.keyframes.push_back( key );
            }
            break;

        case TrackValueType::Float4:
            if ( numValues >= 4 )
            {
                ozz::animation::offline::RawFloat4Track::Keyframe key;
                key.time          = time;
                key.value         = ozz::math::Float4( values[ 0 ], values[ 1 ], values[ 2 ], values[ 3 ] );
                key.interpolation = m_impl->interpolation;
                m_impl->rawFloat4Track.keyframes.push_back( key );
            }
            break;

        case TrackValueType::Quaternion:
            if ( numValues >= 4 )
            {
                ozz::animation::offline::RawQuaternionTrack::Keyframe key;
                key.time          = time;
                key.value         = ozz::math::Quaternion( values[ 0 ], values[ 1 ], values[ 2 ], values[ 3 ] );
                key.interpolation = m_impl->interpolation;
                m_impl->rawQuaternionTrack.keyframes.push_back( key );
            }
            break;
        }

        // Rebuild the track after adding a key
        m_impl->BuildTrack( );
    }

    void AnimationTrack::AddKey( float time, float value )
    {
        if ( m_impl->valueType != TrackValueType::Float )
        {
            LOG( ERROR ) << "Trying to add a float key to a non-float track";
            return;
        }

        AddKey( time, &value, 1 );
    }

    void AnimationTrack::AddKey( float time, const Float_2 &value )
    {
        if ( m_impl->valueType != TrackValueType::Float2 )
        {
            LOG( ERROR ) << "Trying to add a Float_2 key to a non-Float2 track";
            return;
        }

        float values[ 2 ] = { value.X, value.Y };
        AddKey( time, values, 2 );
    }

    void AnimationTrack::AddKey( float time, const Float_3 &value )
    {
        if ( m_impl->valueType != TrackValueType::Float3 )
        {
            LOG( ERROR ) << "Trying to add a Float_3 key to a non-Float3 track";
            return;
        }

        float values[ 3 ] = { value.X, value.Y, value.Z };
        AddKey( time, values, 3 );
    }

    void AnimationTrack::AddKey( float time, const Float_4 &value )
    {
        if ( m_impl->valueType != TrackValueType::Float4 && m_impl->valueType != TrackValueType::Quaternion )
        {
            LOG( ERROR ) << "Trying to add a Float_4 key to a non-Float4/Quaternion track";
            return;
        }

        float values[ 4 ] = { value.X, value.Y, value.Z, value.W };
        AddKey( time, values, 4 );
    }

    TrackValueType AnimationTrack::GetValueType( ) const
    {
        return m_impl->valueType;
    }

    float AnimationTrack::GetDuration( ) const
    {
        return m_impl->GetDuration( );
    }
} // namespace DenOfIz
