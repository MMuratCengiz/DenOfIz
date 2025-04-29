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

#include <cstdint>
#include <initializer_list>

namespace DenOfIz
{
    template <typename Enum>
    class BitSet
    {
        uint32_t m_bits;

        explicit BitSet( const uint32_t bits ) : m_bits( bits )
        {
        }

    public:
        BitSet( ) : m_bits( 0 )
        {
        }

        explicit BitSet( const Enum &en ) : m_bits( static_cast<uint32_t>( en ) )
        {
        }

        explicit BitSet( const uint32_t &bits ) : m_bits( bits )
        {

        }

        explicit constexpr operator Enum( ) const
        {
            return static_cast<Enum>( m_bits );
        }

        constexpr BitSet operator|( const Enum &other )
        {
            return BitSet( m_bits | static_cast<uint32_t>( other ) );
        }

        constexpr BitSet operator|( const BitSet &other )
        {
            return BitSet( m_bits | other.m_bits );
        }

        BitSet &operator|=( const Enum &other )
        {
            m_bits |= static_cast<uint32_t>( other );
            return *this;
        }

        constexpr BitSet operator&( const Enum &other )
        {
            return BitSet( m_bits & static_cast<uint32_t>( other ) );
        }

        constexpr BitSet operator&( const BitSet &other )
        {
            return BitSet( m_bits & other.m_bits );
        }

        BitSet &operator&=( const Enum &other )
        {
            m_bits &= static_cast<uint32_t>( other );
            return *this;
        }

        constexpr BitSet operator~( )
        {
            return BitSet( ~m_bits );
        }

        BitSet &operator=( const Enum &other )
        {
            m_bits = static_cast<uint32_t>( other );
            return *this;
        }

        constexpr bool operator==( const Enum &other )
        {
            return m_bits == static_cast<uint32_t>( other );
        }

        constexpr bool operator!=( const Enum &other )
        {
            return m_bits != static_cast<uint32_t>( other );
        }

        // Interop
        bool None( ) const
        {
            return m_bits == 0;
        }

        // Interop
        void Set( const Enum &other )
        {
            m_bits |= static_cast<uint32_t>( other );
        }

        void Unset( const Enum &other )
        {
            m_bits &= ~static_cast<uint32_t>( other );
        }

        bool IsSet( const Enum &other ) const
        {
            return ( m_bits & static_cast<uint32_t>( other ) ) == static_cast<uint32_t>( other );
        }
        // Interop

        template <typename T>
        bool All( std::initializer_list<T> others ) const
        {
            bool result = true;

            for ( auto &other : others )
            {
                result &= ( m_bits & static_cast<uint32_t>( other ) ) == static_cast<uint32_t>( other );
                if ( !result )
                {
                    return false;
                }
            }
            return true;
        }

        template <typename T>
        bool Any( std::initializer_list<T> others ) const
        {
            bool result = false;
            for ( auto &other : others )
            {
                result |= ( m_bits & static_cast<uint32_t>( other ) ) == static_cast<uint32_t>( other );
                if ( result )
                {
                    return true;
                }
            }
            return false;
        }

        [[nodiscard]] uint32_t Value( ) const
        {
            return m_bits;
        }
    };
} // namespace DenOfIz

