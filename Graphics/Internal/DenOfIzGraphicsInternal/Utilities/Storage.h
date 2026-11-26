/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
