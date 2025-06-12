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
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/Interop.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"
#include "DenOfIzGraphics/Utilities/DZArena.h"

namespace DenOfIz
{
    struct DZ_API Joint
    {
        InteropString Name;
        Float_4x4     InverseBindMatrix;
        Float_3       LocalTranslation;
        Float_4       LocalRotationQuat;
        Float_3       LocalScale;
        Float_4x4     GlobalTransform;
        uint32_t      Index       = 0;
        int32_t       ParentIndex = 0;
        UInt32Array   ChildIndices;
    };

    struct DZ_API JointArray
    {
        Joint   *Elements    = nullptr;
        uint32_t NumElements = 0;
    };

    struct DZ_API SkeletonAsset : AssetHeader, NonCopyable
    {
        DZArena _Arena{ sizeof( SkeletonAsset ) };

        static constexpr uint32_t Latest = 1;

        InteropString Name;
        JointArray    Joints{ };

        // Reference pose can be computed from joint local transforms
        SkeletonAsset( ) : AssetHeader( 0x445A534B454C /* DZSKEL */, Latest, 0 )
        {
        }

        static InteropString Extension( )
        {
            return "dzskel";
        }
    };
} // namespace DenOfIz
