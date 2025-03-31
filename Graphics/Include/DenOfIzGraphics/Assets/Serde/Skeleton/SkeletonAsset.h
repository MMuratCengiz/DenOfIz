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
    struct DZ_API Joint
    {
        InteropString          Name;
        Float_4x4              InverseBindMatrix;
        Float_4x4              LocalTransform;
        Float_4x4              GlobalTransform;
        uint32_t               Index       = 0;
        int32_t                ParentIndex = 0;
        InteropArray<uint32_t> ChildIndices;
    };

    struct DZ_API SkeletonAsset : AssetHeader
    {
        static constexpr uint32_t Latest = 1;

        InteropString       Name;
        InteropArray<Joint> Joints;

        // Reference pose can be computed from joint local transforms
        SkeletonAsset( ) : AssetHeader( 0x445A534B454C /* DZSKEL */, Latest, 0 )
        {
        }
    };
} // namespace DenOfIz
