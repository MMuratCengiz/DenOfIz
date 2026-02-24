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

#include <memory>
#include <string>
#include <vector>
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

namespace DenOfIz
{
    class StringStore
    {
        std::vector<std::unique_ptr<char[]>> m_blocks;
        char                                *m_currentPtr     = nullptr;
        size_t                               m_bytesRemaining = 0;

    public:
        static constexpr size_t s_defaultBlockSize = 8192;

        StringStore( ) = default;

        StringStore( const StringStore & )            = delete;
        StringStore &operator=( const StringStore & ) = delete;

        StringStore( StringStore && )            = default;
        StringStore &operator=( StringStore && ) = default;

        DenOfIz_StringView Store( const std::string &str );
        DenOfIz_StringView Store( const char *cString );
        DenOfIz_StringView Store( const char *str, size_t len );

    private:
        void AllocateNewBlock( size_t minSize );
    };
} // namespace DenOfIz
