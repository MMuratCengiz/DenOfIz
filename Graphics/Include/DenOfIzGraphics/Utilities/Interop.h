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
#define DZ_MAX_INTEROP_STRING_SIZE 512
    class DZ_API InteropString
    {
    private:
        char m_string[ DZ_MAX_INTEROP_STRING_SIZE ] = { 0 };

    public:
        InteropString( ) = default;
        // Use bounded char array for interop
        InteropString( const char bounded[ DZ_MAX_INTEROP_STRING_SIZE ], size_t numChars = DZ_MAX_INTEROP_STRING_SIZE )
        {
            if ( numChars > DZ_MAX_INTEROP_STRING_SIZE )
            {
                numChars = DZ_MAX_INTEROP_STRING_SIZE;
            }
            strncpy_s( m_string, bounded, numChars );
        }

        InteropString( const std::string &str )
        {
            strncpy_s( m_string, str.c_str(), DZ_MAX_INTEROP_STRING_SIZE );
        }

        InteropString( const InteropString &other ) : InteropString( other.m_string )
        {
        }

        const char *CStr( ) const
        {
            return m_string;
        }

        std::string Str( ) const
        {
            return m_string;
        }
    };
} // namespace DenOfIz
