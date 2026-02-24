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

#include "DenOfIzGraphics/Utilities/InteropUtilities.h"

#include <cstring>
#include <string>

extern "C"
{

    DenOfIz_ByteArray DenOfIz_InteropUtilities_StringToBytes( const char *str )
    {
        if ( str == NULL )
        {
            return { NULL, 0 };
        }

        const std::string stdStr       = str;
        const size_t      size         = stdStr.size( ) + 1;
        Byte             *resultBuffer = (Byte *)malloc( size );
        std::memcpy( resultBuffer, stdStr.c_str( ), size );

        DenOfIz_ByteArray result{ };
        result.Elements    = resultBuffer;
        result.NumElements = size;

        return result;
    }
}
