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

#include <DenOfIzGraphics/Utilities/Interop.h>
#include "BinaryContainer.h"
#include <istream>

namespace DenOfIz
{
    class DZ_API BinaryReader
    {
        bool          m_isStreamOwned = false;
        bool          m_isStreamValid;
        std::istream *m_stream;

    public:
        DZ_API explicit BinaryReader( std::istream *stream );
        DZ_API explicit BinaryReader( BinaryContainer &container );
        DZ_API explicit BinaryReader( const InteropString &filePath );
        DZ_API ~BinaryReader( );

        [[nodiscard]] int                ReadByte( ) const;
        int                              Read( InteropArray<Byte> &buffer, uint32_t offset, uint32_t count ) const;
        [[nodiscard]] InteropArray<Byte> ReadBytes( uint32_t count ) const;
        [[nodiscard]] uint32_t           ReadUInt32( ) const;
        [[nodiscard]] int32_t            ReadInt32( ) const;
        [[nodiscard]] float              ReadFloat( ) const;
        [[nodiscard]] InteropString      ReadString( ) const;
        [[nodiscard]] uint64_t           Position( ) const;
        void                             Seek( uint64_t position ) const;
        void                             Skip( uint64_t count ) const;
    };
} // namespace DenOfIz
