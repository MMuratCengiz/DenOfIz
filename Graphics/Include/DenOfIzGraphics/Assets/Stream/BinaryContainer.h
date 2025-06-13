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

#include <sstream>
#include "DenOfIzGraphics/Utilities/Common.h"

#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

namespace DenOfIz
{
    class BinaryContainer
    {
        std::stringstream m_stream;
        mutable std::string m_cachedData; // Cache the string data to avoid dangling pointer

        friend class BinaryWriter;
        friend class BinaryReader;

    public:
        DZ_API explicit BinaryContainer( );
        DZ_API ~BinaryContainer( );
        DZ_API [[nodiscard]] ByteArrayView GetData( ) const;
    };
} // namespace DenOfIz
