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

#include <glog/logging.h>
#include <iterator>
#include <mutex>
#include <string>

#ifdef _WIN32
#ifdef DZ_GRAPHICS_EXPORTS
#define DZ_API __declspec( dllexport )
#elif DZ_GRAPHICS_IMPORTS
#define DZ_API __declspec( dllimport )
#endif
#endif

#ifndef DZ_API
#define DZ_API
#endif

#if _WIN32
#define SafeCopyString( dst, dstSize, src ) strcpy_s( dst, dstSize, src )
#elif __APPLE__ || __linux__
#define SafeCopyString( dst, dstSize, src ) strncpy( dst, src, dstSize )
#endif

namespace DenOfIz
{
    class DZ_API InteropString
    {
        char     *m_data              = nullptr;
        const int s_nullTerminatorLen = 1;

    public:
        // ReSharper disable once CppNonExplicitConvertingConstructor
        InteropString( const char *str = nullptr ) // NOLINT(*-explicit-constructor)
        {
            if ( str )
            {
                const size_t len = strlen( str );
                m_data           = new char[ len + 1 ];
                SafeCopyString( m_data, len + s_nullTerminatorLen, str );
            }
        }

        ~InteropString( )
        {
            delete[] m_data;
            m_data = nullptr;
        }

        InteropString( const InteropString &other )
        {
            if ( other.m_data )
            {
                const size_t len = strlen( other.m_data );
                m_data           = new char[ len + 1 ];
                SafeCopyString( m_data, len + s_nullTerminatorLen, other.m_data );
            }
            else
            {
                m_data = nullptr;
            }
        }

        InteropString &operator=( const InteropString &other )
        {
            if ( this != &other )
            {
                delete[] m_data;
                m_data = nullptr;
                if ( other.m_data )
                {
                    const size_t len = strlen( other.m_data );
                    m_data           = new char[ len + 1 ];
                    SafeCopyString( m_data, len + s_nullTerminatorLen, other.m_data );
                }
                else
                {
                    m_data = nullptr;
                }
            }
            return *this;
        }

        InteropString( InteropString &&other ) noexcept : m_data( other.m_data )
        {
            other.m_data = nullptr;
        }

        InteropString &operator=( InteropString &&other ) noexcept
        {
            if ( this != &other )
            {
                delete[] m_data;
                m_data       = other.m_data;
                other.m_data = nullptr;
            }
            return *this;
        }

        InteropString &Append( const char *str )
        {
            if ( str == nullptr )
            {
                return *this;
            }
            const size_t len     = strlen( str );
            const size_t oldLen  = NumChars( );
            char        *newData = new char[ oldLen + len + 1 ];
            if ( m_data != nullptr )
            {
                SafeCopyString( newData, oldLen + s_nullTerminatorLen, m_data );
                delete[] m_data;
                m_data = nullptr;
            }
            SafeCopyString( newData + oldLen, len + s_nullTerminatorLen, str );
            m_data = newData;
            return *this;
        }

        [[nodiscard]] size_t NumChars( ) const
        {
            if ( m_data == nullptr )
            {
                return 0;
            }
            return strlen( m_data );
        }

        [[nodiscard]] bool IsEmpty( ) const
        {
            return m_data == nullptr || strlen( m_data ) == 0;
        }

        [[nodiscard]] const char *Get( ) const
        {
            if ( m_data == nullptr )
            {
                return "";
            }
            return m_data;
        }
    };

    // [] operator is intentionally omitted to have a consisted interface with other languages
    template <class T>
    class DZ_API InteropArray final
    {
        T     *m_array       = nullptr;
        size_t m_numElements = 0;
        size_t m_capacity    = 8;

    public:
        // ReSharper disable once CppNonExplicitConvertingConstructor
        InteropArray( const size_t numElements = 0 ) // NOLINT(*-explicit-constructor)
        {
            if ( numElements > m_capacity )
            {
                m_capacity = numElements + 1;
            }
            m_numElements = numElements;
            m_array       = new T[ m_capacity ];
            InitializeElements( m_array, 0, m_capacity );
        }

        ~InteropArray( )
        {
            if ( m_array != nullptr )
            {
                DestroyElements( m_array, m_numElements );
                delete[] m_array;
                m_array = nullptr;
            }
        }

        InteropArray( const InteropArray &other )
        {
            if ( this != &other )
            {
                CopyFrom( other );
            }
        }

        InteropArray &operator=( const InteropArray &other )
        {
            if ( this != &other )
            {
                Clear( );
                CopyFrom( other );
            }
            return *this;
        }

        InteropArray( InteropArray &&other ) noexcept
        {
            if ( this != &other )
            {
                MoveFrom( other );
            }
        }

        InteropArray &operator=( InteropArray &&other ) noexcept
        {
            if ( this != &other )
            {
                Clear( );
                MoveFrom( other );
            }
            return *this;
        }

        void MoveFrom( InteropArray &other )
        {
            m_array       = other.m_array;
            m_capacity    = other.m_capacity;
            m_numElements = other.m_numElements;

            other.m_array       = nullptr;
            other.m_capacity    = 0;
            other.m_numElements = 0;
        }

        void CopyFrom( const InteropArray &other )
        {
            Clear( );
            m_capacity    = other.m_capacity;
            m_numElements = other.m_numElements;
            m_array       = new T[ m_capacity ];
            CopyArray( other.m_array, m_array, m_numElements );
        }

        void Clear( )
        {
            if ( m_array )
            {
                DestroyElements( m_array, m_numElements );
                delete[] m_array;
                m_array = nullptr;
            }
            m_numElements = 0;
            m_capacity    = 0;
        }

        T &EmplaceElement( )
        {
            size_t index = NewElement( );
            return m_array[ index ];
        }

        void AddElement( const T &element )
        {
            size_t index     = NewElement( );
            m_array[ index ] = element;
        }

        void Swap( int index1, int index2 )
        {
            CheckBounds( index1 );
            CheckBounds( index2 );

            T temp            = m_array[ index1 ];
            m_array[ index1 ] = m_array[ index2 ];
            m_array[ index2 ] = temp;
        }

        [[nodiscard]] const T &GetElement( const size_t index ) const
        {
            CheckBounds( index );
            return m_array[ index ];
        }

        [[nodiscard]] T &GetElement( const size_t index )
        {
            CheckBounds( index );
            return m_array[ index ];
        }

        void SetElement( const size_t index, const T &element )
        {
            CheckBounds( index );
            if ( m_numElements <= index )
            {
                m_numElements = index + 1;
            }
            m_array[ index ] = element;
        }

        void CopyBytes( unsigned char *inputBytes, int nbytes )
        {
            MemCpy( inputBytes, nbytes );
        }

        void MemCpy( const void *src, const size_t numBytes )
        {
            const size_t numElements = numBytes / sizeof( T );
            Resize( numElements );
            std::memcpy( m_array, src, numBytes );
            m_numElements = numElements;
            m_capacity    = numElements;
        }

        [[nodiscard]] const T *Data( ) const
        {
            return m_array;
        }

        [[nodiscard]] T *Data( )
        {
            return m_array;
        }

        [[nodiscard]] size_t NumElements( ) const
        {
            return m_numElements;
        }

        void Resize( const size_t size )
        {
            if ( m_capacity < size )
            {
                m_capacity = m_capacity * 2;
                if ( m_capacity < size )
                {
                    m_capacity = size;
                    m_capacity = ( m_capacity + 7 ) & ( ~7 );
                }
                T *newArray = new T[ m_capacity ];
                if ( m_array )
                {
                    MoveArray( m_array, newArray, m_numElements );
                    DestroyElements( m_array, m_numElements );
                    delete[] m_array;
                }

                if ( size > m_numElements )
                {
                    InitializeElements( newArray, m_numElements, size );
                }
                m_array = newArray;
            }
            else if ( size > m_numElements )
            {
                InitializeElements( m_array, m_numElements, size );
            }
            else if ( size < m_numElements )
            {
                DestroyElements( m_array + size, m_numElements - size );
            }
            m_numElements = size;
        }

    private:
        void InitializeElements( T *array, size_t start, const size_t end )
        {
            if constexpr ( std::is_trivially_default_constructible_v<T> )
            {
                std::memset( array + start, 0, ( end - start ) * sizeof( T ) );
            }
            else
            {
                for ( size_t i = start; i < end; ++i )
                {
                    new ( array + i ) T( );
                }
            }
        }

        void DestroyElements( T *array, const size_t count )
        {
            if constexpr ( !std::is_trivially_destructible_v<T> )
            {
                for ( size_t i = 0; i < count; ++i )
                {
                    array[ i ].~T( );
                }
            }
        }

        void CopyArray( const T *src, T *dst, const size_t numElements )
        {
            std::copy( src, src + numElements, dst );
        }

        void MoveArray( T *src, T *dst, const size_t numElements )
        {
            std::move( src, src + numElements, dst );
        }

        size_t NewElement( )
        {
            const size_t index = m_numElements;
            Resize( index + 1 );
            return index;
        }

        void CheckBounds( const size_t index ) const
        {
            if ( index >= m_numElements )
            {
                LOG( FATAL ) << "Index out of bounds." << index << " >= " << m_numElements;
            }
        }
    };

    template <>
    inline void InteropArray<InteropString>::InitializeElements( InteropString *array, size_t start, size_t end )
    {
        for ( size_t i = start; i < end; ++i )
        {
            new ( array + i ) InteropString( );
        }
    }
} // namespace DenOfIz
