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

#include "DenOfIzGraphics/Assets/Stream/BinaryReader.h"
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphicsInternal/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphicsInternal/Assets/Stream/BinaryContainerImpl.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

namespace DenOfIz
{
    struct BinaryReaderDesc
    {
        uint64_t NumBytes = 0;
    };

    class BinaryReader
    {
        uint64_t                               m_allowedNumBytes;
        uint64_t                               m_readNumBytes  = 0;
        bool                                   m_isStreamOwned = false;
        bool                                   m_isStreamValid;
        bool                                   m_isStringStream = false;
        std::istream                          *m_stream;
        mutable std::vector<std::vector<char>> m_stringStorage;

    public:
        explicit BinaryReader( std::istream *stream, const BinaryReaderDesc &desc = { } );
        explicit BinaryReader( const DenOfIz_StringView &filePath, const BinaryReaderDesc &desc = { } );
        explicit BinaryReader( const DenOfIz_ByteArrayView &data, const BinaryReaderDesc &desc = { } );
        ~BinaryReader( );

        [[nodiscard]] int                ReadByte( );
        [[nodiscard]] int                Read( const DenOfIz_ByteArray &buffer, uint32_t offset, uint32_t count );
        [[nodiscard]] DenOfIz_ByteArray  ReadAllBytes( );
        [[nodiscard]] DenOfIz_ByteArray  ReadBytes( uint32_t count );
        [[nodiscard]] uint16_t           ReadUInt16( );
        [[nodiscard]] uint32_t           ReadUInt32( );
        [[nodiscard]] uint64_t           ReadUInt64( );
        [[nodiscard]] int16_t            ReadInt16( );
        [[nodiscard]] int32_t            ReadInt32( );
        [[nodiscard]] int64_t            ReadInt64( );
        [[nodiscard]] float              ReadFloat( );
        [[nodiscard]] double             ReadDouble( );
        [[nodiscard]] DenOfIz_StringView ReadString( );
        [[nodiscard]] DenOfIz_UShort2    ReadUInt16_2( );
        [[nodiscard]] DenOfIz_UShort3    ReadUInt16_3( );
        [[nodiscard]] DenOfIz_UShort4    ReadUInt16_4( );
        [[nodiscard]] DenOfIz_Short2     ReadInt16_2( );
        [[nodiscard]] DenOfIz_Short3     ReadInt16_3( );
        [[nodiscard]] DenOfIz_Short4     ReadInt16_4( );
        [[nodiscard]] DenOfIz_UInt2      ReadUInt32_2( );
        [[nodiscard]] DenOfIz_UInt3      ReadUInt32_3( );
        [[nodiscard]] DenOfIz_UInt4      ReadUInt32_4( );
        [[nodiscard]] DenOfIz_Int2       ReadInt32_2( );
        [[nodiscard]] DenOfIz_Int3       ReadInt32_3( );
        [[nodiscard]] DenOfIz_Int4       ReadInt32_4( );
        [[nodiscard]] DenOfIz_Float2     ReadFloat_2( );
        [[nodiscard]] DenOfIz_Float3     ReadFloat_3( );
        [[nodiscard]] DenOfIz_Float4     ReadFloat_4( );
        [[nodiscard]] DenOfIz_Float4x4   ReadFloat_4x4( );
        [[nodiscard]] uint64_t           Position( ) const;
        void                             Seek( uint64_t position ) const;
        void                             Skip( uint64_t count ) const;

    private:
        [[nodiscard]] bool IsStreamValid( ) const;
        bool               TrackReadBytes( uint32_t requested );
    };
} // namespace DenOfIz

using namespace DenOfIz;

BinaryReader::BinaryReader( std::istream *stream, const BinaryReaderDesc &desc ) : m_allowedNumBytes( desc.NumBytes ), m_stream( stream )
{
    m_isStreamOwned = false;
    m_isStreamValid = true;
}

BinaryReader::BinaryReader( const DenOfIz_StringView &filePath, const BinaryReaderDesc &desc ) : m_allowedNumBytes( desc.NumBytes ), m_stream( nullptr )
{
    m_isStreamOwned = true;
    auto *stream    = new std::ifstream;

    const std::string resolvedPath = FileIO::GetResourcePath( filePath );
    stream->open( resolvedPath, std::ios::binary );
    if ( !stream->is_open( ) )
    {
        std::string filePathStr( filePath.Chars, filePath.NumChars );
        spdlog::error( "Failed to open file for reading: {}", filePathStr );
        m_isStreamValid = false;
        delete stream;
        return;
    }

    m_stream        = stream;
    m_isStreamValid = true;
}

BinaryReader::BinaryReader( const DenOfIz_ByteArrayView &data, const BinaryReaderDesc &desc ) : m_allowedNumBytes( desc.NumBytes )
{
    m_isStreamOwned = true;
    auto *stream    = new std::stringstream( std::ios::in | std::ios::out | std::ios::binary );

    if ( data.NumElements > 0 )
    {
        stream->write( reinterpret_cast<const char *>( data.Elements ), data.NumElements );
        stream->flush( );
        stream->seekg( 0, std::ios::beg );
    }

    m_stream         = stream;
    m_isStreamValid  = true;
    m_isStringStream = true;
}

BinaryReader::~BinaryReader( )
{
    if ( m_isStreamValid && m_isStreamOwned && m_stream != nullptr )
    {
        if ( !m_isStringStream && dynamic_cast<std::ifstream *>( m_stream )->is_open( ) )
        {
            dynamic_cast<std::ifstream *>( m_stream )->close( );
        }
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

int BinaryReader::Read( const DenOfIz_ByteArray &buffer, const uint32_t offset, const uint32_t count )
{
    if ( !IsStreamValid( ) || !TrackReadBytes( count ) || buffer.Elements == nullptr )
    {
        return -1;
    }

    m_stream->read( reinterpret_cast<char *>( buffer.Elements + offset ), count );
    return static_cast<int>( m_stream->gcount( ) );
}

DenOfIz_ByteArray BinaryReader::ReadAllBytes( )
{
    if ( !IsStreamValid( ) )
    {
        return { nullptr, 0 };
    }

    const std::streampos currentPos = m_stream->tellg( );

    m_stream->seekg( 0, std::ios::end );
    const std::streampos endPos = m_stream->tellg( );

    const uint32_t numTotalBytes = static_cast<uint32_t>( endPos - currentPos );
    m_stream->seekg( currentPos );

    return ReadBytes( numTotalBytes );
}

DenOfIz_ByteArray BinaryReader::ReadBytes( const uint32_t count )
{
    if ( !IsStreamValid( ) || !TrackReadBytes( count ) )
    {
        return { nullptr, 0 };
    }

    DenOfIz_ByteArray result{ };
    result.Elements    = static_cast<Byte *>( std::malloc( count ) );
    result.NumElements = count;
    if ( const int bytesRead = Read( result, 0, count ); bytesRead >= 0 && static_cast<uint32_t>( bytesRead ) < count )
    {
        DenOfIz_ByteArray resized{ };
        resized.Elements    = static_cast<Byte *>( std::realloc( result.Elements, bytesRead ) );
        resized.NumElements = bytesRead;
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
        spdlog::error( "Failed to read 2 bytes for uint16" );
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
        spdlog::error( "Failed to read 4 bytes for uint32" );
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

double BinaryReader::ReadDouble( )
{
    if ( !IsStreamValid( ) || !TrackReadBytes( 1 ) )
    {
        return 0.0f;
    }

    const uint64_t intValue = ReadUInt64( );
    double         result;
    std::memcpy( &result, &intValue, sizeof( double ) );
    return result;
}

DenOfIz_StringView BinaryReader::ReadString( )
{
    const uint32_t length = ReadUInt32( );
    if ( !IsStreamValid( ) || !TrackReadBytes( length ) )
    {
        return DENOFIZ_STRING( "" );
    }

    m_stringStorage.emplace_back( length + 1, '\0' );
    std::vector<char> &buffer = m_stringStorage.back( );

    m_stream->read( buffer.data( ), length );

    if ( static_cast<uint32_t>( m_stream->gcount( ) ) != length )
    {
        spdlog::error( "Failed to read full string, expected {} bytes", length );
    }

    return { buffer.data( ), length };
}

DenOfIz_UShort2 BinaryReader::ReadUInt16_2( )
{
    return { ReadUInt16( ), ReadUInt16( ) };
}

DenOfIz_UShort3 BinaryReader::ReadUInt16_3( )
{
    return { ReadUInt16( ), ReadUInt16( ), ReadUInt16( ) };
}

DenOfIz_UShort4 BinaryReader::ReadUInt16_4( )
{
    return { ReadUInt16( ), ReadUInt16( ), ReadUInt16( ), ReadUInt16( ) };
}

DenOfIz_Short2 BinaryReader::ReadInt16_2( )
{
    return { ReadInt16( ), ReadInt16( ) };
}

DenOfIz_Short3 BinaryReader::ReadInt16_3( )
{
    return { ReadInt16( ), ReadInt16( ), ReadInt16( ) };
}

DenOfIz_Short4 BinaryReader::ReadInt16_4( )
{
    return { ReadInt16( ), ReadInt16( ), ReadInt16( ), ReadInt16( ) };
}

DenOfIz_UInt2 BinaryReader::ReadUInt32_2( )
{
    return { ReadUInt32( ), ReadUInt32( ) };
}

DenOfIz_UInt3 BinaryReader::ReadUInt32_3( )
{
    return { ReadUInt32( ), ReadUInt32( ), ReadUInt32( ) };
}

DenOfIz_UInt4 BinaryReader::ReadUInt32_4( )
{
    return { ReadUInt32( ), ReadUInt32( ), ReadUInt32( ), ReadUInt32( ) };
}

DenOfIz_Int2 BinaryReader::ReadInt32_2( )
{
    return { ReadInt32( ), ReadInt32( ) };
}

DenOfIz_Int3 BinaryReader::ReadInt32_3( )
{
    return { ReadInt32( ), ReadInt32( ), ReadInt32( ) };
}

DenOfIz_Int4 BinaryReader::ReadInt32_4( )
{
    return { ReadInt32( ), ReadInt32( ), ReadInt32( ), ReadInt32( ) };
}

DenOfIz_Float2 BinaryReader::ReadFloat_2( )
{
    return { ReadFloat( ), ReadFloat( ) };
}

DenOfIz_Float3 BinaryReader::ReadFloat_3( )
{
    return { ReadFloat( ), ReadFloat( ), ReadFloat( ) };
}

DenOfIz_Float4 BinaryReader::ReadFloat_4( )
{
    return { ReadFloat( ), ReadFloat( ), ReadFloat( ), ReadFloat( ) };
}

DenOfIz_Float4x4 BinaryReader::ReadFloat_4x4( )
{
    DenOfIz_Float4x4 result{ };

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
        spdlog::error( "Attempted to read string beyond end of file" );
        return false;
    }

    return true;
}

bool BinaryReader::TrackReadBytes( const uint32_t requested )
{
    if ( m_allowedNumBytes > 0 )
    {
        if ( m_readNumBytes + requested <= m_allowedNumBytes )
        {
            m_readNumBytes += requested;
            return true;
        }
        spdlog::error( "Attempted to read beyond end of the allowed range" );
        return false;
    }
    return true;
}

#define BINARY_READER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::BinaryReader, handle )
#define BINARY_CONTAINER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::BinaryContainerImpl, handle )

extern "C"
{

    DenOfIz_BinaryReader DenOfIz_BinaryReader_CreateFromContainer( DenOfIz_BinaryContainer container, const DenOfIz_BinaryReaderDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( container ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        DenOfIz::BinaryReaderDesc cppDesc{ };
        if ( desc != NULL )
        {
            cppDesc.NumBytes = desc->NumBytes;
        }

        auto *reader = new DenOfIz::BinaryReader( BINARY_CONTAINER_IMPL( container )->Stream( ), cppDesc );
        return DENOFIZ_TO_HANDLE( reader );
    }

    DenOfIz_BinaryReader DenOfIz_BinaryReader_CreateFromFile( DenOfIz_StringView filePath, const DenOfIz_BinaryReaderDesc *desc )
    {
        DenOfIz::BinaryReaderDesc cppDesc{ };
        if ( desc != NULL )
        {
            cppDesc.NumBytes = desc->NumBytes;
        }

        auto *reader = new DenOfIz::BinaryReader( filePath, cppDesc );
        return DENOFIZ_TO_HANDLE( reader );
    }

    DenOfIz_BinaryReader DenOfIz_BinaryReader_CreateFromData( DenOfIz_ByteArrayView data, const DenOfIz_BinaryReaderDesc *desc )
    {
        DenOfIz::BinaryReaderDesc cppDesc{ };
        if ( desc != NULL )
        {
            cppDesc.NumBytes = desc->NumBytes;
        }

        auto *reader = new DenOfIz::BinaryReader( data, cppDesc );
        return DENOFIZ_TO_HANDLE( reader );
    }

    void DenOfIz_BinaryReader_Destroy( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return;
        }
        delete BINARY_READER_IMPL( reader );
    }

    int ReadByte( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return -1;
        }
        return BINARY_READER_IMPL( reader )->ReadByte( );
    }

    int DenOfIz_BinaryReader_Read( DenOfIz_BinaryReader reader, DenOfIz_ByteArray buffer, uint32_t offset, uint32_t count )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return -1;
        }
        return BINARY_READER_IMPL( reader )->Read( buffer, offset, count );
    }

    DenOfIz_ByteArray DenOfIz_BinaryReader_ReadAllBytes( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DenOfIz_ByteArray{ nullptr, 0 };
        }
        return BINARY_READER_IMPL( reader )->ReadAllBytes( );
    }

    DenOfIz_ByteArray DenOfIz_BinaryReader_ReadBytes( DenOfIz_BinaryReader reader, uint32_t count )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DenOfIz_ByteArray{ nullptr, 0 };
        }
        return BINARY_READER_IMPL( reader )->ReadBytes( count );
    }

    uint16_t DenOfIz_BinaryReader_ReadUInt16( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return 0;
        }
        return BINARY_READER_IMPL( reader )->ReadUInt16( );
    }

    uint32_t DenOfIz_BinaryReader_ReadUInt32( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return 0;
        }
        return BINARY_READER_IMPL( reader )->ReadUInt32( );
    }

    uint64_t DenOfIz_BinaryReader_ReadUInt64( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return 0;
        }
        return BINARY_READER_IMPL( reader )->ReadUInt64( );
    }

    int16_t DenOfIz_BinaryReader_ReadInt16( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return 0;
        }
        return BINARY_READER_IMPL( reader )->ReadInt16( );
    }

    int32_t DenOfIz_BinaryReader_ReadInt32( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return 0;
        }
        return BINARY_READER_IMPL( reader )->ReadInt32( );
    }

    int64_t DenOfIz_BinaryReader_ReadInt64( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return 0;
        }
        return BINARY_READER_IMPL( reader )->ReadInt64( );
    }

    float DenOfIz_BinaryReader_ReadFloat( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return 0.0f;
        }
        return BINARY_READER_IMPL( reader )->ReadFloat( );
    }

    double DenOfIz_BinaryReader_ReadDouble( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return 0.0;
        }
        return BINARY_READER_IMPL( reader )->ReadDouble( );
    }

    DenOfIz_StringView DenOfIz_BinaryReader_ReadString( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DenOfIz_StringView{ nullptr, 0 };
        }
        return BINARY_READER_IMPL( reader )->ReadString( );
    }

    DenOfIz_UShort2 DenOfIz_BinaryReader_ReadUInt16_2( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DenOfIz_UShort2{ 0, 0 };
        }
        return BINARY_READER_IMPL( reader )->ReadUInt16_2( );
    }

    DenOfIz_UShort3 DenOfIz_BinaryReader_ReadUInt16_3( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DenOfIz_UShort3{ 0, 0, 0 };
        }
        return BINARY_READER_IMPL( reader )->ReadUInt16_3( );
    }

    DenOfIz_UShort4 DenOfIz_BinaryReader_ReadUInt16_4( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DenOfIz_UShort4{ 0, 0, 0, 0 };
        }
        return BINARY_READER_IMPL( reader )->ReadUInt16_4( );
    }

    DenOfIz_Short2 DenOfIz_BinaryReader_ReadInt16_2( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DenOfIz_Short2{ 0, 0 };
        }
        return BINARY_READER_IMPL( reader )->ReadInt16_2( );
    }

    DenOfIz_Short3 DenOfIz_BinaryReader_ReadInt16_3( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DenOfIz_Short3{ 0, 0, 0 };
        }
        return BINARY_READER_IMPL( reader )->ReadInt16_3( );
    }

    DenOfIz_Short4 DenOfIz_BinaryReader_ReadInt16_4( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DenOfIz_Short4{ 0, 0, 0, 0 };
        }
        return BINARY_READER_IMPL( reader )->ReadInt16_4( );
    }

    DenOfIz_UInt2 DenOfIz_BinaryReader_ReadUInt32_2( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DenOfIz_UInt2{ 0, 0 };
        }
        return BINARY_READER_IMPL( reader )->ReadUInt32_2( );
    }

    DenOfIz_UInt3 DenOfIz_BinaryReader_ReadUInt32_3( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DenOfIz_UInt3{ 0, 0, 0 };
        }
        return BINARY_READER_IMPL( reader )->ReadUInt32_3( );
    }

    DenOfIz_UInt4 DenOfIz_BinaryReader_ReadUInt32_4( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DenOfIz_UInt4{ 0, 0, 0, 0 };
        }
        return BINARY_READER_IMPL( reader )->ReadUInt32_4( );
    }

    DenOfIz_Int2 DenOfIz_BinaryReader_ReadInt32_2( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DenOfIz_Int2{ 0, 0 };
        }
        return BINARY_READER_IMPL( reader )->ReadInt32_2( );
    }

    DenOfIz_Int3 DenOfIz_BinaryReader_ReadInt32_3( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DenOfIz_Int3{ 0, 0, 0 };
        }
        return BINARY_READER_IMPL( reader )->ReadInt32_3( );
    }

    DenOfIz_Int4 DenOfIz_BinaryReader_ReadInt32_4( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DenOfIz_Int4{ 0, 0, 0, 0 };
        }
        return BINARY_READER_IMPL( reader )->ReadInt32_4( );
    }

    DenOfIz_Float2 DenOfIz_BinaryReader_ReadFloat_2( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DenOfIz_Float2{ 0.0f, 0.0f };
        }
        return BINARY_READER_IMPL( reader )->ReadFloat_2( );
    }

    DenOfIz_Float3 DenOfIz_BinaryReader_ReadFloat_3( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DenOfIz_Float3{ 0.0f, 0.0f, 0.0f };
        }
        return BINARY_READER_IMPL( reader )->ReadFloat_3( );
    }

    DenOfIz_Float4 DenOfIz_BinaryReader_ReadFloat_4( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DenOfIz_Float4{ 0.0f, 0.0f, 0.0f, 0.0f };
        }
        return BINARY_READER_IMPL( reader )->ReadFloat_4( );
    }

    DenOfIz_Float4x4 DenOfIz_BinaryReader_ReadFloat_4x4( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DenOfIz_Float4x4{ };
        }
        return BINARY_READER_IMPL( reader )->ReadFloat_4x4( );
    }

    uint64_t DenOfIz_BinaryReader_Position( DenOfIz_BinaryReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return 0;
        }
        return BINARY_READER_IMPL( reader )->Position( );
    }

    void DenOfIz_BinaryReader_Seek( DenOfIz_BinaryReader reader, uint64_t position )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return;
        }
        BINARY_READER_IMPL( reader )->Seek( position );
    }

    void DenOfIz_BinaryReader_Skip( DenOfIz_BinaryReader reader, uint64_t count )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return;
        }
        BINARY_READER_IMPL( reader )->Skip( count );
    }
}
