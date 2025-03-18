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

#include <DenOfIzGraphics/Assets/Stream/BinaryReader.h>
#include <cstring>
#include <fstream>

using namespace DenOfIz;

BinaryReader::BinaryReader( std::istream *stream ) : m_stream( stream )
{
    m_isStreamOwned = false;
    m_isStreamValid = true;
}

BinaryReader::BinaryReader( BinaryContainer &container )
{
    m_stream        = &container.m_stream;
    m_isStreamOwned = false;
    m_isStreamValid = true;
}

BinaryReader::BinaryReader( const InteropString &filePath )
{
    m_isStreamOwned = true;
    auto *stream    = new std::ifstream;

    stream->open( filePath.Get( ), std::ios::binary );
    if ( !stream->is_open( ) )
    {
        LOG( ERROR ) << "Failed to open file for reading: " << filePath.Get( );
        m_isStreamValid = false;
        return;
    }

    m_stream        = stream;
    m_isStreamValid = true;
}

BinaryReader::~BinaryReader( )
{
    if ( m_isStreamValid && m_isStreamOwned && dynamic_cast<std::ifstream *>( m_stream )->is_open( ) )
    {
        dynamic_cast<std::ifstream *>( m_stream )->close( );
        delete m_stream;
        m_stream = nullptr;
    }
}

int BinaryReader::ReadByte( ) const
{
    if ( !m_isStreamValid || m_stream->eof( ) )
    {
        return -1;
    }
    return m_stream->get( );
}

int BinaryReader::Read( InteropArray<Byte> &buffer, const uint32_t offset, const uint32_t count ) const
{
    if ( !m_isStreamValid || m_stream->eof( ) )
    {
        return -1;
    }

    std::vector<Byte> tempBuffer( count );
    m_stream->read( reinterpret_cast<char *>( tempBuffer.data( ) ), count );

    const int bytesRead = static_cast<int>( m_stream->gcount( ) );

    for ( int i = 0; i < bytesRead; i++ )
    {
        buffer.SetElement( offset + i, tempBuffer[ i ] );
    }

    return bytesRead;
}

InteropArray<Byte> BinaryReader::ReadBytes( const uint32_t count ) const
{
    InteropArray<Byte> result( count );
    if ( const int bytesRead = Read( result, 0, count ); bytesRead >= 0 && static_cast<uint32_t>( bytesRead ) < count )
    {
        InteropArray<Byte> resized( bytesRead );
        for ( int i = 0; i < bytesRead; i++ )
        {
            resized.SetElement( i, result.GetElement( i ) );
        }
        return resized;
    }

    return result;
}

uint32_t BinaryReader::ReadUInt32( ) const
{
    if ( !m_isStreamValid || m_stream->eof( ) )
    {
        LOG( ERROR ) << "Attempted to read beyond end of file";
        return 0;
    }

    uint32_t value = 0;
    Byte     bytes[ 4 ];
    m_stream->read( reinterpret_cast<char *>( bytes ), 4 );

    if ( m_stream->gcount( ) != 4 )
    {
        LOG( ERROR ) << "Failed to read 4 bytes for uint32";
        return 0;
    }

    value |= static_cast<uint32_t>( bytes[ 0 ] );
    value |= static_cast<uint32_t>( bytes[ 1 ] ) << 8;
    value |= static_cast<uint32_t>( bytes[ 2 ] ) << 16;
    value |= static_cast<uint32_t>( bytes[ 3 ] ) << 24;

    return value;
}

int32_t BinaryReader::ReadInt32( ) const
{
    return static_cast<int32_t>( ReadUInt32( ) );
}

float BinaryReader::ReadFloat( ) const
{
    const uint32_t intValue = ReadUInt32( );
    float          result;
    std::memcpy( &result, &intValue, sizeof( float ) );
    return result;
}

InteropString BinaryReader::ReadString( ) const
{
    const uint32_t length = ReadUInt32( );
    if ( !m_isStreamValid || m_stream->eof( ) )
    {
        LOG( ERROR ) << "Attempted to read string beyond end of file";
        return { };
    }

    std::vector<char> buffer( length + 1 /* + \0*/, 0 );
    m_stream->read( buffer.data( ), length );

    if ( static_cast<uint32_t>( m_stream->gcount( ) ) != length )
    {
        LOG( ERROR ) << "Failed to read full string, expected " << length << " bytes";
    }

    return { buffer.data( ) };
}

uint64_t BinaryReader::Position( ) const
{
    if ( !m_isStreamValid )
    {
        return 0;
    }
    return m_stream->tellg( );
}

void BinaryReader::Seek( const uint64_t position ) const
{
    if ( !m_isStreamValid )
    {
        return;
    }
    m_stream->seekg( position );
}

void BinaryReader::Skip( const uint64_t count ) const
{
    if ( !m_isStreamValid )
    {
        return;
    }
    const uint64_t newPosition = Position( ) + count;
    Seek( newPosition );
}
