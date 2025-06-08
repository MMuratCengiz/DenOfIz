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

using namespace DenOfIz;

ByteArray InteropUtilities::StringToBytes( const char *str )
{
    if ( !str )
    {
        return { nullptr, 0 };
    }

    const std::string stdStr = str;
    ByteArray         result{ };

    const size_t size = stdStr.size( ) + 1;
    result.Elements   = static_cast<Byte *>( std::malloc( size ) );

    if ( !result.Elements )
    {
        result.NumElements = 0;
        return result;
    }

    result.NumElements = size;
    std::memcpy( result.Elements, stdStr.c_str( ), size );
    return result;
}
