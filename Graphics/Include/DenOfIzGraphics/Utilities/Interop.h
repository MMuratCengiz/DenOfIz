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

#include <string>
#include <vector>
#include <glog/logging.h>

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

namespace DenOfIz
{
    class DZ_API InteropString
    {
        char     *m_data              = nullptr;
        const int s_nullTerminatorLen = 1;

    public:
        InteropString( const char *str = nullptr )
        {
            if ( str )
            {
                size_t len = strlen( str );
                m_data     = new char[ len + 1 ];
                strcpy_s( m_data, len + s_nullTerminatorLen, str );
            }
        }

        ~InteropString( )
        {
            delete[] m_data;
        }

        InteropString( const InteropString &other )
        {
            if ( other.m_data )
            {
                size_t len = strlen( other.m_data );
                m_data     = new char[ len + 1 ];
                strcpy_s( m_data, len + s_nullTerminatorLen, other.m_data );
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
                delete[] m_data; // Free existing data
                if ( other.m_data )
                {
                    size_t len = strlen( other.m_data );
                    m_data     = new char[ len + 1 ];
                    strcpy_s( m_data, len + s_nullTerminatorLen, other.m_data );
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
            size_t len     = strlen( str );
            size_t oldLen  = NumChars( );
            char  *newData = new char[ oldLen + len + 1 ];
            if ( m_data != nullptr )
            {
                strcpy_s( newData, oldLen + s_nullTerminatorLen, m_data );
                delete[] m_data;
            }
            strcpy_s( newData + oldLen, len + s_nullTerminatorLen, str );
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
            return m_data;
        }
    };

    // [] operator is intentionally omitted to have a consisted interface with other languages
    template <class T>
    class DZ_API InteropArray final
    {
        T     *m_array       = nullptr;
        size_t m_numElements = 0;
        size_t m_capacity    = 10;
        size_t m_size        = 0;

    public:
        InteropArray( size_t size = 0 )
        {
            if ( size > m_capacity )
            {
                m_capacity = size;
            }
            m_size        = size;
            m_numElements = 0;
            m_array       = new T[ m_capacity ];
        }

        ~InteropArray( )
        {
            delete[] m_array;
        }

        InteropArray( const InteropArray &other )
        {
            Clear( *this );
            CopyFrom( other );
        }

        InteropArray &operator=( const InteropArray &other )
        {
            if ( this != &other )
            {
                Clear( *this );
                CopyFrom( other );
            }
            return *this;
        }

        InteropArray( InteropArray &&other ) noexcept
        {
            Clear( *this );
            CopyFrom( other );
            Clear( other );
        }

        InteropArray &operator=( InteropArray &&other ) noexcept
        {
            if ( this != &other )
            {
                Clear( *this );
                CopyFrom( other );
                Clear( other );
            }
            return *this;
        }

        void CopyFrom( const InteropArray &other )
        {
            m_size        = other.m_size;
            m_capacity    = other.m_capacity;
            m_numElements = other.m_numElements;
            if ( m_capacity > 0 )
            {
                m_array = new T[ m_capacity ];
                CopyArray( other.m_array, m_array, m_numElements );
            }
        }

        void Clear( InteropArray &arr )
        {
            delete[] arr.m_array;
            arr.m_array       = nullptr;
            arr.m_numElements = 0;
            arr.m_capacity    = 0;
            arr.m_size        = 0;
        }

        T &EmplaceElement( )
        {
            return m_array[ NewElement( ) ] = T( );
        }

        T &AddElement( const T &element )
        {
            return m_array[ NewElement( ) ] = element;
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
            m_array[ index ] = element;
        }

        // For rvalue references (movable objects)
        void SetElement( const size_t index, T &&element )
        {
            CheckBounds( index );
            m_array[ index ] = std::move( element );
            if ( index >= m_numElements )
            {
                m_numElements = index + 1;
            }
        }

        [[nodiscard]] const T *Data( ) const
        {
            return m_array;
        }

        [[nodiscard]] size_t NumElements( ) const
        {
            return m_numElements;
        }

        void Resize( const size_t size )
        {
            if ( m_capacity <= size )
            {
                m_capacity  = m_capacity + ( m_capacity / 2 );
                T *newArray = new T[ m_capacity ];
                CopyArray( m_array, newArray, m_numElements );
                delete[] m_array;
                m_array = newArray;
            }
            m_size = size;
        }

    private:
        void CopyArray( const T *src, T *dst, const size_t numElements )
        {
            std::copy( std::make_move_iterator( src ), std::make_move_iterator( src + numElements ), dst );
        }
        size_t NewElement( )
        {
            size_t index = m_numElements;
            Resize( index + 1 );
            m_numElements++;
            return index;
        }

        void CheckBounds( const size_t index ) const
        {
            if ( index >= m_size )
            {
                LOG( ERROR ) << "Index out of bounds." << index << " >= " << m_size;
            }
        }
    };

    template class DZ_API InteropArray<InteropString>;
} // namespace DenOfIz
