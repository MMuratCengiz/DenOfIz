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

#include <DenOfIzGraphics/Animation/OzzAnimation.h>
#include <DenOfIzGraphics/Assets/Serde/Animation/AnimationAsset.h>
#include <DenOfIzGraphics/Assets/Serde/Skeleton/SkeletonAsset.h>
#include <string>
#include <unordered_map>

namespace DenOfIz
{
    struct DZ_API AnimationStateManagerDesc
    {
        SkeletonAsset *Skeleton = nullptr;
    };

    struct AnimationState
    {
        InteropString Name;
        float         PlaybackSpeed = 1.0f;
        float         CurrentTime   = 0.0f;
        float         Weight        = 1.0f;
        bool          Loop          = true;
        bool          Playing       = false;
        OzzContext   *Context       = nullptr;
    };

    struct BlendingState
    {
        InteropString SourceAnimation;
        InteropString TargetAnimation;
        float         BlendTime        = 0.5f;
        float         CurrentBlendTime = 0.0f;
        bool          InProgress       = false;
    };

    class DZ_API AnimationStateManager
    {
    private:
        OzzAnimation                      *m_ozzAnimation = nullptr;
        std::unordered_map<std::string, AnimationState> m_animations;
        InteropString                     m_currentAnimation;
        BlendingState                     m_blendingState;
        InteropArray<Float_4x4>           m_modelTransforms;

    public:
        explicit AnimationStateManager(const AnimationStateManagerDesc &desc);
        ~AnimationStateManager();

        void AddAnimation(const AnimationAsset &animationAsset);
        void Play(const InteropString &animationName, bool loop = true);
        void BlendTo(const InteropString &animationName, float blendTime = 0.5f);
        void Stop();
        void Pause();
        void Resume();
        void Update(float deltaTime);
        bool HasAnimation(const InteropString &animationName) const;
        void GetModelSpaceTransforms(InteropArray<Float_4x4> &outTransforms) const;
        const InteropString &GetCurrentAnimationName() const;
        int GetNumJoints() const;

    private:
        void UpdateBlending(float deltaTime);
    };
} // namespace DenOfIz
