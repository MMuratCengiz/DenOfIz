/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
