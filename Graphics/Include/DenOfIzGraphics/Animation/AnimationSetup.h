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

#include "Skeleton.h"

namespace DenOfIz
{
    class Skeleton;
    class Animation;

    class DZ_API AnimationSetup
    {
        class Impl;
        Impl *m_impl;

    public:
        explicit AnimationSetup( Skeleton *skeleton );
        ~AnimationSetup( );

        Skeleton *GetSkeleton( ) const;

        static InteropArray<Float_4x4> GetLocalTransforms( );
        InteropArray<Float_4x4>        GetModelTransforms( ) const;

        int GetNumJoints( ) const;

        friend class SamplingJob;
        friend class BlendingJob;
        friend class LocalToModelJob;
    };
} // namespace DenOfIz
