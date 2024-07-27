/*
Blazar Engine - 3D Game Engine
Copyright (c) 2020-2021 Muhammed Murat Cengiz

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

#include "Common.h"

namespace DenOfIz
{

    class Utilities
    {
        Utilities() = default;

    public:
        static std::string ReadFile(const std::string &filename);

        static std::string GetFileDirectory(const std::string &file, bool includeFinalSep = true);

        static std::string GetFilename(const std::string &file);

        static std::string CombineDirectories(const std::string &directory, const std::string &file);

        static std::string AppPath(const std::string&resourcePath);

        inline static uint32_t Align(uint32_t value, uint32_t alignment)
        {
            return (value + alignment - 1) & ~(alignment - 1);
        }

        template<typename T>
        static T& SafeAt(std::vector<T> &vec, size_t index)
        {
            if (index >= vec.size())
            {
                vec.resize(index + 1);
            }
            return vec[index];
        }
    };

} // namespace DenOfIz
