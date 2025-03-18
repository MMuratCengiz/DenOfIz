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

#include "BinaryContainer.h"

#include <DenOfIzGraphics/Utilities/Interop.h>
#include <ostream>

namespace DenOfIz
{
    class BinaryWriter
    {
        bool          m_isStreamOwned = false;
        bool          m_isStreamValid;
        std::ostream *m_stream;

    public:
        // Non public api since std::ostream cannot be mapped
        explicit BinaryWriter( std::ostream *stream );
        DZ_API explicit BinaryWriter( BinaryContainer &container );
        DZ_API explicit BinaryWriter( const InteropString &filePath );
        DZ_API ~BinaryWriter( );

        DZ_API void                   WriteByte( Byte value ) const;
        DZ_API void                   Write( const InteropArray<Byte> &buffer, uint32_t offset, uint32_t count ) const;
        DZ_API void                   WriteBytes( const InteropArray<Byte> &buffer ) const;
        DZ_API void                   WriteUInt32( uint32_t value ) const;
        DZ_API void                   WriteInt32( int32_t value ) const;
        DZ_API void                   WriteFloat( float value ) const;
        DZ_API void                   WriteString( const InteropString &value ) const;
        DZ_API [[nodiscard]] uint64_t Position( ) const;
        DZ_API void                   Seek( uint64_t position ) const;
        DZ_API void                   Flush( ) const;
    };
} // namespace DenOfIz
