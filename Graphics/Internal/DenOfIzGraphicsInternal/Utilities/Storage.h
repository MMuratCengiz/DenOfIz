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

#include <vector>

namespace DenOfIz
{
    class Storage
    {
        std::vector<void *> m_container;

    public:
        void Reserve( const uint32_t size )
        {
            m_container.reserve( size );
        }

        template <typename T>
        T &Store( )
        {
            T *ptr = (T *)malloc( sizeof( T ) );
            m_container.emplace_back( ptr );
            return *ptr;
        }

        template <typename T>
        T *StoreArray( size_t count )
        {
            T *ptr = (T *)malloc( sizeof( T ) * count );
            m_container.emplace_back( ptr );
            return ptr;
        }

        void Clear( )
        {
            for ( auto &item : m_container )
            {
                free( item );
            }
            m_container.clear( );
        }

        ~Storage( )
        {
            Clear( );
        }
    };
} // namespace DenOfIz
