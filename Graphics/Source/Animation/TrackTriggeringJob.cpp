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

#include <DenOfIzGraphics/Animation/TrackTriggeringJob.h>
#include <glog/logging.h>

#include <ozz/animation/runtime/track_triggering_job.h>

namespace DenOfIz
{
    class TrackTriggeringJob::Impl
    {
    public:
        ozz::vector<ozz::animation::TrackTriggeringJob::Event> ozzEvents;

        static bool ValidateSetup( const TrackTriggeringJob &job )
        {
            if ( !job.track )
            {
                LOG( ERROR ) << "TrackTriggeringJob: track is null";
                return false;
            }

            if ( job.previousRatio < 0.0f || job.previousRatio > 1.0f )
            {
                LOG( WARNING ) << "TrackTriggeringJob: previousRatio " << job.previousRatio << " is outside [0,1] range, will be clamped";
            }

            if ( job.currentRatio < 0.0f || job.currentRatio > 1.0f )
            {
                LOG( WARNING ) << "TrackTriggeringJob: currentRatio " << job.currentRatio << " is outside [0,1] range, will be clamped";
            }

            return true;
        }
    };

    TrackTriggeringJob::TrackTriggeringJob( ) : m_impl( new Impl( ) ), track( nullptr ), previousRatio( 0.0f ), currentRatio( 0.0f ), processLap( false ), edgeTrigger( true )
    {
    }

    TrackTriggeringJob::~TrackTriggeringJob( )
    {
        delete m_impl;
    }

    bool TrackTriggeringJob::Run( )
    {
        if ( !m_impl->ValidateSetup( *this ) )
        {
            return false;
        }

        const float clampedPreviousRatio = std::max( 0.0f, std::min( previousRatio, 1.0f ) );
        const float clampedCurrentRatio  = std::max( 0.0f, std::min( currentRatio, 1.0f ) );

        auto &trackImpl = *track->m_impl;
        if ( trackImpl.valueType != TrackValueType::Float )
        {
            LOG( ERROR ) << "TrackTriggeringJob only works with float tracks";
            return false;
        }

        ozz::animation::TrackTriggeringJob ozzJob;
        ozzJob.track          = trackImpl.floatTrack.get( );
        ozzJob.previous_ratio = clampedPreviousRatio;
        ozzJob.ratio          = clampedCurrentRatio;
        ozzJob.process_lap    = processLap;
        ozzJob.edge_triggered = edgeTrigger;

        outEvents.Clear( );
        m_impl->ozzEvents.clear( );

        ozzJob.events = &m_impl->ozzEvents;

        if ( !ozzJob.Run( ) )
        {
            LOG( ERROR ) << "Track triggering job failed";
            return false;
        }

        const size_t numEvents = m_impl->ozzEvents.size( );
        if ( numEvents > 0 )
        {
            outEvents.Resize( numEvents );

            for ( size_t i = 0; i < numEvents; ++i )
            {
                const ozz::animation::TrackTriggeringJob::Event &ozzEvent = m_impl->ozzEvents[ i ];
                TrackTriggerEvent                               &event    = outEvents.GetElement( i );

                event.ratio    = ozzEvent.ratio;
                event.keyframe = ozzEvent.keyframe;
            }
        }

        return true;
    }
} // namespace DenOfIz
