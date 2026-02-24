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

#include <gtest/gtest.h>
#include "DenOfIzGraphics/Utilities/Interop.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"

using namespace DenOfIz;

template <typename T>
void AssertArrayEq( const T *arr1, const T *arr2, size_t size )
{
    for ( size_t i = 0; i < size; ++i )
    {
        ASSERT_EQ( arr1[ i ], arr2[ i ] );
    }
}

inline bool FloatEquals( const float a, const float b, const float epsilon = 0.00001f )
{
    return std::abs( a - b ) < epsilon;
}

inline bool Float3Equals( const DenOfIz_Float3 &a, const DenOfIz_Float3 &b, const float tolerance = 1e-5f )
{
    return std::abs( a.X - b.X ) < tolerance && std::abs( a.Y - b.Y ) < tolerance && std::abs( a.Z - b.Z ) < tolerance;
}

inline bool Float4Equals( const DenOfIz_Float4 &a, const DenOfIz_Float4 &b, const float epsilon = 0.00001f )
{
    return FloatEquals( a.X, b.X, epsilon ) && FloatEquals( a.Y, b.Y, epsilon ) && FloatEquals( a.Z, b.Z, epsilon ) && FloatEquals( a.W, b.W, epsilon );
}

inline bool MatricesEqual( const DenOfIz_Float4x4 &a, const DenOfIz_Float4x4 &b, const float epsilon = 0.00001f )
{
    for ( int i = 0; i < 4; ++i )
    {
        for ( int j = 0; j < 4; ++j )
        {
            const float *r_mat4_base = &a._11;
            const float *mat4_base   = &b._11;
            if ( std::abs( r_mat4_base[ i * 4 + j ] - mat4_base[ i * 4 + j ] ) > epsilon )
            {
                return false;
            }
        }
    }
    return true;
}

inline bool Vector3Equal( const DenOfIz_Float3 &a, const DenOfIz_Float3 &b, const float epsilon = 0.00001f )
{
    return std::abs( a.X - b.X ) < epsilon && std::abs( a.Y - b.Y ) < epsilon && std::abs( a.Z - b.Z ) < epsilon;
}
