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
    private:
        uint32_t m_bits;

        explicit BitSet(uint32_t bits) : m_bits(bits)
        {
        }

    public:
        inline BitSet() : m_bits(0)
        {
        }

        inline BitSet(const Enum &en) : m_bits((uint32_t)en)
        {
        }

        inline constexpr operator Enum() const
        {
            return (Enum)m_bits;
        }

        inline constexpr BitSet<Enum> operator|(const Enum &other)
        {
            return BitSet<Enum>(m_bits | (uint32_t)other);
        }

        inline constexpr BitSet<Enum> operator|(const BitSet<Enum> &other)
        {
            return BitSet<Enum>(m_bits | other.m_bits);
        }

        inline BitSet<Enum> &operator|=(const Enum &other)
        {
            m_bits |= (uint32_t)other;
            return *this;
        }

        inline constexpr BitSet<Enum> operator&(const Enum &other)
        {
            return BitSet<Enum>(m_bits & (uint32_t)other);
        }

        inline constexpr BitSet<Enum> operator&(const BitSet<Enum> &other)
        {
            return BitSet<Enum>(m_bits & other.m_bits);
        }

        inline BitSet<Enum> &operator&=(const Enum &other)
        {
            m_bits &= (uint32_t)other;
            return *this;
        }

        inline constexpr BitSet<Enum> operator~()
        {
            return BitSet<Enum>(~m_bits);
        }

        inline BitSet<Enum> &operator=(const Enum &other)
        {
            m_bits = (uint32_t)other;
            return *this;
        }

        inline constexpr bool operator==(const Enum &other)
        {
            return m_bits == ((uint32_t)other);
        }

        inline constexpr bool operator!=(const Enum &other)
        {
            return m_bits != ((uint32_t)other);
        }

        inline bool None()
        {
            return m_bits == 0;
        }

        inline bool IsSet(const Enum &other) const
        {
            return (m_bits & ((uint32_t)other)) == ((uint32_t)other);
        }

        template <typename T>
        inline bool All(std::initializer_list<T> others) const
        {
            bool result = true;

            for ( auto &other : others )
            {
                result &= (m_bits & ((uint32_t)other)) == ((uint32_t)other);
                if ( !result )
                {
                    return false;
                }
            }
            return true;
        }

        template <typename T>
        inline bool Any(std::initializer_list<T> others) const
        {
            bool result = false;
            for ( auto &other : others )
            {
                result |= (m_bits & ((uint32_t)other)) == ((uint32_t)other);
                if ( result )
                {
                    return true;
                }
            }
            return false;
        }
    };
} // namespace DenOfIz
