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
    class DZ_API IKAimJob
    {
    private:
        class Impl;
        Impl *m_impl;

    public:
        IKAimJob( );
        ~IKAimJob( );

        // The animation setup containing both the skeleton and the pose to modify
        AnimationSetup *setup;

        // The joint on which the IK is applied
        int jointIndex;

        // The target point that the joint should aim at
        Float_3 target;

        // Forward direction in joint local-space
        Float_3 forward;

        // Up direction in model space
        Float_3 up;

        // Maximum angle for the correction
        float maxAngle;

        // The weight of the correction, between 0 (no correction) and 1 (full correction)
        float weight;

        // If true, the influenced hierarchy will maintain its offset with the targeted
        // joint but ignore its rotation. Otherwise, it will simply be animated.
        bool alignedToModel;

        // When used in combination with an offset, allows to define the axis (in joint
        // local-space) that should be used for the rotation. Otherwise, joint rotation
        // is influenced "as is".
        Float_3 twistAxis;

        // Offset axis of the visual on the joint, in joint local-space.
        Float_3 poleVector;

        bool Run( );
    };
} // namespace DenOfIz
