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

#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"
#include <cstring>
#include <fstream>
#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphicsInternal/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphicsInternal/Assets/Stream/BinaryContainerImpl.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

namespace DenOfIz
{
    class BinaryWriter
    {
        bool          m_isStreamOwned = false;
        bool          m_isStreamValid;
        std::ostream *m_stream;

    public:
        explicit BinaryWriter( std::ostream *stream );
        explicit BinaryWriter( const DenOfIz_StringView &filePath );
        ~BinaryWriter( );

        void                   WriteByte( Byte value ) const;
        void                   Write( const DenOfIz_ByteArrayView &buffer, uint32_t offset, uint32_t count ) const;
        void                   WriteBytes( const DenOfIz_ByteArray &buffer ) const;
        void                   WriteBytes( const DenOfIz_ByteArrayView &buffer ) const;
        void                   WriteUInt16( uint16_t value ) const;
        void                   WriteUInt32( uint32_t value ) const;
        void                   WriteUInt64( uint64_t value ) const;
        void                   WriteInt16( int16_t value ) const;
        void                   WriteInt32( int32_t value ) const;
        void                   WriteInt64( int64_t value ) const;
        void                   WriteFloat( float value ) const;
        void                   WriteDouble( double value ) const;
        void                   WriteString( const DenOfIz_StringView &value ) const;
        void                   WriteUInt16_2( const DenOfIz_UShort2 &value ) const;
        void                   WriteUInt16_3( const DenOfIz_UShort3 &value ) const;
        void                   WriteUInt16_4( const DenOfIz_UShort4 &value ) const;
        void                   WriteInt16_2( const DenOfIz_Short2 &value ) const;
        void                   WriteInt16_3( const DenOfIz_Short3 &value ) const;
        void                   WriteInt16_4( const DenOfIz_Short4 &value ) const;
        void                   WriteUInt32_2( const DenOfIz_UInt2 &value ) const;
        void                   WriteUInt32_3( const DenOfIz_UInt3 &value ) const;
        void                   WriteUInt32_4( const DenOfIz_UInt4 &value ) const;
        void                   WriteInt32_2( const DenOfIz_Int2 &value ) const;
        void                   WriteInt32_3( const DenOfIz_Int3 &value ) const;
        void                   WriteInt32_4( const DenOfIz_Int4 &value ) const;
        void                   WriteFloat_2( const DenOfIz_Float2 &value ) const;
        void                   WriteFloat_3( const DenOfIz_Float3 &value ) const;
        void                   WriteFloat_4( const DenOfIz_Float4 &value ) const;
        void                   WriteFloat_4x4( const DenOfIz_Float4x4 &value ) const;
        [[nodiscard]] uint64_t Position( ) const;
        void                   Seek( uint64_t position ) const;
        void                   Flush( ) const;
    };
} // namespace DenOfIz

using namespace DenOfIz;

BinaryWriter::BinaryWriter( std::ostream *stream ) : m_stream( stream )
{
    m_isStreamOwned = false;
    m_isStreamValid = true;
}

BinaryWriter::BinaryWriter( const DenOfIz_StringView &filePath )
{
    m_isStreamOwned = true;
    auto *stream    = new std::ofstream;

    const std::string resolvedPath = FileIO::GetResourcePath( filePath );
    stream->open( resolvedPath, std::ios::binary );
    m_isStreamValid = true;
    if ( !stream->is_open( ) )
    {
        std::string filePathStr( filePath.Chars, filePath.NumChars );
        spdlog::error( "Failed to open file for writing: {}", filePathStr );
        m_isStreamValid = false;
    }
    m_stream = stream;
}

BinaryWriter::~BinaryWriter( )
{
    if ( m_isStreamOwned && dynamic_cast<std::ofstream *>( m_stream )->is_open( ) )
    {
        dynamic_cast<std::ofstream *>( m_stream )->close( );
        delete m_stream;
        m_stream = nullptr;
    }
}

void BinaryWriter::WriteByte( const Byte value ) const
{
    if ( !m_isStreamValid )
    {
        return;
    }
    m_stream->put( static_cast<char>( value ) );
}

void BinaryWriter::Write( const DenOfIz_ByteArrayView &buffer, const uint32_t offset, const uint32_t count ) const
{
    if ( !m_isStreamValid )
    {
        return;
    }

    m_stream->write( reinterpret_cast<const char *>( buffer.Elements + offset ), count );
}

void BinaryWriter::WriteBytes( const DenOfIz_ByteArray &buffer ) const
{
    WriteBytes( DenOfIz_ByteArrayView{ buffer.Elements, buffer.NumElements } );
}

void BinaryWriter::WriteBytes( const DenOfIz_ByteArrayView &buffer ) const
{
    Write( buffer, 0, buffer.NumElements );
}

void BinaryWriter::WriteUInt16( const uint16_t value ) const
{
    if ( !m_isStreamValid )
    {
        return;
    }

    Byte bytes[ 2 ];
    bytes[ 0 ] = static_cast<Byte>( value & 0xFF );
    bytes[ 1 ] = static_cast<Byte>( value >> 8 & 0xFF );
    m_stream->write( reinterpret_cast<const char *>( bytes ), 2 );
}

void BinaryWriter::WriteUInt32( const uint32_t value ) const
{
    if ( !m_isStreamValid )
    {
        return;
    }

    Byte bytes[ 4 ];
    bytes[ 0 ] = static_cast<Byte>( value & 0xFF );
    bytes[ 1 ] = static_cast<Byte>( value >> 8 & 0xFF );
    bytes[ 2 ] = static_cast<Byte>( value >> 16 & 0xFF );
    bytes[ 3 ] = static_cast<Byte>( value >> 24 & 0xFF );

    m_stream->write( reinterpret_cast<const char *>( bytes ), 4 );
}

void BinaryWriter::WriteUInt64( const uint64_t value ) const
{
    if ( !m_isStreamValid )
    {
        return;
    }

    WriteUInt32( static_cast<uint32_t>( value >> 32 ) );
    WriteUInt32( static_cast<uint32_t>( value & 0xFFFFFFFF ) );
}

void BinaryWriter::WriteInt16( const int16_t value ) const
{
    WriteUInt16( static_cast<uint16_t>( value ) );
}

void BinaryWriter::WriteInt32( const int32_t value ) const
{
    WriteUInt32( static_cast<uint32_t>( value ) );
}

void BinaryWriter::WriteInt64( const int64_t value ) const
{
    WriteUInt64( static_cast<uint32_t>( value ) );
}

void BinaryWriter::WriteFloat( const float value ) const
{
    uint32_t intValue;
    std::memcpy( &intValue, &value, sizeof( float ) );
    WriteUInt32( intValue );
}

void BinaryWriter::WriteDouble( const double value ) const
{
    uint64_t intValue;
    std::memcpy( &intValue, &value, sizeof( double ) );
    WriteUInt64( intValue );
}

void BinaryWriter::WriteString( const DenOfIz_StringView &value ) const
{
    WriteUInt32( value.NumChars );
    if ( !m_isStreamValid )
    {
        return;
    }

    m_stream->write( value.Chars, value.NumChars );
}

void BinaryWriter::WriteUInt16_2( const DenOfIz_UShort2 &value ) const
{
    WriteUInt16( value.X );
    WriteUInt16( value.Y );
}

void BinaryWriter::WriteUInt16_3( const DenOfIz_UShort3 &value ) const
{
    WriteUInt16( value.X );
    WriteUInt16( value.Y );
    WriteUInt16( value.Z );
}

void BinaryWriter::WriteUInt16_4( const DenOfIz_UShort4 &value ) const
{
    WriteUInt16( value.X );
    WriteUInt16( value.Y );
    WriteUInt16( value.Z );
    WriteUInt16( value.W );
}

void BinaryWriter::WriteInt16_2( const DenOfIz_Short2 &value ) const
{
    WriteInt16( value.X );
    WriteInt16( value.Y );
}

void BinaryWriter::WriteInt16_3( const DenOfIz_Short3 &value ) const
{
    WriteInt16( value.X );
    WriteInt16( value.Y );
    WriteInt16( value.Z );
}

void BinaryWriter::WriteInt16_4( const DenOfIz_Short4 &value ) const
{
    WriteInt16( value.X );
    WriteInt16( value.Y );
    WriteInt16( value.Z );
    WriteInt16( value.W );
}

void BinaryWriter::WriteUInt32_2( const DenOfIz_UInt2 &value ) const
{
    WriteUInt32( value.X );
    WriteUInt32( value.Y );
}

void BinaryWriter::WriteUInt32_3( const DenOfIz_UInt3 &value ) const
{
    WriteUInt32( value.X );
    WriteUInt32( value.Y );
    WriteUInt32( value.Z );
}

void BinaryWriter::WriteUInt32_4( const DenOfIz_UInt4 &value ) const
{
    WriteUInt32( value.X );
    WriteUInt32( value.Y );
    WriteUInt32( value.Z );
    WriteUInt32( value.W );
}

void BinaryWriter::WriteInt32_2( const DenOfIz_Int2 &value ) const
{
    WriteInt32( value.X );
    WriteInt32( value.Y );
}

void BinaryWriter::WriteInt32_3( const DenOfIz_Int3 &value ) const
{
    WriteInt32( value.X );
    WriteInt32( value.Y );
    WriteInt32( value.Z );
}

void BinaryWriter::WriteInt32_4( const DenOfIz_Int4 &value ) const
{
    WriteInt32( value.X );
    WriteInt32( value.Y );
    WriteInt32( value.Z );
    WriteInt32( value.W );
}

void BinaryWriter::WriteFloat_2( const DenOfIz_Float2 &value ) const
{
    WriteFloat( value.X );
    WriteFloat( value.Y );
}

void BinaryWriter::WriteFloat_3( const DenOfIz_Float3 &value ) const
{
    WriteFloat( value.X );
    WriteFloat( value.Y );
    WriteFloat( value.Z );
}

void BinaryWriter::WriteFloat_4( const DenOfIz_Float4 &value ) const
{
    WriteFloat( value.X );
    WriteFloat( value.Y );
    WriteFloat( value.Z );
    WriteFloat( value.W );
}

void BinaryWriter::WriteFloat_4x4( const DenOfIz_Float4x4 &value ) const
{
    WriteFloat( value._11 );
    WriteFloat( value._12 );
    WriteFloat( value._13 );
    WriteFloat( value._14 );

    WriteFloat( value._21 );
    WriteFloat( value._22 );
    WriteFloat( value._23 );
    WriteFloat( value._24 );

    WriteFloat( value._31 );
    WriteFloat( value._32 );
    WriteFloat( value._33 );
    WriteFloat( value._34 );

    WriteFloat( value._41 );
    WriteFloat( value._42 );
    WriteFloat( value._43 );
    WriteFloat( value._44 );
}

uint64_t BinaryWriter::Position( ) const
{
    if ( !m_isStreamValid )
    {
        return 0;
    }
    return m_stream->tellp( );
}

void BinaryWriter::Seek( const uint64_t position ) const
{
    if ( !m_isStreamValid )
    {
        return;
    }
    m_stream->seekp( static_cast<long long>( position ) );
}

void BinaryWriter::Flush( ) const
{
    if ( !m_isStreamValid )
    {
        return;
    }
    m_stream->flush( );
}

#define BINARY_WRITER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::BinaryWriter, handle )
#define BINARY_CONTAINER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::BinaryContainerImpl, handle )

extern "C"
{

    DenOfIz_BinaryWriter DenOfIz_BinaryWriter_CreateFromContainer( DenOfIz_BinaryContainer container )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( container ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        auto *writer = new BinaryWriter( BINARY_CONTAINER_IMPL( container )->Stream( ) );
        return DENOFIZ_TO_HANDLE( writer );
    }

    DenOfIz_BinaryWriter DenOfIz_BinaryWriter_CreateFromFile( DenOfIz_StringView filePath )
    {
        auto *writer = new DenOfIz::BinaryWriter( filePath );
        return DENOFIZ_TO_HANDLE( writer );
    }

    void DenOfIz_BinaryWriter_Destroy( DenOfIz_BinaryWriter writer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        delete BINARY_WRITER_IMPL( writer );
    }

    void DenOfIz_BinaryWriter_WriteByte( DenOfIz_BinaryWriter writer, Byte value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteByte( value );
    }

    void DenOfIz_BinaryWriter_Write( DenOfIz_BinaryWriter writer, DenOfIz_ByteArrayView buffer, uint32_t offset, uint32_t count )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->Write( buffer, offset, count );
    }

    void DenOfIz_BinaryWriter_WriteBytes( DenOfIz_BinaryWriter writer, DenOfIz_ByteArrayView buffer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteBytes( buffer );
    }

    void DenOfIz_BinaryWriter_WriteUInt16( DenOfIz_BinaryWriter writer, uint16_t value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteUInt16( value );
    }

    void DenOfIz_BinaryWriter_WriteUInt32( DenOfIz_BinaryWriter writer, uint32_t value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteUInt32( value );
    }

    void DenOfIz_BinaryWriter_WriteUInt64( DenOfIz_BinaryWriter writer, uint64_t value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteUInt64( value );
    }

    void DenOfIz_BinaryWriter_WriteInt16( DenOfIz_BinaryWriter writer, int16_t value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteInt16( value );
    }

    void DenOfIz_BinaryWriter_WriteInt32( DenOfIz_BinaryWriter writer, int32_t value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteInt32( value );
    }

    void DenOfIz_BinaryWriter_WriteInt64( DenOfIz_BinaryWriter writer, int64_t value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteInt64( value );
    }

    void DenOfIz_BinaryWriter_WriteFloat( DenOfIz_BinaryWriter writer, float value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteFloat( value );
    }

    void DenOfIz_BinaryWriter_WriteDouble( DenOfIz_BinaryWriter writer, double value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteDouble( value );
    }

    void DenOfIz_BinaryWriter_WriteString( DenOfIz_BinaryWriter writer, DenOfIz_StringView value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteString( value );
    }

    void DenOfIz_BinaryWriter_WriteUInt16_2( DenOfIz_BinaryWriter writer, DenOfIz_UShort2 value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteUInt16_2( value );
    }

    void DenOfIz_BinaryWriter_WriteUInt16_3( DenOfIz_BinaryWriter writer, DenOfIz_UShort3 value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteUInt16_3( value );
    }

    void DenOfIz_BinaryWriter_WriteUInt16_4( DenOfIz_BinaryWriter writer, DenOfIz_UShort4 value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteUInt16_4( value );
    }

    void DenOfIz_BinaryWriter_WriteInt16_2( DenOfIz_BinaryWriter writer, DenOfIz_Short2 value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteInt16_2( value );
    }

    void DenOfIz_BinaryWriter_WriteInt16_3( DenOfIz_BinaryWriter writer, DenOfIz_Short3 value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteInt16_3( value );
    }

    void DenOfIz_BinaryWriter_WriteInt16_4( DenOfIz_BinaryWriter writer, DenOfIz_Short4 value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteInt16_4( value );
    }

    void DenOfIz_BinaryWriter_WriteUInt32_2( DenOfIz_BinaryWriter writer, DenOfIz_UInt2 value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteUInt32_2( value );
    }

    void DenOfIz_BinaryWriter_WriteUInt32_3( DenOfIz_BinaryWriter writer, DenOfIz_UInt3 value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteUInt32_3( value );
    }

    void DenOfIz_BinaryWriter_WriteUInt32_4( DenOfIz_BinaryWriter writer, DenOfIz_UInt4 value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteUInt32_4( value );
    }

    void DenOfIz_BinaryWriter_WriteInt32_2( DenOfIz_BinaryWriter writer, DenOfIz_Int2 value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteInt32_2( value );
    }

    void DenOfIz_BinaryWriter_WriteInt32_3( DenOfIz_BinaryWriter writer, DenOfIz_Int3 value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteInt32_3( value );
    }

    void DenOfIz_BinaryWriter_WriteInt32_4( DenOfIz_BinaryWriter writer, DenOfIz_Int4 value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteInt32_4( value );
    }

    void DenOfIz_BinaryWriter_WriteFloat_2( DenOfIz_BinaryWriter writer, DenOfIz_Float2 value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteFloat_2( value );
    }

    void DenOfIz_BinaryWriter_WriteFloat_3( DenOfIz_BinaryWriter writer, DenOfIz_Float3 value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteFloat_3( value );
    }

    void DenOfIz_BinaryWriter_WriteFloat_4( DenOfIz_BinaryWriter writer, DenOfIz_Float4 value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteFloat_4( value );
    }

    void DenOfIz_BinaryWriter_WriteFloat_4x4( DenOfIz_BinaryWriter writer, DenOfIz_Float4x4 value )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->WriteFloat_4x4( value );
    }

    uint64_t DenOfIz_BinaryWriter_Position( DenOfIz_BinaryWriter writer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return 0;
        }
        return BINARY_WRITER_IMPL( writer )->Position( );
    }

    void DenOfIz_BinaryWriter_Seek( DenOfIz_BinaryWriter writer, uint64_t position )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->Seek( position );
    }

    void DenOfIz_BinaryWriter_Flush( DenOfIz_BinaryWriter writer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( writer ) )
        {
            return;
        }
        BINARY_WRITER_IMPL( writer )->Flush( );
    }
}
