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

#include <iosfwd>
#include <ostream>
#include <sstream>

#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

namespace DenOfIz
{
    class BinaryContainerImpl
    {
        mutable std::string m_cachedData;
        std::stringstream   m_stream;

    public:
        BinaryContainerImpl( );
        ~BinaryContainerImpl( );

        std::iostream        *Stream( );
        DenOfIz_ByteArrayView GetData( ) const;
    };
} // namespace DenOfIz
