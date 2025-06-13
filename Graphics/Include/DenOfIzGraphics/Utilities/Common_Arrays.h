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

#include <stdint.h>
#include "Interop.h"

namespace DenOfIz
{
#define DZ_ARRAY_METHODS( ARRAY_TYPE, ELEMENT_TYPE )                                                                                                                               \
    static ARRAY_TYPE Create( const size_t NumElements )                                                                                                                           \
    {                                                                                                                                                                              \
        ARRAY_TYPE Array{ };                                                                                                                                                       \
        Array.Elements    = new ELEMENT_TYPE[ NumElements ];                                                                                                                       \
        Array.NumElements = NumElements;                                                                                                                                           \
        return Array;                                                                                                                                                              \
    }                                                                                                                                                                              \
                                                                                                                                                                                   \
    void Dispose( ) const                                                                                                                                                          \
    {                                                                                                                                                                              \
        delete[] Elements;                                                                                                                                                         \
    }

#define DZ_ARRAY_ARRAY_METHODS( ARRAY_TYPE, ELEMENT_TYPE )                                                                                                                         \
    static ARRAY_TYPE Create( const size_t NumElements )                                                                                                                           \
    {                                                                                                                                                                              \
        ARRAY_TYPE Array{ };                                                                                                                                                       \
        Array.Elements    = new ELEMENT_TYPE[ NumElements ];                                                                                                                       \
        Array.NumElements = NumElements;                                                                                                                                           \
        return Array;                                                                                                                                                              \
    }                                                                                                                                                                              \
                                                                                                                                                                                   \
    void Dispose( ) const                                                                                                                                                          \
    {                                                                                                                                                                              \
        for ( uint32_t i = 0; i < NumElements; ++i )                                                                                                                               \
        {                                                                                                                                                                          \
            Elements[ i ].Dispose( );                                                                                                                                              \
        }                                                                                                                                                                          \
        delete[] Elements;                                                                                                                                                         \
    }

    struct DZ_API ByteArray
    {
        Byte  *Elements;
        size_t NumElements;

        DZ_ARRAY_METHODS( ByteArray, Byte )
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

        DZ_ARRAY_ARRAY_METHODS( ByteArrayArray, ByteArray )
    };

    struct DZ_API BoolArray
    {
        bool    *Elements;
        uint32_t NumElements;

        static BoolArray Create( const size_t NumElements )
        {
            BoolArray Array{ };
            Array.Elements    = new bool[ NumElements ];
            Array.NumElements = NumElements;
            return Array;
        }

        void Dispose( ) const
        {
            delete[] Elements;
        }
    };

    struct DZ_API FloatArray
    {
        float   *Elements;
        uint32_t NumElements;

        DZ_ARRAY_METHODS( FloatArray, float )
    };

    struct DZ_API FloatArrayArray
    {
        FloatArray *Elements;
        uint32_t    NumElements;

        DZ_ARRAY_ARRAY_METHODS( FloatArrayArray, FloatArray )
    };

    struct DZ_API Int32Array
    {
        int32_t *Elements;
        uint32_t NumElements;

        DZ_ARRAY_METHODS( Int32Array, int32_t )
    };

    struct DZ_API UInt16Array
    {
        uint16_t *Elements;
        uint16_t  NumElements;

        DZ_ARRAY_METHODS( UInt16Array, uint16_t )
    };

    struct DZ_API UInt32Array
    {
        uint32_t *Elements;
        uint32_t  NumElements;

        DZ_ARRAY_METHODS( UInt32Array, uint32_t )
    };

    struct DZ_API UInt32ArrayView
    {
        uint32_t const *Elements;
        size_t          NumElements;
    };

    struct DZ_API Int16Array
    {
        int16_t *Elements;
        uint32_t NumElements;

        DZ_ARRAY_METHODS( Int16Array, int16_t )
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
        uint32_t    NumElements;

        DZ_ARRAY_METHODS( StringArray, StringView )
    };

    struct DZ_API InteropStringArray
    {
        InteropString *Elements;
        uint32_t       NumElements;

        DZ_ARRAY_METHODS( InteropStringArray, InteropString )
    };
} // namespace DenOfIz
