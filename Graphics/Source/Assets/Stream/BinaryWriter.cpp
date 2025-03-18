
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

BinaryWriter::BinaryWriter( std::ostream *stream ) : m_stream( stream )
{
    m_isStreamOwned = false;
    m_isStreamValid = true;
}

BinaryWriter::BinaryWriter( BinaryContainer &container )
{
    m_stream        = &container.m_stream;
    m_isStreamOwned = false;
    m_isStreamValid = true;
}

BinaryWriter::BinaryWriter( const InteropString &filePath )
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

void BinaryWriter::WriteInt32( const int32_t value ) const
{
    WriteUInt32( static_cast<uint32_t>( value ) );
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
