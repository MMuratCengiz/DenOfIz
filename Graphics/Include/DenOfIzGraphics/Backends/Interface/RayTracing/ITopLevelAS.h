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

#include "../IBufferResource.h"
#include "IBottomLevelAS.h"
#include "RayTracingData.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

namespace DenOfIz
{
    struct DZ_API ASInstanceDesc
    {
        IBottomLevelAS *BLAS;
        FloatArray      Transform;
        uint32_t        ContributionToHitGroupIndex;
        uint32_t        ID;
        uint32_t        Mask = 0xFF;
    };

    struct DZ_API ASInstanceDescArray
    {
        ASInstanceDesc *Elements;
        uint32_t        NumElements;
    };

    struct DZ_API TopLevelASDesc
    {
        ASInstanceDescArray Instances;
        uint32_t            BuildFlags;
    };

    struct DZ_API UpdateTransformsDesc
    {
        // Each element in the outer array is a new instance
        FloatArrayArray Transforms;
    };

    class DZ_API ITopLevelAS
    {
    public:
        virtual ~ITopLevelAS( )                                                   = default;
        virtual void UpdateInstanceTransforms( const UpdateTransformsDesc &desc ) = 0;
    };
} // namespace DenOfIz
