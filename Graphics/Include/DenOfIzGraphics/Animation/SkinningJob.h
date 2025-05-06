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

#include <DenOfIzGraphics/Animation/AnimationSetup.h>

namespace DenOfIz
{
    class DZ_API SkinningJob
    {
    public:
        enum class Influences : int32_t
        {
            One  = 1,
            Two  = 2,
            Four = 4,
            Dynamic
        };

    private:
        class Impl;
        Impl *m_impl;

    public:
        SkinningJob( );
        ~SkinningJob( );

        AnimationSetup         *setup;
        InteropArray<int>       jointIndices;
        InteropArray<Float_4x4> jointInverseBindPoses;
        Influences              influences;

        InteropArray<Float_3> in_positions;
        InteropArray<Float_3> in_normals;
        InteropArray<Float_3> in_tangents;
        InteropArray<Float_4> in_weights;

        InteropArray<Float_3> out_positions;
        InteropArray<Float_3> out_normals;
        InteropArray<Float_3> out_tangents;

        bool Run( );
    };
} // namespace DenOfIz
