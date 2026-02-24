/*
Blazar Engine - 3D Game Engine
Copyright (c) 2020-2021 Muhammed Murat Cengiz

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

#include "DenOfIzGraphics/Utilities/StepTimer.h"
#include <chrono>

#define STEP_TIMER_IMPL( handle ) DENOFIZ_FROM_HANDLE( StepTimer, handle )

namespace
{
    class StepTimer
    {
        using Clock     = std::chrono::high_resolution_clock;
        using TimePoint = std::chrono::time_point<Clock>;

        TimePoint m_lastTime{ };
        uint64_t  m_maxDelta{ };

        uint64_t m_elapsedTicks{ };
        uint64_t m_totalTicks{ };
        uint64_t m_leftOverTicks{ };

        uint32_t m_frameCount{ };
        uint32_t m_framesPerSecond{ };
        uint32_t m_framesThisSecond{ };
        uint64_t m_secondCounter{ };

        bool     m_isFixedTimeStep{ false };
        uint64_t m_targetElapsedTicks{ };

        static constexpr uint64_t TicksPerSecond      = 10000000;
        static constexpr double   MicrosecondsPerTick = 1.0 / ( TicksPerSecond / 1000000.0 );

    public:
        StepTimer( );
        ~StepTimer( ) = default;

        double   GetDeltaTime( ) const;
        uint64_t GetElapsedTicks( ) const;
        double   GetElapsedSeconds( ) const;
        uint64_t GetTotalTicks( ) const;
        double   GetTotalSeconds( ) const;
        uint32_t GetFrameCount( ) const;
        uint32_t GetFramesPerSecond( ) const;

        void SetFixedTimeStep( bool isFixedTimestep );
        void SetTargetElapsedTicks( uint64_t targetElapsed );
        void SetTargetElapsedSeconds( double targetElapsed );

        void ResetElapsedTime( );
        void Tick( );
    };

    StepTimer::StepTimer( )
    {
        m_lastTime           = Clock::now( );
        m_maxDelta           = TicksPerSecond / 10;
        m_targetElapsedTicks = TicksPerSecond / 60;
    }

    double StepTimer::GetDeltaTime( ) const
    {
        return static_cast<double>( m_elapsedTicks ) / TicksPerSecond;
    }

    uint64_t StepTimer::GetElapsedTicks( ) const
    {
        return m_elapsedTicks;
    }

    double StepTimer::GetElapsedSeconds( ) const
    {
        return static_cast<double>( m_elapsedTicks ) / TicksPerSecond;
    }

    uint64_t StepTimer::GetTotalTicks( ) const
    {
        return m_totalTicks;
    }

    double StepTimer::GetTotalSeconds( ) const
    {
        return static_cast<double>( m_totalTicks ) / TicksPerSecond;
    }

    uint32_t StepTimer::GetFrameCount( ) const
    {
        return m_frameCount;
    }

    uint32_t StepTimer::GetFramesPerSecond( ) const
    {
        return m_framesPerSecond;
    }

    void StepTimer::SetFixedTimeStep( const bool isFixedTimestep )
    {
        m_isFixedTimeStep = isFixedTimestep;
    }

    void StepTimer::SetTargetElapsedTicks( const uint64_t targetElapsed )
    {
        m_targetElapsedTicks = targetElapsed;
    }

    void StepTimer::SetTargetElapsedSeconds( const double targetElapsed )
    {
        m_targetElapsedTicks = static_cast<uint64_t>( targetElapsed * TicksPerSecond );
    }

    void StepTimer::ResetElapsedTime( )
    {
        m_lastTime         = Clock::now( );
        m_leftOverTicks    = 0;
        m_framesPerSecond  = 0;
        m_framesThisSecond = 0;
        m_secondCounter    = 0;
    }

    void StepTimer::Tick( )
    {
        const auto currentTime = Clock::now( );
        const auto timeDelta   = std::chrono::duration_cast<std::chrono::nanoseconds>( currentTime - m_lastTime );
        m_lastTime             = currentTime;

        uint64_t deltaTicks = timeDelta.count( ) / 100;
        m_secondCounter += deltaTicks;

        if ( deltaTicks > m_maxDelta )
        {
            deltaTicks = m_maxDelta;
        }

        const uint32_t lastFrameCount = m_frameCount;

        if ( m_isFixedTimeStep )
        {
            if ( abs( static_cast<int64_t>( deltaTicks - m_targetElapsedTicks ) ) < TicksPerSecond / 4000 )
            {
                deltaTicks = m_targetElapsedTicks;
            }

            m_leftOverTicks += deltaTicks;

            while ( m_leftOverTicks >= m_targetElapsedTicks )
            {
                m_elapsedTicks = m_targetElapsedTicks;
                m_totalTicks += m_targetElapsedTicks;
                m_leftOverTicks -= m_targetElapsedTicks;
                m_frameCount++;
            }
        }
        else
        {
            m_elapsedTicks = deltaTicks;
            m_totalTicks += deltaTicks;
            m_leftOverTicks = 0;
            m_frameCount++;
        }

        if ( m_frameCount != lastFrameCount )
        {
            m_framesThisSecond++;
        }

        if ( m_secondCounter >= TicksPerSecond )
        {
            m_framesPerSecond  = m_framesThisSecond;
            m_framesThisSecond = 0;
            m_secondCounter %= TicksPerSecond;
        }
    }
} // namespace

extern "C"
{

    DenOfIz_StepTimer DenOfIz_StepTimer_Create( )
    {
        auto *timer = new StepTimer( );
        return DENOFIZ_TO_HANDLE( timer );
    }

    void DenOfIz_StepTimer_Destroy( DenOfIz_StepTimer timer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( timer ) )
        {
            return;
        }
        delete STEP_TIMER_IMPL( timer );
    }

    double DenOfIz_StepTimer_GetDeltaTime( DenOfIz_StepTimer timer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( timer ) )
        {
            return 0.0;
        }
        return STEP_TIMER_IMPL( timer )->GetDeltaTime( );
    }

    uint64_t DenOfIz_StepTimer_GetElapsedTicks( DenOfIz_StepTimer timer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( timer ) )
        {
            return 0;
        }
        return STEP_TIMER_IMPL( timer )->GetElapsedTicks( );
    }

    double DenOfIz_StepTimer_GetElapsedSeconds( DenOfIz_StepTimer timer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( timer ) )
        {
            return 0.0;
        }
        return STEP_TIMER_IMPL( timer )->GetElapsedSeconds( );
    }

    uint64_t DenOfIz_StepTimer_GetTotalTicks( DenOfIz_StepTimer timer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( timer ) )
        {
            return 0;
        }
        return STEP_TIMER_IMPL( timer )->GetTotalTicks( );
    }

    double DenOfIz_StepTimer_GetTotalSeconds( DenOfIz_StepTimer timer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( timer ) )
        {
            return 0.0;
        }
        return STEP_TIMER_IMPL( timer )->GetTotalSeconds( );
    }

    uint32_t DenOfIz_StepTimer_GetFrameCount( DenOfIz_StepTimer timer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( timer ) )
        {
            return 0;
        }
        return STEP_TIMER_IMPL( timer )->GetFrameCount( );
    }

    uint32_t DenOfIz_StepTimer_GetFramesPerSecond( DenOfIz_StepTimer timer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( timer ) )
        {
            return 0;
        }
        return STEP_TIMER_IMPL( timer )->GetFramesPerSecond( );
    }

    void DenOfIz_StepTimer_SetFixedTimeStep( DenOfIz_StepTimer timer, bool isFixedTimestep )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( timer ) )
        {
            return;
        }
        STEP_TIMER_IMPL( timer )->SetFixedTimeStep( isFixedTimestep );
    }

    void DenOfIz_StepTimer_SetTargetElapsedTicks( DenOfIz_StepTimer timer, uint64_t targetElapsed )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( timer ) )
        {
            return;
        }
        STEP_TIMER_IMPL( timer )->SetTargetElapsedTicks( targetElapsed );
    }

    void DenOfIz_StepTimer_SetTargetElapsedSeconds( DenOfIz_StepTimer timer, double targetElapsed )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( timer ) )
        {
            return;
        }
        STEP_TIMER_IMPL( timer )->SetTargetElapsedSeconds( targetElapsed );
    }

    void DenOfIz_StepTimer_ResetElapsedTime( DenOfIz_StepTimer timer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( timer ) )
        {
            return;
        }
        STEP_TIMER_IMPL( timer )->ResetElapsedTime( );
    }

    void DenOfIz_StepTimer_Tick( DenOfIz_StepTimer timer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( timer ) )
        {
            return;
        }
        STEP_TIMER_IMPL( timer )->Tick( );
    }
}
