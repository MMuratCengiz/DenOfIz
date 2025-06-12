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

#include <new>
#include <type_traits>
#include "DenOfIzGraphics/Utilities/DZArena.h"

namespace DenOfIz
{
    // Internal templated helper for DZArena, do not include in public header files
    template <typename T>
    class DZArenaAllocator
    {
    public:
        static T *Allocate( DZArena &arena, size_t count = 1 )
        {
            return reinterpret_cast<T *>( arena.Allocate( count * sizeof( T ), alignof( T ) ) );
        }

        static T *AllocateAndConstruct( DZArena &arena, size_t count = 1 )
        {
            T *ptr = Allocate( arena, count );
            for ( size_t i = 0; i < count; ++i )
            {
                new ( &ptr[ i ] ) T( );
            }
            return ptr;
        }

        template <typename... Args>
        static T *AllocateAndConstruct( DZArena &arena, size_t count, Args &&...args )
        {
            T *ptr = Allocate( arena, count );
            for ( size_t i = 0; i < count; ++i )
            {
                new ( &ptr[ i ] ) T( std::forward<Args>( args )... );
            }
            return ptr;
        }

        static T *AllocateAndCopy( DZArena &arena, const T *source, size_t count )
        {
            T *ptr = Allocate( arena, count );
            if constexpr ( std::is_trivially_copyable_v<T> )
            {
                std::memcpy( ptr, source, count * sizeof( T ) );
            }
            else
            {
                for ( size_t i = 0; i < count; ++i )
                {
                    new ( &ptr[ i ] ) T( source[ i ] );
                }
            }
            return ptr;
        }
    };

    template <typename ArrayType, typename ElementType>
    struct DZArenaArrayHelper
    {
        static void AllocateArray( DZArena &arena, ArrayType &array, size_t count )
        {
            array.NumElements = static_cast<decltype( array.NumElements )>( count );
            array.Elements    = DZArenaAllocator<ElementType>::Allocate( arena, count );
        }

        static void AllocateAndConstructArray( DZArena &arena, ArrayType &array, size_t count )
        {
            array.NumElements = static_cast<decltype( array.NumElements )>( count );
            array.Elements    = DZArenaAllocator<ElementType>::AllocateAndConstruct( arena, count );
        }

        template <typename... Args>
        static void AllocateAndConstructArray( DZArena &arena, ArrayType &array, size_t count, Args &&...args )
        {
            array.NumElements = static_cast<decltype( array.NumElements )>( count );
            array.Elements    = DZArenaAllocator<ElementType>::AllocateAndConstruct( arena, count, std::forward<Args>( args )... );
        }

        static void AllocateAndCopyArray( DZArena &arena, ArrayType &array, const ElementType *source, size_t count )
        {
            array.NumElements = static_cast<decltype( array.NumElements )>( count );
            array.Elements    = DZArenaAllocator<ElementType>::AllocateAndCopy( arena, source, count );
        }
    };

    template <typename T>
    class DZArenaVector
    {
        DZArena *m_arena;
        T       *m_data;
        size_t   m_size;
        size_t   m_capacity;
    public:
        DZArenaVector( DZArena *arena ) : m_arena( arena ), m_data( nullptr ), m_size( 0 ), m_capacity( 0 )
        {
        }

        void Reserve( size_t newCapacity )
        {
            if ( newCapacity <= m_capacity )
            {
                return;
            }

            T *newData = DZArenaAllocator<T>::Allocate( *m_arena, newCapacity );

            if ( m_data )
            {
                if constexpr ( std::is_trivially_copyable_v<T> )
                {
                    std::memcpy( newData, m_data, m_size * sizeof( T ) );
                }
                else
                {
                    for ( size_t i = 0; i < m_size; ++i )
                    {
                        new ( &newData[ i ] ) T( std::move( m_data[ i ] ) );
                        m_data[ i ].~T( );
                    }
                }
            }

            m_data     = newData;
            m_capacity = newCapacity;
        }

        void PushBack( const T &value )
        {
            if ( m_size >= m_capacity )
            {
                Reserve( m_capacity == 0 ? 4 : m_capacity * 2 );
            }

            new ( &m_data[ m_size ] ) T( value );
            ++m_size;
        }

        template <typename... Args>
        void EmplaceBack( Args &&...args )
        {
            if ( m_size >= m_capacity )
            {
                Reserve( m_capacity == 0 ? 4 : m_capacity * 2 );
            }

            new ( &m_data[ m_size ] ) T( std::forward<Args>( args )... );
            ++m_size;
        }

        T &operator[]( size_t index )
        {
            return m_data[ index ];
        }
        const T &operator[]( size_t index ) const
        {
            return m_data[ index ];
        }

        T *Data( )
        {
            return m_data;
        }
        const T *Data( ) const
        {
            return m_data;
        }
        size_t Size( ) const
        {
            return m_size;
        }
        size_t Capacity( ) const
        {
            return m_capacity;
        }
    };

    class DZArenaString
    {
    public:
        static char *Duplicate( DZArena &arena, const char *str )
        {
            if ( !str )
            {
                return nullptr;
            }

            size_t len    = std::strlen( str ) + 1;
            char  *result = DZArenaAllocator<char>::Allocate( arena, len );
            std::memcpy( result, str, len );
            return result;
        }

        static char *Concatenate( DZArena &arena, const char *str1, const char *str2 )
        {
            size_t len1 = str1 ? std::strlen( str1 ) : 0;
            size_t len2 = str2 ? std::strlen( str2 ) : 0;

            char *result = DZArenaAllocator<char>::Allocate( arena, len1 + len2 + 1 );

            if ( str1 )
            {
                std::memcpy( result, str1, len1 );
            }
            if ( str2 )
            {
                std::memcpy( result + len1, str2, len2 );
            }

            result[ len1 + len2 ] = '\0';
            return result;
        }
    };
} // namespace DenOfIz
