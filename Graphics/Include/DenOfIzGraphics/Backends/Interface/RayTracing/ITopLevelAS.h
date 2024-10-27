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
#include "RayTracingData.h"

namespace DenOfIz
{
    struct DZ_API ASInstanceDesc
    {
        IBufferResource    *BLASBuffer;
        InteropArray<float> Transform; // TODO: InteropArray so it can be used in other languages interop, find a way to force size
        uint32_t            ContributionToHitGroupIndex;
        uint32_t            ID;
        uint32_t            Mask = 1;
    };
    template class DZ_API InteropArray<ASInstanceDesc>;

    struct DZ_API TopLevelASDesc
    {
        InteropArray<ASInstanceDesc> Instances;
        ASBuildFlags                 BuildFlags;
    };

    struct DZ_API TopLevelASUpdateDesc
    {

    };

    class DZ_API ITopLevelAS
    {
    public:
        virtual ~ITopLevelAS( )                                                     = default;
        [[nodiscard]] virtual IBufferResource *Buffer( ) const                      = 0;
        virtual void                           Update( const TopLevelASDesc &desc ) = 0;
    };
} // namespace DenOfIz
