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

#include <DenOfIzGraphics/Assets/Serde/Asset.h>
#include <DenOfIzGraphics/Utilities/Interop.h>
#include <DenOfIzGraphics/Utilities/InteropMath.h>

namespace DenOfIz
{
    struct DZ_API MorphKeyframe
    {
        float Timestamp;
        float Weight;
    };

    struct DZ_API MorphAnimTrack
    {
        InteropString               Name;
        InteropArray<MorphKeyframe> Keyframes;
    };

    struct DZ_API JointPose
    {
        Float4 Position;
        Float4 Rotation;
        Float4 Scale;
    };

    struct DZ_API JointKeyframe
    {
        float     Timestamp;
        JointPose Pose;
    };

    struct DZ_API JointAnimTrack
    {
        InteropString               JointName;
        InteropArray<JointKeyframe> Keyframes;
    };

    struct DZ_API AnimationClip
    {
        InteropString                Name;
        float                        Duration       = 0;
        double                       TicksPerSecond = 0;
        InteropArray<JointAnimTrack> Tracks;
        InteropArray<MorphAnimTrack> MorphTracks;
    };

    struct DZ_API AnimationAsset : AssetHeader
    {
        static constexpr uint32_t Latest = 1;
        static constexpr uint64_t Magic  = 0x445A414E494D; // 'DZANIM'

        InteropString               Name;
        AssetUri                   SkeletonRef;
        InteropArray<AnimationClip> Animations;

        AnimationAsset( ) : AssetHeader( Magic, Latest, 0 )
        {
        }
    };

} // namespace DenOfIz
