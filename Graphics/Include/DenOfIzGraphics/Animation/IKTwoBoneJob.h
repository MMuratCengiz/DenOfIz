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

#include "DenOfIzGraphics/Utilities/InteropMath.h"

namespace DenOfIz
{
    /**
     * @brief Wrapper around ozz::animation::IKTwoBoneJob
     *
     * This class performs inverse kinematics on a three-joint chain (two bones).
     * It computes the transformations (rotations) that need to be applied to
     * the first two joints of the chain such that the third joint reaches
     * the provided target position (if possible).
     */
    struct DZ_API IKTwoBoneJobDesc
    {
        Float_4x4 StartJointMatrix{ };
        Float_4x4 MidJointMatrix{ };
        Float_4x4 EndJointMatrix{ };
        Float_3   Target{ 0, 0, 1 };
        Float_3   PoleVector{ 0, 0, 1 };
        Float_3   MidAxis{ 0, 0, 1 };
        float     Weight     = 0;
        float     TwistAngle = 0;
        float     Soften     = 0;
    };

    class DZ_API IKTwoBoneJob
    {
        IKTwoBoneJobDesc m_desc;
        bool             m_reached = false;
        Float_4          m_startJointCorrection{ };
        Float_4          m_midJointCorrection{ };

        class Impl;
        Impl *m_impl;

    public:
        explicit IKTwoBoneJob( const IKTwoBoneJobDesc &desc );
        ~IKTwoBoneJob( );
        bool Run( );

        [[nodiscard]] bool    GetReached( ) const;
        [[nodiscard]] Float_4 GetStartJointCorrection( ) const;
        [[nodiscard]] Float_4 GetMidJointCorrection( ) const;
    };

} // namespace DenOfIz
