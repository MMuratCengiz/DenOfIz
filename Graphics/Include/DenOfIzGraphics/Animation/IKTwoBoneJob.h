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
    class DZ_API IKTwoBoneJob
    {
    private:
        class Impl;
        Impl *m_impl;

    public:
        IKTwoBoneJob( );
        ~IKTwoBoneJob( );

        // The animation setup containing both the skeleton and the pose to modify
        AnimationSetup *setup;

        // The three joint chain used by the IK
        int startJointIndex; // Shoulder
        int midJointIndex;   // Elbow
        int endJointIndex;   // Wrist

        // Target to reach, expressed in model-space
        Float_3 target;

        // Target model-space rotation that the end joint should have
        Float_4 targetRotation;

        // Pole vector, expressed in model-space, used to control the joint chain plane rotation
        Float_3 poleVector;

        // The weight of the correction, between 0 (no correction) and 1 (full correction)
        float weight;

        // Optional twist offset angle to rotate around middle-end axis
        float twistAngle;

        // If true, softens the effect of the IK based on target distance
        bool soften;

        // Maximum length ratio (can't be more than 1)
        float softenDistance;

        // Whether to allow the start joint to rotate as part of the IK solution
        bool allowStretching;

        // Whether to use pole vector
        bool usePoleVector;

        // Whether to use target rotation
        bool useTargetRotation;

        bool Run( );
    };
} // namespace DenOfIz
