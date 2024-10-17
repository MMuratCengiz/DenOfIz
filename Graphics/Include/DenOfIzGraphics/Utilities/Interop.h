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
        std::string m_string;

    public:
        InteropString( ) = default;
        // ReSharper disable once CppNonExplicitConvertingConstructor
        InteropString( const char *str ) // NOLINT(*-explicit-constructor)
        {
            m_string = std::string( str );
        }

        // ReSharper disable once CppNonExplicitConvertingConstructor
        InteropString( const std::string &str ) // NOLINT(*-explicit-constructor)
        {
            m_string = str;
        }

        InteropString( const InteropString &other ) : InteropString( other.m_string )
        {
        }

        [[nodiscard]] const char *CStr( ) const
        {
            return m_string.c_str( );
        }

        [[nodiscard]] std::string Str( ) const
        {
            return m_string;
        }
    };

    // [] operator is intentionally omitted to have a consisted interface with other languages
    template <class T>
    class InteropArray
    {
        std::vector<T> m_array;

    public:
        DZ_API    InteropArray( ) = default;
        DZ_API T &EmplaceElement( )
        {
            return m_array.emplace_back( );
        }

        DZ_API T &AddElement( const T &element )
        {
            return m_array.emplace_back( element );
        }

        [[nodiscard]] DZ_API const T &GetElement( const size_t index ) const
        {
            return m_array[ index ];
        }

        [[nodiscard]] DZ_API T &GetElement( const size_t index )
        {
            return m_array[ index ];
        }

        DZ_API void SetElement( const size_t index, const T &element )
        {
            m_array[ index ] = element;
        }

        DZ_API void SetNumElements( const size_t numElements )
        {
            m_array.resize( numElements );
        }

        [[nodiscard]] DZ_API size_t NumElements( ) const
        {
            return m_array.size( );
        }
    };
} // namespace DenOfIz
