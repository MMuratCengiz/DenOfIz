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

#include <string>
#include <unordered_map>
#include "DenOfIzGraphics/Animation/OzzAnimation.h"
#include "DenOfIzGraphics/Assets/Serde/Animation/AnimationAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAsset.h"

namespace DenOfIz
{
    struct DZ_API AnimationStateManagerDesc
    {
        SkeletonAsset *Skeleton = nullptr;
    };

    struct DZ_API AnimationState
    {
        InteropString Name;
        float         PlaybackSpeed = 1.0f;
        float         CurrentTime   = 0.0f;
        float         Weight        = 1.0f;
        bool          Loop          = true;
        bool          Playing       = false;
        OzzContext   *Context       = nullptr;
    };

    struct DZ_API BlendingState
    {
        InteropString SourceAnimation;
        InteropString TargetAnimation;
        float         BlendTime        = 0.5f;
        float         CurrentBlendTime = 0.0f;
        bool          InProgress       = false;
    };

    class AnimationStateManager
    {
        OzzAnimation                                   *m_ozzAnimation = nullptr;
        std::unordered_map<std::string, AnimationState> m_animations;
        InteropString                                   m_currentAnimation;
        BlendingState                                   m_blendingState;
        std::vector<Float_4x4>                          m_modelTransforms;
        std::vector<Float_4x4>                          m_blendSourceTransforms;
        std::vector<Float_4x4>                          m_blendTargetTransforms;

    public:
        DZ_API explicit AnimationStateManager( const AnimationStateManagerDesc &desc );
        DZ_API ~AnimationStateManager( );
        DZ_API void                               AddAnimation( const AnimationAsset &animationAsset );
        DZ_API void                               Play( const InteropString &animationName, bool loop = true );
        DZ_API void                               BlendTo( const InteropString &animationName, float blendTime = 0.5f );
        DZ_API void                               Stop( );
        DZ_API void                               Pause( );
        DZ_API void                               Resume( );
        DZ_API void                               Update( float deltaTime );
        DZ_API [[nodiscard]] bool                 HasAnimation( const InteropString &animationName ) const;
        DZ_API Float_4x4Array                     GetModelSpaceTransforms( );
        DZ_API [[nodiscard]] const InteropString &GetCurrentAnimationName( ) const;
        DZ_API [[nodiscard]] int                  GetNumJoints( ) const;

    private:
        void UpdateBlending( float deltaTime );
    };
} // namespace DenOfIz
