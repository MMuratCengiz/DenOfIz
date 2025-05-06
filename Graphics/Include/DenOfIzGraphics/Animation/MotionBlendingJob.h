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
    struct DZ_API MotionBlendSpec
    {
        float   weight;
        Float_3 translation;
        Float_4 rotation;
    };

    class DZ_API MotionBlendingJob
    {
    private:
        class Impl;
        Impl *m_impl;

    public:
        MotionBlendingJob( );
        ~MotionBlendingJob( );

        // Array of blend specifications, each with weight, translation and rotation
        InteropArray<MotionBlendSpec> inputs;

        // Threshold below which blending has no effect
        float threshold;

        // Output blended transform
        Float_3 outTranslation;
        Float_4 outRotation;

        bool Run( );
    };
} // namespace DenOfIz
