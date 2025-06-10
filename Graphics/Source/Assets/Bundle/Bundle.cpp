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

#include "DenOfIzGraphics/Assets/Bundle/Bundle.h"
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <miniz/miniz.h>
#include <ranges>
#include <string>
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

Bundle::Bundle( const BundleDesc &desc ) : m_desc( desc ), m_bundleFile( nullptr ), m_isDirty( false ), m_isCompressed( desc.Compress )
{
    const InteropString resolvedPath = FileIO::GetResourcePath( desc.Path );
    if ( FileIO::FileExists( resolvedPath ) )
    {
        m_bundleFile = new std::fstream( resolvedPath.Get( ), std::ios::binary | std::ios::in | std::ios::out );
        LoadTableOfContents( );
    }
    else if ( desc.CreateIfNotExists )
    {
        const std::filesystem::path parentPath = std::filesystem::path( resolvedPath.Get( ) ).parent_path( );
        if ( !parentPath.empty( ) && !std::filesystem::exists( parentPath ) )
        {
            FileIO::CreateDirectories( InteropString( parentPath.string( ).c_str( ) ) );
        }

        m_bundleFile = new std::fstream( resolvedPath.Get( ), std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc );
        WriteEmptyHeader( );
    }
}

Bundle::Bundle( const BundleDirectoryDesc &directoryDesc ) : m_bundleFile( nullptr ), m_isDirty( false ), m_isCompressed( directoryDesc.Compress )
{
    BundleDesc desc;
    desc.Path              = FileIO::GetResourcePath( directoryDesc.OutputBundlePath );
    desc.CreateIfNotExists = true;
    desc.Compress          = directoryDesc.Compress;
    m_desc                 = desc;

    const std::filesystem::path bundlePath( desc.Path.Get( ) );
    const std::filesystem::path parentPath = bundlePath.parent_path( );
    if ( !parentPath.empty( ) && !std::filesystem::exists( parentPath ) )
    {
        FileIO::CreateDirectories( InteropString( parentPath.string( ).c_str( ) ) );
    }

    m_bundleFile = new std::fstream( desc.Path.Get( ), std::ios::binary | std::ios::in | std::ios::out | std::ios::trunc );
    WriteEmptyHeader( );

    const InteropString resolvedDirPath = FileIO::GetResourcePath( directoryDesc.DirectoryPath );
    const std::string   dirPath         = resolvedDirPath.Get( );

    if ( !FileIO::FileExists( resolvedDirPath ) || !std::filesystem::is_directory( dirPath ) )
    {
        spdlog::error( "Directory does not exist or is not a directory: {}", dirPath );
        return;
    }

    const std::filesystem::path basePath = dirPath;
    auto                        visitor  = [ & ]( const std::filesystem::path &path )
    {
        if ( !std::filesystem::is_regular_file( path ) )
        {
            return;
        }

        const std::filesystem::path relativePath = std::filesystem::relative( path, basePath );
        const std::string           relPathStr   = relativePath.string( );

        std::string extension = path.extension( ).string( );
        if ( !extension.empty( ) && extension[ 0 ] == '.' )
        {
            extension = extension.substr( 1 );
        }
        const AssetType assetType = DetermineAssetTypeFromExtension( InteropString( extension.c_str( ) ) );
        if ( directoryDesc.AssetTypeFilter.NumElements( ) > 0 )
        {
            bool matchesFilter = false;
            for ( size_t i = 0; i < directoryDesc.AssetTypeFilter.NumElements( ); ++i )
            {
                if ( directoryDesc.AssetTypeFilter.GetElement( i ) == assetType )
                {
                    matchesFilter = true;
                    break;
                }
            }
            if ( !matchesFilter )
            {
                return;
            }
        }

        const InteropString filePath( path.string( ).c_str( ) );
        if ( FileIO::FileExists( filePath ) )
        {
            ByteArray fileData = FileIO::ReadFile( filePath );
            const AssetUri  assetUri = AssetUri::Create( InteropString( relPathStr.c_str( ) ) );
            AddAsset( assetUri, assetType, ByteArrayView( fileData ) );
            fileData.Dispose();
        }
        else
        {
            spdlog::error( "Failed to add asset: file does not exist: {}", relPathStr );
        }
    };

    if ( directoryDesc.Recursive )
    {
        for ( const auto &entry : std::filesystem::recursive_directory_iterator( dirPath ) )
        {
            visitor( entry.path( ) );
        }
    }
    else
    {
        for ( const auto &entry : std::filesystem::directory_iterator( dirPath ) )
        {
            visitor( entry.path( ) );
        }
    }

    Save( );
}

Bundle::~Bundle( )
{
    delete m_bundleFile;
}

Bundle *Bundle::CreateFromDirectory( const BundleDirectoryDesc &directoryDesc )
{
    return new Bundle( directoryDesc );
}

BinaryReader *Bundle::OpenReader( const AssetUri &assetUri )
{
    const std::string uriStr = assetUri.ToInteropString( ).Get( );
    const auto        it     = m_assetEntries.find( uriStr );
    if ( it != m_assetEntries.end( ) )
    {
        BinaryReaderDesc desc{ };
        desc.NumBytes = it->second.NumBytes;

        if ( m_isCompressed )
        {
            m_bundleFile->seekg( it->second.Offset, std::ios::beg );
            BinaryReader fileReader( m_bundleFile );

            const uint64_t  compressedSize = fileReader.ReadUInt64( );
            const ByteArray compressedData = fileReader.ReadBytes( static_cast<uint32_t>( compressedSize ) );

            std::vector<Byte> decompressedData( it->second.NumBytes );
            mz_ulong          decompressedSize = static_cast<mz_ulong>( it->second.NumBytes );
            const int         result           = mz_uncompress( decompressedData.data( ), &decompressedSize, compressedData.Elements, static_cast<mz_ulong>( compressedSize ) );

            if ( result != MZ_OK )
            {
                spdlog::error( "Failed to decompress asset: {}", uriStr );
                return nullptr;
            }

            ByteArray decompressedDataArray{ };
            decompressedDataArray.Elements    = decompressedData.data( );
            decompressedDataArray.NumElements = decompressedSize;
            return new BinaryReader( ByteArrayView( decompressedDataArray ) );
        }

        // Uncompressed data
        m_bundleFile->seekg( it->second.Offset, std::ios::beg );
        BinaryReader    fileReader( m_bundleFile );
        const ByteArray buffer = fileReader.ReadBytes( static_cast<uint32_t>( it->second.NumBytes ) );
        return new BinaryReader( ByteArrayView( buffer ) );
    }

    // If it's not in the bundle, check if we can find it in the filesystem, useful for dev mode
    const InteropString fsPath = FileIO::GetResourcePath( assetUri.Path );
    if ( FileIO::FileExists( fsPath ) )
    {
        return new BinaryReader( fsPath );
    }

    return nullptr;
}

BinaryWriter *Bundle::OpenWriter( const AssetUri &assetUri )
{
    const std::string uriStr = assetUri.ToInteropString( ).Get( );
    if ( const auto it = m_assetEntries.find( uriStr ); it != m_assetEntries.end( ) )
    {
        constexpr BinaryWriterDesc desc{ };
        return new BinaryWriter( m_bundleFile, desc );
    }
    // If it's not in the bundle, check if we can find it in the filesystem, useful for dev mode
    const InteropString fsPath = FileIO::GetResourcePath( assetUri.Path );
    if ( FileIO::FileExists( fsPath ) )
    {
        return new BinaryWriter( fsPath );
    }

    return nullptr;
}

void Bundle::LoadTableOfContents( )
{
    if ( !m_bundleFile->good( ) )
    {
        spdlog::error( "Failed to read bundle: invalid file stream" );
        return;
    }

    BinaryReader reader( m_bundleFile );

    BundleHeader header;
    header.Magic        = reader.ReadUInt64( );
    header.Version      = reader.ReadUInt32( );
    header.NumBytes     = reader.ReadUInt32( );
    header.NumAssets    = reader.ReadUInt32( );
    header.TOCOffset    = reader.ReadUInt64( );
    header.IsCompressed = reader.ReadByte( ) != 0;

    if ( header.Magic != BundleHeader::BundleHeaderMagic )
    {
        spdlog::error( "Invalid bundle format: incorrect magic number" );
        return;
    }

    if ( header.Version > BundleHeader::Latest )
    {
        spdlog::error( "Unsupported bundle version: {}", header.Version );
        return;
    }

    m_isCompressed = header.IsCompressed;
    m_assetEntries.clear( );
    reader.Seek( header.TOCOffset );

    for ( uint32_t i = 0; i < header.NumAssets; i++ )
    {
        BundleTOCEntry tocEntry;
        tocEntry.AssetTypeId = reader.ReadUInt32( );
        tocEntry.Offset      = reader.ReadUInt64( );
        tocEntry.NumBytes    = reader.ReadUInt64( );
        tocEntry.PathLength  = reader.ReadUInt32( );

        const InteropString path = reader.ReadString( );

        AssetEntry entry;
        entry.Type     = static_cast<AssetType>( tocEntry.AssetTypeId );
        entry.Offset   = tocEntry.Offset;
        entry.NumBytes = tocEntry.NumBytes;
        entry.Path     = path;

        std::string pathStr       = path.Get( );
        m_assetEntries[ pathStr ] = entry;
    }

    spdlog::info( "Loaded bundle TOC: {} assets", header.NumAssets );
}

void Bundle::WriteEmptyHeader( ) const
{
    if ( !m_bundleFile->good( ) )
    {
        spdlog::error( "Failed to write bundle: invalid file stream" );
        return;
    }

    const BinaryWriter writer( m_bundleFile );
    BundleHeader       header;
    header.Magic        = BundleHeader::BundleHeaderMagic;
    header.Version      = BundleHeader::Latest;
    header.NumAssets    = 0;
    header.TOCOffset    = sizeof( BundleHeader );
    header.IsCompressed = m_isCompressed;

    writer.WriteUInt64( header.Magic );
    writer.WriteUInt32( header.Version );
    writer.WriteUInt32( header.NumBytes );
    writer.WriteUInt32( header.NumAssets );
    writer.WriteUInt64( header.TOCOffset );
    writer.WriteByte( header.IsCompressed ? 1 : 0 );

    writer.Seek( header.TOCOffset );
    writer.Flush( );

    spdlog::info( "Created new empty bundle" );
}

void Bundle::AddAsset( const AssetUri &assetUri, const AssetType type, const ByteArrayView &data )
{
    if ( !m_bundleFile || !m_bundleFile->good( ) )
    {
        spdlog::error( "Failed to add asset: invalid file stream" );
        return;
    }

    const std::string uriStr = assetUri.ToInteropString( ).Get( );
    if ( const auto existingIt = m_assetEntries.find( uriStr ); existingIt != m_assetEntries.end( ) )
    {
        spdlog::warn( "Asset already exists in bundle, replacing: {}", uriStr );
    }

    m_bundleFile->seekp( 0, std::ios::end );
    const uint64_t assetOffset = m_bundleFile->tellp( );
    const uint64_t numBytes    = data.NumElements;

    const BinaryWriter writer( m_bundleFile );
    if ( m_isCompressed )
    {
        const mz_ulong    sourceSize = static_cast<mz_ulong>( numBytes );
        const mz_ulong    destSize   = mz_compressBound( sourceSize );
        std::vector<Byte> compressedData( destSize );

        mz_ulong  compressedSize = destSize;
        const int result         = mz_compress( compressedData.data( ), &compressedSize, data.Elements, sourceSize );

        if ( result != MZ_OK )
        {
            spdlog::error( "Failed to compress asset: {}", uriStr );
            return;
        }

        std::vector<Byte> compressedInteropData( compressedSize );
        for ( size_t i = 0; i < compressedSize; ++i )
        {
            compressedInteropData[ i ] = compressedData[ i ];
        }

        writer.WriteUInt64( compressedSize );
        ByteArrayView compressedDataArray{ };
        compressedDataArray.Elements    = compressedInteropData.data( );
        compressedDataArray.NumElements = compressedInteropData.size( );
        writer.WriteBytes( compressedDataArray );
    }
    else
    {
        writer.WriteBytes( data );
    }

    AssetEntry entry;
    entry.Type     = type;
    entry.Offset   = assetOffset;
    entry.NumBytes = numBytes;
    entry.Path     = assetUri.Path;

    m_assetEntries[ uriStr ] = entry;
    m_isDirty                = true;

    spdlog::info( "Added asset to bundle: {} ({} bytes)", uriStr, numBytes );
}

bool Bundle::Exists( const AssetUri &assetUri ) const
{
    return m_assetEntries.contains( assetUri.ToInteropString( ).Get( ) );
}

bool Bundle::Save( )
{
    if ( !m_bundleFile || !m_bundleFile->good( ) )
    {
        spdlog::error( "Failed to save bundle: invalid file stream" );
        return false;
    }

    m_bundleFile->seekg( 0, std::ios::end );
    const uint64_t currentSize  = m_bundleFile->tellg( );
    const uint64_t newTocOffset = currentSize;

    const BinaryWriter writer( m_bundleFile );
    writer.Seek( newTocOffset );

    for ( const auto &entry : m_assetEntries )
    {
        writer.WriteUInt32( static_cast<uint32_t>( entry.second.Type ) );
        writer.WriteUInt64( entry.second.Offset );
        writer.WriteUInt64( entry.second.NumBytes );
        writer.WriteUInt32( static_cast<uint32_t>( entry.first.length( ) ) );
        writer.WriteString( InteropString( entry.first.c_str( ) ) );
    }

    BundleHeader header;
    header.Magic        = BundleHeader::BundleHeaderMagic;
    header.Version      = BundleHeader::Latest;
    header.NumAssets    = static_cast<uint32_t>( m_assetEntries.size( ) );
    header.TOCOffset    = newTocOffset;
    header.IsCompressed = m_isCompressed;

    writer.Seek( 0 );
    writer.WriteUInt64( header.Magic );
    writer.WriteUInt32( header.Version );
    writer.WriteUInt32( header.NumBytes );
    writer.WriteUInt32( header.NumAssets );
    writer.WriteUInt64( header.TOCOffset );
    writer.WriteByte( header.IsCompressed ? 1 : 0 );

    writer.Flush( );
    m_isDirty = false;

    spdlog::info( "Saved bundle with {} assets", header.NumAssets );
    return true;
}

InteropArray<AssetUri> Bundle::GetAllAssets( ) const
{
    InteropArray<AssetUri> result( m_assetEntries.size( ) );
    size_t                 index = 0;
    for ( const auto &key : m_assetEntries | std::views::keys )
    {
        const AssetUri uri = AssetUri::Parse( InteropString( key.c_str( ) ) );
        result.SetElement( index++, uri );
    }
    return result;
}

InteropArray<AssetUri> Bundle::GetAssetsByType( const AssetType type ) const
{
    size_t count = 0;
    for ( const auto &val : m_assetEntries | std::views::values )
    {
        if ( val.Type == type )
        {
            count++;
        }
    }

    InteropArray<AssetUri> result( count );
    size_t                 index = 0;

    for ( const auto &entry : m_assetEntries )
    {
        if ( entry.second.Type == type )
        {
            const AssetUri uri = AssetUri::Parse( InteropString( entry.first.c_str( ) ) );
            result.SetElement( index++, uri );
        }
    }

    return result;
}

bool Bundle::IsCompressed( ) const
{
    return m_isCompressed;
}

const InteropString &Bundle::GetPath( ) const
{
    return m_desc.Path;
}

AssetType Bundle::DetermineAssetTypeFromExtension( const InteropString &extension )
{
    std::string ext = extension.Get( );
    std::ranges::transform( ext, ext.begin( ), tolower );

    // Could also use Asset::Extension( ) but would add dependency
    if ( ext == "dzmesh" )
    {
        return AssetType::Mesh;
    }
    if ( ext == "dzmat" )
    {
        return AssetType::Material;
    }
    if ( ext == "dztex" )
    {
        return AssetType::Texture;
    }
    if ( ext == "dzanim" )
    {
        return AssetType::Animation;
    }
    if ( ext == "dzskel" )
    {
        return AssetType::Skeleton;
    }
    if ( ext == "dzphys" )
    {
        return AssetType::Physics;
    }
    if ( ext == "dzfont" )
    {
        return AssetType::Font;
    }
    return AssetType::Unknown;
}
