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
        char *m_data = nullptr;

    public:
        InteropString( const char *str = nullptr )
        {
            if ( str )
            {
                size_t len = strlen( str );
                m_data     = new char[ len + 1 ];
                strcpy_s( m_data, len, str );
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
                strcpy_s( m_data, len, other.m_data );
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
                    strcpy_s( m_data, len, other.m_data );
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
                strcpy_s( newData, oldLen, m_data );
                delete[] m_data;
            }
            strcpy_s( newData + oldLen, len, str );
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

    public:
        InteropArray( size_t numElements = 0 ) : m_numElements( numElements ), m_array( numElements ? new T[ numElements ] : nullptr )
        {
        }

        ~InteropArray( )
        {
            delete[] m_array;
        }

        InteropArray( const InteropArray &other )
        {
            m_numElements = other.m_numElements;
            if ( m_numElements != 0 )
            {
                m_array = new T[ m_numElements ];
                for ( size_t i = 0; i < m_numElements; ++i )
                {
                    m_array[ i ] = other.m_array[ i ];
                }
            }
        }

        InteropArray &operator=( const InteropArray &other )
        {
            if ( this != &other )
            {
                delete[] m_array; // Free existing data
                m_numElements = other.m_numElements;
                m_array       = new T[ m_numElements ];
                for ( size_t i = 0; i < m_numElements; ++i )
                {
                    m_array[ i ] = other.m_array[ i ];
                }
            }
            return *this;
        }

        InteropArray( InteropArray &&other ) noexcept : m_numElements( other.m_numElements ), m_array( other.m_array )
        {
            other.m_numElements = 0;
            other.m_array       = nullptr;
        }

        InteropArray &operator=( InteropArray &&other ) noexcept
        {
            if ( this != &other )
            {
                delete[] m_array; // Free existing data
                m_numElements       = other.m_numElements;
                m_array             = other.m_array;
                other.m_numElements = 0;
                other.m_array       = nullptr; // Nullify the moved-from object
            }
            return *this;
        }

        T &EmplaceElement( )
        {
            size_t index = m_numElements;
            Resize( m_numElements );
            return m_array[ index ] = T( );
        }

        T &AddElement( const T &element )
        {
            size_t index = m_numElements;
            Resize( m_numElements );
            return m_array[ index ] = element;
        }

        void Swap( int index1, int index2 )
        {
            T temp            = m_array[ index1 ];
            m_array[ index1 ] = m_array[ index2 ];
            m_array[ index2 ] = temp;
        }

        [[nodiscard]] const T &GetElement( const size_t index ) const
        {
            return m_array[ index ];
        }

        [[nodiscard]] T &GetElement( const size_t index )
        {
            return m_array[ index ];
        }

        void SetElement( const size_t index, const T &element )
        {
            static_assert( std::is_copy_assignable<T>::value, "T must be copy assignable for this function to work." );
            m_array[ index ] = element;
        }

        // For rvalue references (movable objects)
        void SetElement( const size_t index, T &&element )
        {
            m_array[ index ] = std::move( element );
        }

        const T *Data( ) const
        {
            return m_array;
        }

        [[nodiscard]] size_t NumElements( ) const
        {
            return m_numElements;
        }

        void Resize( const size_t numElements )
        {
            if ( m_numElements <= numElements )
            {
                // Resize the array
                m_numElements = numElements + ( numElements / 2 );
                T *newArray   = new T[ m_numElements ];
                for ( size_t i = 0; i < m_numElements; ++i )
                {
                    newArray[ i ] = std::move( m_array[ i ] );
                }
                delete[] m_array;
            }
        }
    };

    template class DZ_API InteropArray<InteropString>;
    template class DZ_API InteropArray<int>;
    template class DZ_API InteropArray<float>;
} // namespace DenOfIz
