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

#pragma once

#include <chrono>
#include "Common.h"

namespace DenOfIz
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
        DZ_API StepTimer( );

        [[nodiscard]] DZ_API double   GetDeltaTime( ) const;
        [[nodiscard]] DZ_API uint64_t GetElapsedTicks( ) const;
        [[nodiscard]] DZ_API double   GetElapsedSeconds( ) const;
        [[nodiscard]] DZ_API uint64_t GetTotalTicks( ) const;
        [[nodiscard]] DZ_API double   GetTotalSeconds( ) const;
        [[nodiscard]] DZ_API uint32_t GetFrameCount( ) const;
        [[nodiscard]] DZ_API uint32_t GetFramesPerSecond( ) const;
        [[nodiscard]] DZ_API bool     HasNewSecond( ) const;

        DZ_API void SetFixedTimeStep( bool isFixedTimestep );
        DZ_API void SetTargetElapsedTicks( uint64_t targetElapsed );
        DZ_API void SetTargetElapsedSeconds( double targetElapsed );

        DZ_API void ResetElapsedTime( );
        DZ_API void Tick( );
    };
} // namespace DenOfIz
