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

#include "DenOfIzGraphics/Utilities/Interop.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"
#include <istream>
#include "BinaryContainer.h"

namespace DenOfIz
{
    struct DZ_API BinaryReaderDesc
    {
        uint64_t NumBytes = 0; // 0, reading more than this amount of bytes won't be allowed
    };

    class BinaryReader
    {
        uint64_t      m_allowedNumBytes;
        uint64_t      m_readNumBytes  = 0;
        bool          m_isStreamOwned = false;
        bool          m_isStreamValid;
        bool          m_isStringStream = false;
        std::istream *m_stream;

    public:
        explicit BinaryReader( std::istream *stream, const BinaryReaderDesc &desc = { } );
        DZ_API explicit BinaryReader( BinaryContainer &container, const BinaryReaderDesc &desc = { } );
        DZ_API explicit BinaryReader( const InteropString &filePath, const BinaryReaderDesc &desc = { } );
        DZ_API explicit BinaryReader( const InteropArray<Byte> &data, const BinaryReaderDesc &desc = { } );
        DZ_API ~BinaryReader( );

        [[nodiscard]] DZ_API int ReadByte( );
        [[nodiscard]] DZ_API int Read( InteropArray<Byte> &buffer, uint32_t offset, uint32_t count );
        [[nodiscard]] DZ_API InteropArray<Byte> ReadAllBytes( );
        [[nodiscard]] DZ_API InteropArray<Byte> ReadBytes( uint32_t count );
        [[nodiscard]] DZ_API uint16_t           ReadUInt16( );
        [[nodiscard]] DZ_API uint32_t           ReadUInt32( );
        [[nodiscard]] DZ_API uint64_t           ReadUInt64( );
        [[nodiscard]] DZ_API int16_t            ReadInt16( );
        [[nodiscard]] DZ_API int32_t            ReadInt32( );
        [[nodiscard]] DZ_API int64_t            ReadInt64( );
        [[nodiscard]] DZ_API float              ReadFloat( );
        [[nodiscard]] DZ_API double             ReadDouble( );
        [[nodiscard]] DZ_API InteropString      ReadString( );
        [[nodiscard]] DZ_API UInt16_2           ReadUInt16_2( );
        [[nodiscard]] DZ_API UInt16_3           ReadUInt16_3( );
        [[nodiscard]] DZ_API UInt16_4           ReadUInt16_4( );
        [[nodiscard]] DZ_API Int16_2            ReadInt16_2( );
        [[nodiscard]] DZ_API Int16_3            ReadInt16_3( );
        [[nodiscard]] DZ_API Int16_4            ReadInt16_4( );
        [[nodiscard]] DZ_API UInt32_2           ReadUInt32_2( );
        [[nodiscard]] DZ_API UInt32_3           ReadUInt32_3( );
        [[nodiscard]] DZ_API UInt32_4           ReadUInt32_4( );
        [[nodiscard]] DZ_API Int32_2            ReadInt32_2( );
        [[nodiscard]] DZ_API Int32_3            ReadInt32_3( );
        [[nodiscard]] DZ_API Int32_4            ReadInt32_4( );
        [[nodiscard]] DZ_API Float_2            ReadFloat_2( );
        [[nodiscard]] DZ_API Float_3            ReadFloat_3( );
        [[nodiscard]] DZ_API Float_4            ReadFloat_4( );
        [[nodiscard]] DZ_API Float_4x4          ReadFloat_4x4( );
        [[nodiscard]] DZ_API uint64_t           Position( ) const;
        DZ_API void                             Seek( uint64_t position ) const;
        DZ_API void                             Skip( uint64_t count ) const;

        // Utility function to log data as a C++ array for embedding in code
        DZ_API void LogAsCppArray( const InteropString &variableName = "Data" ) const;
        DZ_API void WriteCppArrayToFile( const InteropString &targetFile = "Data.txt" ) const;

    private:
        [[nodiscard]] bool IsStreamValid( ) const;
        bool               TrackReadBytes( uint32_t requested );
    };
} // namespace DenOfIz
