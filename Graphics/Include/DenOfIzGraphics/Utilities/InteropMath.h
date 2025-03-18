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

#include "Interop.h"

namespace DenOfIz
{
    struct DZ_API Float2
    {
        float X;
        float Y;
    };

    struct DZ_API Float3
    {
        float X;
        float Y;
        float Z;
    };

    struct DZ_API Float4
    {
        float X;
        float Y;
        float Z;
        float W;
    };

    struct DZ_API Int2
    {
        int X;
        int Y;
    };

    struct DZ_API Int3
    {
        int X;
        int Y;
        int Z;
    };

    struct DZ_API Int4
    {
        int X;
        int Y;
        int Z;
        int W;
    };

    struct DZ_API UInt2
    {
        uint32_t X;
        uint32_t Y;
    };

    struct DZ_API UInt3
    {
        uint32_t X;
        uint32_t Y;
        uint32_t Z;
    };

    struct DZ_API UInt4
    {
        uint32_t X;
        uint32_t Y;
        uint32_t Z;
        uint32_t W;
    };

    struct DZ_API Matrix4
    {
        Float4 Cols[ 4 ];
    };

    struct DZ_API Float4x4
    {
        float M[ 16 ];

        Float4x4( )
        {
            for ( int i = 0; i < 16; i++ )
            {
                M[ i ] = 0.0f;
            }
            M[ 0 ] = M[ 5 ] = M[ 10 ] = M[ 15 ] = 1.0f;
        }
    };

} // namespace DenOfIz
