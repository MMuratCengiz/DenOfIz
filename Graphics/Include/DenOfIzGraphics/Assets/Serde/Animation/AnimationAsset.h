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

#include "DenOfIzGraphics/Assets/Serde/Asset.h"
#include "DenOfIzGraphics/Utilities/Interop.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"

namespace DenOfIz
{
    struct DZ_API PositionKey
    {
        float   Timestamp; // Time in seconds
        Float_3 Value;
    };
    template class DZ_API InteropArray<PositionKey>;

    struct DZ_API RotationKey
    {
        float   Timestamp; // Time in seconds
        Float_4 Value;     // Quaternion
    };
    template class DZ_API InteropArray<RotationKey>;

    struct DZ_API ScaleKey
    {
        float   Timestamp;
        Float_3 Value;
    };
    template class DZ_API InteropArray<ScaleKey>;

    struct DZ_API MorphKeyframe
    {
        float Timestamp;
        float Weight;
    };
    template class DZ_API InteropArray<MorphKeyframe>;

    struct DZ_API MorphAnimTrack
    {
        InteropString               Name;
        InteropArray<MorphKeyframe> Keyframes;
    };
    template class DZ_API InteropArray<MorphAnimTrack>;

    struct DZ_API JointAnimTrack
    {
        InteropString             JointName;
        InteropArray<PositionKey> PositionKeys;
        InteropArray<RotationKey> RotationKeys;
        InteropArray<ScaleKey>    ScaleKeys;
    };
    template class DZ_API InteropArray<JointAnimTrack>;

    struct DZ_API AnimationClip
    {
        InteropString                Name;
        float                        Duration{ };
        InteropArray<JointAnimTrack> Tracks;
        InteropArray<MorphAnimTrack> MorphTracks;
    };
    template class DZ_API InteropArray<AnimationClip>;

    struct DZ_API AnimationAsset : AssetHeader
    {
        static constexpr uint32_t Latest = 1;

        InteropString               Name;
        AssetUri                    SkeletonRef;
        InteropArray<AnimationClip> Animations;

        AnimationAsset( ) : AssetHeader( 0x445A414E494D /*DZANIM*/, Latest, 0 )
        {
        }

        static InteropString Extension( )
        {
            return "dzanim";
        }
    };
} // namespace DenOfIz
