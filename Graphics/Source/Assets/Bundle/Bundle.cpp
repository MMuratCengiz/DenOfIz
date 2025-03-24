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

#include <DenOfIzGraphics/Assets/Bundle/Bundle.h>
#include <filesystem>

using namespace DenOfIz;

Bundle::Bundle( const BundleDesc &desc ) : m_desc( desc ), m_bundleFile( nullptr ), m_isDirty( false )
{

    if ( std::filesystem::exists( desc.Path.Get( ) ) )
    {
        m_bundleFile = new std::fstream( desc.Path.Get( ), std::ios::binary | std::ios::in | std::ios::out );
        LoadTableOfContents( );
    }
    else if ( desc.CreateIfNotExists )
    {
        m_bundleFile = new std::fstream( desc.Path.Get( ), std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc );
        WriteEmptyHeader( );
    }
}

Bundle::~Bundle( ) = default;

BinaryReader *Bundle::OpenReader( const AssetUri &assetUri )
{
    const std::string pathStr = assetUri.ToString( ).Get( );

    if ( const auto it = m_assetEntries.find( pathStr ); it != m_assetEntries.end( ) )
    {
        BinaryReaderDesc desc{ };
        desc.NumBytes = it->second.NumBytes;
        return new BinaryReader( m_bundleFile, desc );
    }

    const std::string fsPath = assetUri.Path.Get( );
    if ( std::filesystem::exists( fsPath ) )
    {
        return new BinaryReader( InteropString( fsPath.c_str( ) ) );
    }

    return nullptr;
}

BinaryWriter *Bundle::OpenWriter( const AssetUri &assetUri )
{
    const std::string pathStr = assetUri.ToString( ).Get( );

    if ( const auto it = m_assetEntries.find( pathStr ); it != m_assetEntries.end( ) )
    {
        constexpr BinaryWriterDesc desc{ };
        return new BinaryWriter( m_bundleFile, desc );
    }

    const std::string fsPath = assetUri.Path.Get( );
    if ( std::filesystem::exists( fsPath ) )
    {
        return new BinaryWriter( InteropString( fsPath.c_str( ) ) );
    }

    return nullptr;
}

void Bundle::LoadTableOfContents( )
{
    if ( !m_bundleFile->good( ) )
    {
        LOG( ERROR ) << "Failed to read bundle: invalid file stream";
        return;
    }

    BundleHeader header;
    m_bundleFile->seekg( 0, std::ios::beg );
    m_bundleFile->read( reinterpret_cast<char *>( &header ), sizeof( BundleHeader ) );

    if ( header.Magic != BundleHeader::BundleHeaderMagic )
    {
        LOG( ERROR ) << "Invalid bundle format: incorrect magic number";
        return;
    }

    if ( header.Version > BundleHeader::Latest )
    {
        LOG( ERROR ) << "Unsupported bundle version: " << header.Version;
        return;
    }

    m_assetEntries.clear( );
    m_bundleFile->seekg( header.TOCOffset, std::ios::beg );

    for ( uint32_t i = 0; i < header.NumAssets; i++ )
    {
        BundleTOCEntry tocEntry{ };
        m_bundleFile->read( reinterpret_cast<char *>( &tocEntry ), sizeof( BundleTOCEntry ) );

        std::vector<char> pathBuffer( tocEntry.PathLength + 1, 0 );
        m_bundleFile->read( pathBuffer.data( ), tocEntry.PathLength );

        AssetEntry entry;
        entry.Type     = static_cast<AssetType>( tocEntry.AssetTypeId );
        entry.Offset   = tocEntry.Offset;
        entry.NumBytes = tocEntry.NumBytes;
        entry.Path     = InteropString( pathBuffer.data( ) );

        std::string pathStr       = pathBuffer.data( );
        m_assetEntries[ pathStr ] = entry;
    }

    LOG( INFO ) << "Loaded bundle TOC: " << header.NumAssets << " assets";
}

void Bundle::WriteEmptyHeader( ) const
{
    if ( !m_bundleFile || !m_bundleFile->good( ) )
    {
        LOG( ERROR ) << "Failed to write bundle: invalid file stream";
        return;
    }

    BundleHeader header;
    header.Magic     = BundleHeader::BundleHeaderMagic;
    header.Version   = BundleHeader::Latest;
    header.NumAssets = 0;
    header.TOCOffset = sizeof( BundleHeader ); // TOC starts right after header

    m_bundleFile->seekp( 0, std::ios::beg );
    m_bundleFile->write( reinterpret_cast<const char *>( &header ), sizeof( BundleHeader ) );

    m_bundleFile->seekp( header.TOCOffset, std::ios::beg );
    m_bundleFile->flush( );

    LOG( INFO ) << "Created new empty bundle";
}

void Bundle::AddAsset( const AssetUri &assetUri, const AssetType type, const InteropArray<Byte> &data )
{
    if ( !m_bundleFile || !m_bundleFile->good( ) )
    {
        LOG( ERROR ) << "Failed to add asset: invalid file stream";
        return;
    }

    const std::string pathStr = assetUri.Path.Get( );
    if ( const auto existingIt = m_assetEntries.find( pathStr ); existingIt != m_assetEntries.end( ) )
    {
        LOG( WARNING ) << "Asset already exists in bundle, replacing: " << pathStr;
        // We'll just overwrite the existing entry below
    }

    m_bundleFile->seekp( 0, std::ios::end );
    const uint64_t assetOffset = m_bundleFile->tellp( );
    const uint64_t numBytes    = data.NumElements( );
    m_bundleFile->write( reinterpret_cast<const char *>( data.Data( ) ), numBytes );

    AssetEntry entry;
    entry.Type     = type;
    entry.Offset   = assetOffset;
    entry.NumBytes = numBytes;
    entry.Path     = assetUri.Path;

    m_assetEntries[ pathStr ] = entry;
    m_isDirty                 = true;

    LOG( INFO ) << "Added asset to bundle: " << pathStr << " (" << numBytes << " bytes)";
}

bool Bundle::Exists( const AssetUri &assetUri ) const
{
    const std::string pathStr = assetUri.Path.Get( );
    return m_assetEntries.contains( pathStr );
}

bool Bundle::Save( )
{
    if ( !m_bundleFile || !m_bundleFile->good( ) )
    {
        LOG( ERROR ) << "Failed to save bundle: invalid file stream";
        return false;
    }

    m_bundleFile->seekg( 0, std::ios::end );
    const uint64_t currentSize  = m_bundleFile->tellg( );
    const uint64_t newTocOffset = currentSize;

    m_bundleFile->seekp( newTocOffset, std::ios::beg );

    for ( const auto &entry : m_assetEntries )
    {
        BundleTOCEntry tocEntry{ };
        tocEntry.AssetTypeId = static_cast<uint32_t>( entry.second.Type );
        tocEntry.Offset      = entry.second.Offset;
        tocEntry.NumBytes    = entry.second.NumBytes;
        tocEntry.PathLength  = static_cast<uint32_t>( entry.first.length( ) );

        m_bundleFile->write( reinterpret_cast<const char *>( &tocEntry ), sizeof( BundleTOCEntry ) );
        m_bundleFile->write( entry.first.c_str( ), tocEntry.PathLength );
    }

    BundleHeader header;
    header.Magic     = BundleHeader::BundleHeaderMagic;
    header.Version   = BundleHeader::Latest;
    header.NumAssets = static_cast<uint32_t>( m_assetEntries.size( ) );
    header.TOCOffset = newTocOffset;

    m_bundleFile->seekp( 0, std::ios::beg );
    m_bundleFile->write( reinterpret_cast<const char *>( &header ), sizeof( BundleHeader ) );

    m_bundleFile->flush( );
    m_isDirty = false;

    LOG( INFO ) << "Saved bundle with " << header.NumAssets << " assets";
    return true;
}
