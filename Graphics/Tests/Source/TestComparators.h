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

#include "DenOfIzGraphics/Utilities/Interop.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"
#include <gtest/gtest.h>

using namespace DenOfIz;

template <typename T>
void AssertInteropArrayEq( const InteropArray<T> &arr1, const InteropArray<T> &arr2 )
{
    ASSERT_EQ( arr1.NumElements( ), arr2.NumElements( ) );
    for ( size_t i = 0; i < arr1.NumElements( ); ++i )
    {
        ASSERT_EQ( arr1.GetElement( i ), arr2.GetElement( i ) );
    }
}

inline bool FloatEquals( const float a, const float b, const float epsilon = 0.00001f )
{
    return std::abs( a - b ) < epsilon;
}

inline bool Float3Equals( const Float_3 &a, const Float_3 &b, const float tolerance = 1e-5f )
{
    return std::abs( a.X - b.X ) < tolerance && std::abs( a.Y - b.Y ) < tolerance && std::abs( a.Z - b.Z ) < tolerance;
}

inline bool Float4Equals( const Float_4 &a, const Float_4 &b, const float epsilon = 0.00001f )
{
    return FloatEquals( a.X, b.X, epsilon ) && FloatEquals( a.Y, b.Y, epsilon ) && FloatEquals( a.Z, b.Z, epsilon ) && FloatEquals( a.W, b.W, epsilon );
}

inline bool MatricesEqual( const Float_4x4 &a, const Float_4x4 &b, const float epsilon = 0.00001f )
{
    for ( int i = 0; i < 4; ++i )
    {
        for ( int j = 0; j < 4; ++j )
        {
            if ( std::abs( a.GetElement( i, j ) - b.GetElement( i, j ) ) > epsilon )
            {
                return false;
            }
        }
    }
    return true;
}

inline bool Vector3Equal( const Float_3 &a, const Float_3 &b, const float epsilon = 0.00001f )
{
    return std::abs( a.X - b.X ) < epsilon && std::abs( a.Y - b.Y ) < epsilon && std::abs( a.Z - b.Z ) < epsilon;
}
