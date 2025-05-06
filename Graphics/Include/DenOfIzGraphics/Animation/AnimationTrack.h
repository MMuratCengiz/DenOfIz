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

#include <DenOfIzGraphics/Utilities/InteropMath.h>

namespace DenOfIz
{
    enum class TrackValueType
    {
        Float,
        Float2,
        Float3,
        Float4,
        Quaternion
    };

    class DZ_API AnimationTrack
    {
        class Impl;
        Impl *m_impl;

    public:
        explicit AnimationTrack( TrackValueType valueType );
        ~AnimationTrack( );

        void AddKey( float time, const float *values, int numValues );
        void AddKey( float time, float value );
        void AddKey( float time, const Float_2 &value );
        void AddKey( float time, const Float_3 &value );
        void AddKey( float time, const Float_4 &value );

        TrackValueType GetValueType( ) const;
        float          GetDuration( ) const;

        friend class TrackSamplingJob;
        friend class TrackTriggeringJob;
    };
} // namespace DenOfIz
