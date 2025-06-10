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

#include <stdint.h>

namespace DenOfIz
{
    struct DZ_API ByteArray
    {
        Byte  *Elements;
        size_t NumElements;

        static ByteArray Create( const size_t NumElements )
        {
            ByteArray Array{ };
            Array.Elements    = new Byte[ NumElements ];
            Array.NumElements = NumElements;
            return Array;
        }
        
        void Dispose( ) const
        {
            delete[] Elements;
        }
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

        static ByteArrayArray Create( const size_t NumElements )
        {
            ByteArrayArray Array{ };
            Array.Elements    = new ByteArray[ NumElements ];
            Array.NumElements = NumElements;
            return Array;
        }
        
        void Dispose() const
        {
            // First dispose each ByteArray
            for (uint32_t i = 0; i < NumElements; ++i)
            {
                Elements[i].Dispose();
            }
            // Then delete the array itself
            delete[] Elements;
        }
    };

    struct DZ_API BoolArray
    {
        bool    *Elements;
        uint32_t NumElements;
        
        static BoolArray Create(const size_t NumElements)
        {
            BoolArray Array{ };
            Array.Elements = new bool[NumElements];
            Array.NumElements = NumElements;
            return Array;
        }
        
        void Dispose() const
        {
            delete[] Elements;
        }
    };

    struct DZ_API FloatArray
    {
        float   *Elements;
        uint32_t NumElements;
        
        static FloatArray Create(const size_t NumElements)
        {
            FloatArray Array{ };
            Array.Elements = new float[NumElements];
            Array.NumElements = NumElements;
            return Array;
        }
        
        void Dispose() const
        {
            delete[] Elements;
        }
    };

    struct DZ_API FloatArrayArray
    {
        FloatArray *Elements;
        uint32_t    NumElements;
        
        static FloatArrayArray Create(const size_t NumElements)
        {
            FloatArrayArray Array{ };
            Array.Elements = new FloatArray[NumElements];
            Array.NumElements = NumElements;
            return Array;
        }
        
        void Dispose() const
        {
            // First dispose each FloatArray
            for (uint32_t i = 0; i < NumElements; ++i)
            {
                Elements[i].Dispose();
            }
            // Then delete the array itself
            delete[] Elements;
        }
    };

    struct DZ_API Int32Array
    {
        int32_t *Elements;
        uint32_t NumElements;
        
        static Int32Array Create(const size_t NumElements)
        {
            Int32Array Array{ };
            Array.Elements = new int32_t[NumElements];
            Array.NumElements = NumElements;
            return Array;
        }
        
        void Dispose() const
        {
            delete[] Elements;
        }
    };

    struct DZ_API UInt16Array
    {
        uint16_t *Elements;
        uint16_t  NumElements;
        
        static UInt16Array Create(const size_t NumElements)
        {
            UInt16Array Array{ };
            Array.Elements = new uint16_t[NumElements];
            Array.NumElements = static_cast<uint16_t>(NumElements);
            return Array;
        }
        
        void Dispose() const
        {
            delete[] Elements;
        }
    };

    struct DZ_API UInt32Array
    {
        uint32_t *Elements;
        uint32_t  NumElements;
        
        static UInt32Array Create(const size_t NumElements)
        {
            UInt32Array Array{ };
            Array.Elements = new uint32_t[NumElements];
            Array.NumElements = NumElements;
            return Array;
        }
        
        void Dispose() const
        {
            delete[] Elements;
        }
    };

    struct DZ_API Int16Array
    {
        int16_t *Elements;
        uint32_t NumElements;
        
        static Int16Array Create(const size_t NumElements)
        {
            Int16Array Array{ };
            Array.Elements = new int16_t[NumElements];
            Array.NumElements = NumElements;
            return Array;
        }
        
        void Dispose() const
        {
            delete[] Elements;
        }
    };

    struct DZ_API StringView
    {
        const char *Chars;
        uint32_t    Length;
    };

    struct DZ_API StringArray
    {
        StringView *Elements;
        uint32_t    NumElements;
        
        static StringArray Create(const size_t NumElements)
        {
            StringArray Array{ };
            Array.Elements = new StringView[NumElements];
            Array.NumElements = NumElements;
            return Array;
        }
        
        void Dispose() const
        {
            delete[] Elements;
        }
    };
} // namespace DenOfIz
