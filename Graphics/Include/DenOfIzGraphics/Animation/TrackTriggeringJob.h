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

#pragma once

#include <DenOfIzGraphics/Animation/AnimationTrack.h>

namespace DenOfIz
{
    struct DZ_API TrackTriggerEvent
    {
        float   ratio;    // Ratio at which the event is triggered
        int32_t keyframe; // Keyframe at which the event is triggered
    };

    class DZ_API TrackTriggeringJob
    {
    private:
        class Impl;
        Impl *m_impl;

    public:
        TrackTriggeringJob( );
        ~TrackTriggeringJob( );

        // The track to scan for triggers
        AnimationTrack *track;

        // Previous and current ratio (time / duration) in the track. Triggers are
        // detected between those 2 ratios.
        float previousRatio;
        float currentRatio;

        // If true, then an event will be triggered even if previousRatio > currentRatio
        bool processLap;

        // If true, keys (identified by their ID) are treated as edge-triggered events,
        // meaning they'll be triggered exactly once when they're passed by.
        // Otherwise (level-triggered events), a key can be triggered as long as it's
        // between previousRatio and currentRatio.
        bool edgeTrigger;

        // Output array of trigger events found between previousRatio and currentRatio.
        InteropArray<TrackTriggerEvent> outEvents;

        bool Run( );
    };
} // namespace DenOfIz
