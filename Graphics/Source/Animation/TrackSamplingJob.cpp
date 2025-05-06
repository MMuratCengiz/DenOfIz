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

#include <DenOfIzGraphics/Animation/TrackSamplingJob.h>
#include <glog/logging.h>

#include <ozz/animation/runtime/track_sampling_job.h>
#include <ozz/base/maths/quaternion.h>
#include <ozz/base/maths/vec_float.h>

namespace DenOfIz
{
    class TrackSamplingJob::Impl
    {
    public:
        static Float_2 FromOzzFloat2( const ozz::math::Float2 &v )
        {
            return Float_2{ v.x, v.y };
        }

        static Float_3 FromOzzFloat3( const ozz::math::Float3 &v )
        {
            return Float_3{ v.x, v.y, v.z };
        }

        static Float_4 FromOzzFloat4( const ozz::math::Float4 &v )
        {
            return Float_4{ v.x, v.y, v.z, v.w };
        }

        static Float_4 FromOzzQuaternion( const ozz::math::Quaternion &q )
        {
            return Float_4{ q.x, q.y, q.z, q.w };
        }

        static bool ValidateSetup( const TrackSamplingJob &job )
        {
            if ( !job.track )
            {
                LOG( ERROR ) << "TrackSamplingJob: track is null";
                return false;
            }

            if ( job.ratio < 0.0f || job.ratio > 1.0f )
            {
                LOG( WARNING ) << "TrackSamplingJob: ratio " << job.ratio << " is outside [0,1] range, will be clamped";
            }

            return true;
        }
    };

    TrackSamplingJob::TrackSamplingJob( ) :
        m_impl( new Impl( ) ), track( nullptr ), ratio( 0.0f ), outFloat( 0.0f ), outFloat2{ 0.0f, 0.0f }, outFloat3{ 0.0f, 0.0f, 0.0f }, outFloat4{ 0.0f, 0.0f, 0.0f, 1.0f }
    {
    }

    TrackSamplingJob::~TrackSamplingJob( )
    {
        delete m_impl;
    }

    bool TrackSamplingJob::Run( )
    {
        if ( !m_impl->ValidateSetup( *this ) )
        {
            return false;
        }

        const float clampedRatio = std::max( 0.0f, std::min( ratio, 1.0f ) );
        auto       &trackImpl    = *track->m_impl;
        bool        success      = false;

        switch ( trackImpl.valueType )
        {
        case TrackValueType::Float:
            {
                ozz::animation::FloatTrackSamplingJob ozzJob;
                ozzJob.track  = trackImpl.floatTrack.get( );
                ozzJob.ratio  = clampedRatio;
                ozzJob.result = &outFloat;
                success       = ozzJob.Run( );
                break;
            }
        case TrackValueType::Float2:
            {
                ozz::animation::Float2TrackSamplingJob ozzJob;
                ozzJob.track = trackImpl.float2Track.get( );
                ozzJob.ratio = clampedRatio;
                ozz::math::Float2 result;
                ozzJob.result = &result;
                success       = ozzJob.Run( );
                if ( success )
                {
                    outFloat2 = m_impl->FromOzzFloat2( result );
                }
                break;
            }
        case TrackValueType::Float3:
            {
                ozz::animation::Float3TrackSamplingJob ozzJob;
                ozzJob.track = trackImpl.float3Track.get( );
                ozzJob.ratio = clampedRatio;
                ozz::math::Float3 result;
                ozzJob.result = &result;
                success       = ozzJob.Run( );
                if ( success )
                {
                    outFloat3 = m_impl->FromOzzFloat3( result );
                }
                break;
            }
        case TrackValueType::Float4:
            {
                ozz::animation::Float4TrackSamplingJob ozzJob;
                ozzJob.track = trackImpl.float4Track.get( );
                ozzJob.ratio = clampedRatio;
                ozz::math::Float4 result;
                ozzJob.result = &result;
                success       = ozzJob.Run( );
                if ( success )
                {
                    outFloat4 = m_impl->FromOzzFloat4( result );
                }
                break;
            }
        case TrackValueType::Quaternion:
            {
                ozz::animation::QuaternionTrackSamplingJob ozzJob;
                ozzJob.track = trackImpl.quaternionTrack.get( );
                ozzJob.ratio = clampedRatio;
                ozz::math::Quaternion result;
                ozzJob.result = &result;
                success       = ozzJob.Run( );
                if ( success )
                {
                    outFloat4 = m_impl->FromOzzQuaternion( result );
                }
                break;
            }
        default:
            break;
        }

        if ( !success )
        {
            LOG( ERROR ) << "Failed to sample track";
            return false;
        }

        return true;
    }
} // namespace DenOfIz
