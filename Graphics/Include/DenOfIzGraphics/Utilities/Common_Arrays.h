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

#include <cstdint>
#include "Interop.h"

namespace DenOfIz
{
    typedef unsigned char Byte;

    struct DZ_API ByteArray
    {
        Byte  *Elements;
        size_t NumElements;
    };

    struct DZ_API ByteArrayView
    {
        const Byte *Elements;
        size_t      NumElements;
        ByteArrayView( ) = default;
        explicit ByteArrayView( const ByteArray &Array )
        {
            Elements    = Array.Elements;
            NumElements = Array.NumElements;
        }
        ByteArrayView( const Byte *Elements, const size_t NumElements )
        {
            this->Elements    = Elements;
            this->NumElements = NumElements;
        }
    };

    struct DZ_API ByteArrayArray
    {
        ByteArray *Elements;
        uint32_t   NumElements;
    };

    struct DZ_API BoolArray
    {
        bool  *Elements;
        size_t NumElements;
    };

    struct DZ_API FloatArray
    {
        float *Elements;
        size_t NumElements;
    };

    struct DZ_API FloatArrayArray
    {
        FloatArray *Elements;
        size_t      NumElements;
    };

    struct DZ_API Int32Array
    {
        int32_t *Elements;
        size_t   NumElements;
    };

    struct DZ_API UInt16Array
    {
        uint16_t *Elements;
        size_t    NumElements;
    };

    struct DZ_API UInt32Array
    {
        uint32_t *Elements    = nullptr;
        size_t    NumElements = 0;
    };

    struct DZ_API UInt32ArrayView
    {
        uint32_t const *Elements;
        size_t          NumElements;
    };

    struct DZ_API Int16Array
    {
        int16_t *Elements;
        size_t   NumElements;
    };

    struct DZ_API StringView
    {
        const char *Chars;
        uint32_t    Length;

        StringView( ) = default;
        StringView( const char *Chars, const uint32_t Length )
        {
            this->Chars  = Chars;
            this->Length = Length;
        }
        explicit StringView( const char *CharsNullDelimited )
        {
            this->Chars  = CharsNullDelimited;
            this->Length = strlen( CharsNullDelimited );
        }
    };

    struct DZ_API StringArray
    {
        StringView *Elements;
        size_t      NumElements;
    };

    struct DZ_API InteropStringArray
    {
        InteropString const *Elements;
        size_t               NumElements;
    };
} // namespace DenOfIz
