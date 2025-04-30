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

/*
 * NOTE:
 *  This class is mostly used to specify types, it is not intended to provide full math functions, please convert them to relevant types of the Math library you use.
 */

namespace DenOfIz
{
    struct DZ_API Float_2
    {
        float X;
        float Y;
    };
    template class DZ_API InteropArray<Float_2>;

    struct DZ_API Float_3
    {
        float X;
        float Y;
        float Z;
    };
    template class DZ_API InteropArray<Float_3>;

    struct DZ_API Float_4
    {
        float X;
        float Y;
        float Z;
        float W;
    };
    template class DZ_API InteropArray<Float_4>;

    struct DZ_API Int16_2
    {
        int16_t X;
        int16_t Y;
    };
    template class DZ_API InteropArray<Int16_2>;

    struct DZ_API Int16_3
    {
        int16_t X;
        int16_t Y;
        int16_t Z;
    };
    template class DZ_API InteropArray<Int16_3>;

    struct DZ_API Int16_4
    {
        int16_t X;
        int16_t Y;
        int16_t Z;
        int16_t W;
    };
    template class DZ_API InteropArray<Int16_4>;

    struct DZ_API Int32_2
    {
        int X;
        int Y;
    };
    template class DZ_API InteropArray<Int32_2>;

    struct DZ_API Int32_3
    {
        int X;
        int Y;
        int Z;
    };
    template class DZ_API InteropArray<Int32_3>;

    struct DZ_API Int32_4
    {
        int X;
        int Y;
        int Z;
        int W;
    };
    template class DZ_API InteropArray<Int32_4>;

    struct DZ_API UInt16_2
    {
        uint16_t X;
        uint16_t Y;
    };
    template class DZ_API InteropArray<UInt16_2>;

    struct DZ_API UInt16_3
    {
        uint16_t X;
        uint16_t Y;
        uint16_t Z;
    };
    template class DZ_API InteropArray<UInt16_3>;

    struct DZ_API UInt16_4
    {
        uint16_t X;
        uint16_t Y;
        uint16_t Z;
        uint16_t W;
    };
    template class DZ_API InteropArray<UInt16_4>;

    struct DZ_API UInt32_2
    {
        uint32_t X;
        uint32_t Y;
    };
    template class DZ_API InteropArray<UInt32_2>;

    struct DZ_API UInt32_3
    {
        uint32_t X;
        uint32_t Y;
        uint32_t Z;
    };
    template class DZ_API InteropArray<UInt32_3>;

    struct DZ_API UInt32_4
    {
        uint32_t X;
        uint32_t Y;
        uint32_t Z;
        uint32_t W;
    };
    template class DZ_API InteropArray<UInt32_4>;

    struct Float_4x4
    {
        float _11 = 1, _12 = 0, _13 = 0, _14 = 0;
        float _21 = 0, _22 = 1, _23 = 0, _24 = 0;
        float _31 = 0, _32 = 0, _33 = 1, _34 = 0;
        float _41 = 0, _42 = 0, _43 = 0, _44 = 1;

        float GetElement( const uint32_t major, const uint32_t minor ) const
        {
            if ( major >= 4 || minor >= 4 )
            {
                return 0.0f;
            }

            const float *base = &_11;
            return base[ major * 4 + minor ];
        }

        void SetElement( const uint32_t major, const uint32_t minor, const float value )
        {
            if ( major >= 4 || minor >= 4 )
            {
                return;
            }

            float *base               = &_11;
            base[ major * 4 + minor ] = value;
        }
    };
} // namespace DenOfIz
