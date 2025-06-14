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

#include <algorithm>
#include <cctype>
#include <cstring>
#include <iostream>

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

#include <type_traits>

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
                const size_t len = std::strlen( str );
                m_data           = new char[ len + 1 ];
                SafeCopyString( m_data, len + s_nullTerminatorLen, str );
            }
        }

        InteropString( const char *str, const size_t len )
        {
            if ( str )
            {
                m_data = new char[ len + 1 ];
                std::memcpy( m_data, str, len );
                m_data[ len ] = '\0';
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

        [[nodiscard]] bool Equals( const InteropString &other ) const
        {
            if ( this == &other )
            {
                return true;
            }

            if ( m_data == nullptr && other.m_data == nullptr )
            {
                return true;
            }

            if ( m_data == nullptr || other.m_data == nullptr )
            {
                return false;
            }

            return strcmp( m_data, other.m_data ) == 0;
        }

        [[nodiscard]] InteropString ToLower( ) const
        {
            if ( m_data == nullptr )
            {
                return { };
            }
            InteropString result( m_data );
            for ( size_t i = 0; i < strlen( result.m_data ); ++i )
            {
                result.m_data[ i ] = static_cast<char>( tolower( result.m_data[ i ] ) );
            }
            return std::move( result );
        }

        [[nodiscard]] InteropString ToUpper( ) const
        {
            if ( m_data == nullptr )
            {
                return { };
            }
            InteropString result( m_data );
            for ( size_t i = 0; i < strlen( result.m_data ); ++i )
            {
                result.m_data[ i ] = static_cast<char>( toupper( result.m_data[ i ] ) );
            }
            return std::move( result );
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

        InteropString Append( const char *str )
        {
            if ( str == nullptr )
            {
                return *this;
            }
            const size_t  len    = strlen( str );
            const size_t  oldLen = NumChars( );
            InteropString result;
            const auto    newData = new char[ oldLen + len + s_nullTerminatorLen ];
            if ( m_data != nullptr )
            {
                SafeCopyString( newData, oldLen + s_nullTerminatorLen, m_data );
            }
            SafeCopyString( newData + oldLen, len + s_nullTerminatorLen, str );
            delete[] result.m_data;
            result.m_data = newData;
            return std::move( result );
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
} // namespace DenOfIz
