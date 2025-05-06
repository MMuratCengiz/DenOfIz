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

#include <DenOfIzGraphics/Assets/Serde/Animation/AnimationAsset.h>
#include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAsset.h>

namespace DenOfIz
{
    struct DZ_API AnimationStateManagerDesc2
    {
        SkeletonAsset *Skeleton = nullptr;
    };

    class DZ_API AnimationStateManager2
    {
        class Impl;
        Impl *m_impl;

    public:
        explicit AnimationStateManager2( const AnimationStateManagerDesc2 &desc );
        ~AnimationStateManager2( );

        void                 AddAnimation( const AnimationAsset &animationAsset ) const;
        void                 Play( const InteropString &animationName, bool loop = true ) const;
        void                 BlendTo( const InteropString &animationName, float blendTime = 0.5f ) const;
        void                 Stop( ) const;
        void                 Pause( ) const;
        void                 Resume( ) const;
        void                 Update( float deltaTime ) const;
        bool                 HasAnimation( const InteropString &animationName ) const;
        void                 GetModelSpaceTransforms( InteropArray<Float_4x4> &outTransforms ) const;
        const InteropString &GetCurrentAnimationName( ) const;
        int                  GetNumJoints( ) const;
    };
} // namespace DenOfIz
