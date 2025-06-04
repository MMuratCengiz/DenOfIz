// Blazar Engine - 3D Game Engine
// Copyright (c) 2020-2021 Muhammed Murat Cengiz
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "DenOfIzGraphics/Utilities/StepTimer.h"

using namespace DenOfIz;

StepTimer::StepTimer( )
{
    m_lastTime           = Clock::now( );
    m_maxDelta           = TicksPerSecond / 10;
    m_targetElapsedTicks = TicksPerSecond / 60;
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
    m_lastTime       = currentTime;

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

        if ( OnEachSecond )
        {
            OnEachSecond( m_framesPerSecond );
        }
    }
}
