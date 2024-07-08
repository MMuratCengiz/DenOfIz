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
#include <DenOfIzCore/Time.h>

using namespace DenOfIz;

void Time::Tick()
{
    if ( firstTickTime == 0 )
    {
        firstTickTime = DoubleEpochNow();
    }

    if ( prev == 0 )
    {
        prev = DoubleEpochNow();
        return;
    }

    double now = DoubleEpochNow();

    deltaTime = (now - prev) / 1000000.0; // std::max( now - prev, (double) 1 / 60.f );
    prev      = now;
    frames++;

    if ( now - lastFrameTick >= 1000000 )
    {
        ListenFps(frames);
        lastFrameTick = now;
        frames        = 0;
    }
}

const double Time::GetDeltaTime() const
{
    return deltaTime;
}

const double Time::GetFirstTickTime() const
{
    return firstTickTime;
}

double Time::DoubleEpochNow()
{
    return std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}
