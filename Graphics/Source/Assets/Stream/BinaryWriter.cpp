
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

#include <DenOfIzGraphics/Assets/Stream/BinaryWriter.h>
#include <cstring>
#include <fstream>

using namespace DenOfIz;

BinaryWriter::BinaryWriter( std::ostream *stream, const BinaryWriterDesc &desc ) : m_desc( desc ), m_stream( stream )
{
    m_isStreamOwned = false;
    m_isStreamValid = true;
}

BinaryWriter::BinaryWriter( BinaryContainer &container, const BinaryWriterDesc &desc ) : m_desc( desc )
{
    m_stream        = &container.m_stream;
    m_isStreamOwned = false;
    m_isStreamValid = true;
}

BinaryWriter::BinaryWriter( const InteropString &filePath, const BinaryWriterDesc &desc ) : m_desc( desc )
{
    m_isStreamOwned = true;
    auto *stream    = new std::ofstream;

    stream->open( filePath.Get( ), std::ios::binary );
    m_isStreamValid = true;
    if ( !stream->is_open( ) )
    {
        LOG( ERROR ) << "Failed to open file for writing: " << filePath.Get( );
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

void BinaryWriter::Write( const InteropArray<Byte> &buffer, const uint32_t offset, const uint32_t count ) const
{
    if ( !m_isStreamValid )
    {
        return;
    }

    std::vector<Byte> tempBuffer( count );
    for ( uint32_t i = 0; i < count; i++ )
    {
        tempBuffer[ i ] = buffer.GetElement( offset + i );
    }

    m_stream->write( reinterpret_cast<const char *>( tempBuffer.data( ) ), count );
}

void BinaryWriter::WriteBytes( const InteropArray<Byte> &buffer ) const
{
    Write( buffer, 0, buffer.NumElements( ) );
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

void BinaryWriter::WriteString( const InteropString &value ) const
{
    const char *str    = value.Get( );
    const auto  length = static_cast<uint32_t>( strlen( str ) );

    WriteUInt32( length );
    if ( !m_isStreamValid )
    {
        return;
    }

    m_stream->write( str, length );
}

void BinaryWriter::WriteUInt16_2( const UInt16_2 &value ) const
{
    WriteUInt16( value.X );
    WriteUInt16( value.Y );
}

void BinaryWriter::WriteUInt16_3( const UInt16_3 &value ) const
{
    WriteUInt16( value.X );
    WriteUInt16( value.Y );
    WriteUInt16( value.Z );
}

void BinaryWriter::WriteUInt16_4( const UInt16_4 &value ) const
{
    WriteUInt16( value.X );
    WriteUInt16( value.Y );
    WriteUInt16( value.Z );
    WriteUInt16( value.W );
}

void BinaryWriter::WriteInt16_2( const Int16_2 &value ) const
{
    WriteInt16( value.X );
    WriteInt16( value.Y );
}

void BinaryWriter::WriteInt16_3( const Int16_3 &value ) const
{
    WriteInt16( value.X );
    WriteInt16( value.Y );
    WriteInt16( value.Z );
}

void BinaryWriter::WriteInt16_4( const Int16_4 &value ) const
{
    WriteInt16( value.X );
    WriteInt16( value.Y );
    WriteInt16( value.Z );
    WriteInt16( value.W );
}

void BinaryWriter::WriteUInt32_2( const UInt32_2 &value ) const
{
    WriteUInt32( value.X );
    WriteUInt32( value.Y );
}

void BinaryWriter::WriteUInt32_3( const UInt32_3 &value ) const
{
    WriteUInt32( value.X );
    WriteUInt32( value.Y );
    WriteUInt32( value.Z );
}

void BinaryWriter::WriteUInt32_4( const UInt32_4 &value ) const
{
    WriteUInt32( value.X );
    WriteUInt32( value.Y );
    WriteUInt32( value.Z );
    WriteUInt32( value.W );
}

void BinaryWriter::WriteInt32_2( const Int32_2 &value ) const
{
    WriteInt32( value.X );
    WriteInt32( value.Y );
}

void BinaryWriter::WriteInt32_3( const Int32_3 &value ) const
{
    WriteInt32( value.X );
    WriteInt32( value.Y );
    WriteInt32( value.Z );
}

void BinaryWriter::WriteInt32_4( const Int32_4 &value ) const
{
    WriteInt32( value.X );
    WriteInt32( value.Y );
    WriteInt32( value.Z );
    WriteInt32( value.W );
}

void BinaryWriter::WriteFloat_2( const Float_2 &value ) const
{
    WriteFloat( value.X );
    WriteFloat( value.Y );
}

void BinaryWriter::WriteFloat_3( const Float_3 &value ) const
{
    WriteFloat( value.X );
    WriteFloat( value.Y );
    WriteFloat( value.Z );
}

void BinaryWriter::WriteFloat_4( const Float_4 &value ) const
{
    WriteFloat( value.X );
    WriteFloat( value.Y );
    WriteFloat( value.Z );
    WriteFloat( value.W );
}

void BinaryWriter::WriteFloat_4x4( const Float_4x4 &value ) const
{
    for ( const float i : value.M )
    {
        WriteFloat( i );
    }
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
