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
#include <istream>
#include "BinaryContainer.h"

namespace DenOfIz
{
    struct DZ_API BinaryReaderDesc
    {
        uint64_t Offset     = 0;
        uint64_t NumBytes   = 0; // 0 means All
        bool     ForceRange = true;
    };

    class DZ_API BinaryReader
    {
        uint64_t      m_offset;
        uint64_t      m_allowedNumBytes;
        uint64_t      m_readNumBytes = 0;
        bool          m_forceRange;
        bool          m_isStreamOwned = false;
        bool          m_isStreamValid;
        std::istream *m_stream;

    public:
        DZ_API explicit BinaryReader( std::istream *stream, const BinaryReaderDesc &desc = { } );
        DZ_API explicit BinaryReader( BinaryContainer &container, const BinaryReaderDesc &desc = { } );
        DZ_API explicit BinaryReader( const InteropString &filePath, const BinaryReaderDesc &desc = { } );
        DZ_API ~BinaryReader( );

        [[nodiscard]] int                ReadByte( );
        int                              Read( InteropArray<Byte> &buffer, uint32_t offset, uint32_t count );
        [[nodiscard]] InteropArray<Byte> ReadBytes( uint32_t count );
        [[nodiscard]] uint32_t           ReadUInt32( );
        [[nodiscard]] int32_t            ReadInt32( );
        [[nodiscard]] float              ReadFloat( );
        [[nodiscard]] InteropString      ReadString( );
        [[nodiscard]] uint64_t           Position( ) const;
        void                             Seek( uint64_t position ) const;
        void                             Skip( uint64_t count ) const;

    private:
        bool IsStreamValid( ) const;
        bool TrackReadBytes( uint32_t requested );
        void InitStream( ) const;
    };
} // namespace DenOfIz
