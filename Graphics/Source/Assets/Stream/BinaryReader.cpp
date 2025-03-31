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

BinaryReader::BinaryReader( std::istream *stream, const BinaryReaderDesc &desc ) : m_allowedNumBytes( desc.NumBytes ), m_stream( stream )
{
    m_isStreamOwned = false;
    m_isStreamValid = true;
}

BinaryReader::BinaryReader( BinaryContainer &container, const BinaryReaderDesc &desc ) : m_allowedNumBytes( desc.NumBytes )
{
    m_stream        = &container.m_stream;
    m_isStreamOwned = false;
    m_isStreamValid = true;
}

BinaryReader::BinaryReader( const InteropString &filePath, const BinaryReaderDesc &desc ) : m_allowedNumBytes( desc.NumBytes )
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

int BinaryReader::ReadByte( )
{
    if ( !IsStreamValid( ) || !TrackReadBytes( 1 ) )
    {
        return -1;
    }
    return m_stream->get( );
}

int BinaryReader::Read( InteropArray<Byte> &buffer, const uint32_t offset, const uint32_t count )
{
    if ( !IsStreamValid( ) || !TrackReadBytes( count ) )
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

InteropArray<Byte> BinaryReader::ReadBytes( const uint32_t count )
{
    if ( !IsStreamValid( ) || !TrackReadBytes( count ) )
    {
        return 0.0f;
    }

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

uint16_t BinaryReader::ReadUInt16( )
{
    if ( !IsStreamValid( ) || !TrackReadBytes( 1 ) )
    {
        return 0;
    }
    uint16_t value = 0;
    Byte     bytes[ 2 ];
    m_stream->read( reinterpret_cast<char *>( bytes ), 2 );
    if ( m_stream->gcount( ) != 2 )
    {
        LOG( ERROR ) << "Failed to read 2 bytes for uint16";
        return 0;
    }

    value |= static_cast<uint16_t>( bytes[ 0 ] );
    value |= static_cast<uint16_t>( bytes[ 1 ] ) << 8;

    return value;
}

uint32_t BinaryReader::ReadUInt32( )
{
    if ( !IsStreamValid( ) || !TrackReadBytes( 1 ) )
    {
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

uint64_t BinaryReader::ReadUInt64( )
{
    if ( !IsStreamValid( ) || !TrackReadBytes( 1 ) )
    {
        return 0;
    }

    const uint32_t high = ReadUInt32( );
    const uint32_t low  = ReadUInt32( );

    return static_cast<uint64_t>( high ) << 32 | low;
}

int16_t BinaryReader::ReadInt16( )
{
    return static_cast<int16_t>( ReadUInt16( ) );
}

int32_t BinaryReader::ReadInt32( )
{
    return static_cast<int32_t>( ReadUInt32( ) );
}

int64_t BinaryReader::ReadInt64( )
{
    return static_cast<int64_t>( ReadUInt64( ) );
}

float BinaryReader::ReadFloat( )
{
    if ( !IsStreamValid( ) || !TrackReadBytes( 1 ) )
    {
        return 0.0f;
    }

    const uint32_t intValue = ReadUInt32( );
    float          result;
    std::memcpy( &result, &intValue, sizeof( float ) );
    return result;
}

InteropString BinaryReader::ReadString( )
{
    const uint32_t length = ReadUInt32( );
    if ( !IsStreamValid( ) || !TrackReadBytes( length ) )
    {
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

UInt16_2 BinaryReader::ReadUInt16_2( )
{
    return { ReadUInt16( ), ReadUInt16( ) };
}

UInt16_3 BinaryReader::ReadUInt16_3( )
{
    return { ReadUInt16( ), ReadUInt16( ), ReadUInt16( ) };
}

UInt16_4 BinaryReader::ReadUInt16_4( )
{
    return { ReadUInt16( ), ReadUInt16( ), ReadUInt16( ), ReadUInt16( ) };
}

Int16_2 BinaryReader::ReadInt16_2( )
{
    return { ReadInt16( ), ReadInt16( ) };
}

Int16_3 BinaryReader::ReadInt16_3( )
{
    return { ReadInt16( ), ReadInt16( ), ReadInt16( ) };
}

Int16_4 BinaryReader::ReadInt16_4( )
{
    return { ReadInt16( ), ReadInt16( ), ReadInt16( ), ReadInt16( ) };
}

UInt32_2 BinaryReader::ReadUInt32_2( )
{
    return { ReadUInt32( ), ReadUInt32( ) };
}

UInt32_3 BinaryReader::ReadUInt32_3( )
{
    return { ReadUInt32( ), ReadUInt32( ), ReadUInt32( ) };
}

UInt32_4 BinaryReader::ReadUInt32_4( )
{
    return { ReadUInt32( ), ReadUInt32( ), ReadUInt32( ), ReadUInt32( ) };
}

Int32_2 BinaryReader::ReadInt32_2( )
{
    return { ReadInt32( ), ReadInt32( ) };
}

Int32_3 BinaryReader::ReadInt32_3( )
{
    return { ReadInt32( ), ReadInt32( ), ReadInt32( ) };
}

Int32_4 BinaryReader::ReadInt32_4( )
{
    return { ReadInt32( ), ReadInt32( ), ReadInt32( ), ReadInt32( ) };
}

Float_2 BinaryReader::ReadFloat_2( )
{
    return { ReadFloat( ), ReadFloat( ) };
}

Float_3 BinaryReader::ReadFloat_3( )
{
    return { ReadFloat( ), ReadFloat( ), ReadFloat( ) };
}

Float_4 BinaryReader::ReadFloat_4( )
{
    return { ReadFloat( ), ReadFloat( ), ReadFloat( ), ReadFloat( ) };
}

Float_4x4 BinaryReader::ReadFloat_4x4( )
{
    Float_4x4 result{ };

    result._11 = ReadFloat( );
    result._12 = ReadFloat( );
    result._13 = ReadFloat( );
    result._14 = ReadFloat( );

    result._21 = ReadFloat( );
    result._22 = ReadFloat( );
    result._23 = ReadFloat( );
    result._24 = ReadFloat( );

    result._31 = ReadFloat( );
    result._32 = ReadFloat( );
    result._33 = ReadFloat( );
    result._34 = ReadFloat( );

    result._41 = ReadFloat( );
    result._42 = ReadFloat( );
    result._43 = ReadFloat( );
    result._44 = ReadFloat( );

    return result;
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

bool BinaryReader::IsStreamValid( ) const
{
    if ( !m_isStreamValid || m_stream->eof( ) )
    {
        LOG( ERROR ) << "Attempted to read string beyond end of file";
        return false;
    }

    return true;
}

bool BinaryReader::TrackReadBytes( const uint32_t requested )
{
    // 0 is all
    if ( m_allowedNumBytes > 0 )
    {
        if ( m_readNumBytes + requested <= m_allowedNumBytes )
        {
            m_readNumBytes += requested;
            return true;
        }
        LOG( ERROR ) << "Attempted to read beyond end of the allowed range";
        return false;
    }
    return true;
}
