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

#include "stdexcept"

namespace DenOfIz
{
    class CheckedCast
    {
    private:
        template <typename T>
        static T* Validate(T *ptr)
        {
            if (ptr == nullptr)
            {
                throw std::runtime_error("Invalid cast");
            }
            return ptr;
        }
    public:
        template <typename T, typename U>
        static T *Dynamic(U *ptr)
        {
            if (ptr == nullptr)
            {
                return nullptr;
            }
            return Validate(dynamic_cast<T *>(ptr));
        }

        template <typename T, typename U>
        static T *Static(U *ptr)
        {
            if (ptr == nullptr)
            {
                return nullptr;
            }
            return Validate(static_cast<T *>(ptr));
        }

        template <typename T, typename U>
        static T *Reinterpret(U *ptr)
        {
            if (ptr == nullptr)
            {
                return nullptr;
            }
            return Validate(reinterpret_cast<T *>(ptr));
        }

        template <typename T, typename U>
        static T *Const(U *ptr)
        {
            if (ptr == nullptr)
            {
                return nullptr;
            }
            return Validate(const_cast<T *>(ptr));
        }
    };
}