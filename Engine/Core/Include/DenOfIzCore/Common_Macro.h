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

#define DZ_RETURN_IF(condition)                                                                                                                                                    \
    if ( condition )                                                                                                                                                               \
    return
#define DZ_ASSERTM(exp, msg) assert(((void)msg, exp))
#define DZ_NOT_NULL(exp) DZ_ASSERTM(exp != nullptr, #exp " is null")

namespace DenOfIz
{
    class NonCopyable
    {
        NonCopyable(NonCopyable const &)            = delete;
        NonCopyable(NonCopyable &&)                 = delete;
        NonCopyable &operator=(NonCopyable const &) = delete;
        NonCopyable &operator=(NonCopyable &&)      = delete;

    protected:
        NonCopyable()  = default;
        ~NonCopyable() = default;
    };
} // namespace DenOfIz

#define DZ_BITSET(Enum)                                                                                                                                                            \
    inline constexpr Enum operator|(Enum a, Enum b)                                                                                                                                \
    {                                                                                                                                                                              \
        return Enum(((uint32_t)a) | ((uint32_t)b));                                                                                                                                \
    }                                                                                                                                                                              \
    inline Enum &operator|=(Enum &a, Enum b)                                                                                                                                       \
    {                                                                                                                                                                              \
        return (Enum &)(((uint32_t &)a) |= ((uint32_t)b));                                                                                                                         \
    }                                                                                                                                                                              \
    inline constexpr Enum operator&(Enum a, Enum b)                                                                                                                                \
    {                                                                                                                                                                              \
        return Enum(((uint32_t)a) & ((uint32_t)b));                                                                                                                                \
    }                                                                                                                                                                              \
    inline Enum &operator&=(Enum &a, Enum b)                                                                                                                                       \
    {                                                                                                                                                                              \
        return (Enum &)(((uint32_t &)a) &= ((uint32_t)b));                                                                                                                         \
    }                                                                                                                                                                              \
    inline constexpr Enum operator~(Enum a)                                                                                                                                        \
    {                                                                                                                                                                              \
        return Enum(~((uint32_t)a));                                                                                                                                               \
    }                                                                                                                                                                              \
    inline constexpr Enum operator^(Enum a, Enum b)                                                                                                                                \
    {                                                                                                                                                                              \
        return Enum(((uint32_t)a) ^ ((uint32_t)b));                                                                                                                                \
    }                                                                                                                                                                              \
    inline Enum &operator^=(Enum &a, Enum b)                                                                                                                                       \
    {                                                                                                                                                                              \
        return (Enum &)(((uint32_t &)a) ^= ((uint32_t)b));                                                                                                                         \
    }
