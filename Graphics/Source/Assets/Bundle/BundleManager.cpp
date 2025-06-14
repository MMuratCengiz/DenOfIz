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

#include "DenOfIzGraphics/Assets/Bundle/BundleManager.h"
#include <filesystem>
#include <ranges>
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

BundleManager::BundleManager( const BundleManagerDesc &desc ) : m_defaultSearchPath( desc.DefaultSearchPath )
{
}

BundleManager::~BundleManager( ) = default;

void BundleManager::MountBundle( Bundle *bundle )
{
    auto it = m_mountedBundles.begin( );
    while ( it != m_mountedBundles.end( ) )
    {
        ++it;
    }

    m_mountedBundles.insert( it, bundle );
    InvalidateCache( );
}

void BundleManager::UnmountBundle( Bundle *bundle )
{
    const auto it = std::ranges::find( m_mountedBundles, bundle );
    if ( it != m_mountedBundles.end( ) )
    {
        m_mountedBundles.erase( it );
        InvalidateCache( );
    }
}

void BundleManager::MountDirectory( const InteropString &directoryPath, bool recursive, int priority )
{
    BundleDirectoryDesc directoryDesc;
    directoryDesc.DirectoryPath = FileIO::GetResourcePath( directoryPath );
    directoryDesc.Recursive     = recursive;

    const std::string           dirStr = directoryDesc.DirectoryPath.Get( );
    const std::filesystem::path dir( dirStr );

    const std::filesystem::path bundlePath = dir / ( dir.filename( ).string( ) + ".dzbundle" );
    directoryDesc.OutputBundlePath         = InteropString( bundlePath.string( ).c_str( ) );

    Bundle *bundle = Bundle::CreateFromDirectory( directoryDesc );
    MountBundle( bundle );
}

BinaryReader *BundleManager::OpenReader( const AssetUri &path )
{
    const std::string uriStr = path.ToInteropString( ).Get( );

    if ( const auto cacheIt = m_assetLocationCache.find( uriStr ); cacheIt != m_assetLocationCache.end( ) )
    {
        return cacheIt->second->OpenReader( path );
    }

    for ( Bundle *bundle : m_mountedBundles )
    {
        if ( bundle->Exists( path ) )
        {
            m_assetLocationCache[ uriStr ] = bundle;
            return bundle->OpenReader( path );
        }
    }

    // If it's not in any bundle, return from filesystem
    const InteropString resolvedPath = FileIO::GetResourcePath( path.Path );
    if ( FileIO::FileExists( resolvedPath ) )
    {
        return new BinaryReader( resolvedPath );
    }

    return nullptr;
}

BinaryWriter *BundleManager::OpenWriter( const AssetUri &path )
{
    const std::string uriStr = path.ToInteropString( ).Get( );
    if ( const auto cacheIt = m_assetLocationCache.find( uriStr ); cacheIt != m_assetLocationCache.end( ) )
    {
        return cacheIt->second->OpenWriter( path );
    }

    for ( Bundle *bundle : m_mountedBundles )
    {
        if ( bundle->Exists( path ) )
        {
            m_assetLocationCache[ uriStr ] = bundle;
            return bundle->OpenWriter( path );
        }
    }

    // If it's not in any bundle, return from filesystem
    const InteropString resolvedPath = FileIO::GetResourcePath( path.Path );
    if ( FileIO::FileExists( resolvedPath ) )
    {
        return new BinaryWriter( resolvedPath );
    }

    return nullptr;
}

void BundleManager::AddAsset( Bundle *bundle, const AssetUri &path, AssetType type, const ByteArrayView &data )
{
    if ( !bundle )
    {
        if ( m_mountedBundles.empty( ) )
        {
            spdlog::error( "Cannot add asset: no bundle provided and no mounted bundles available" );
            return;
        }
        bundle = m_mountedBundles[ 0 ];
    }

    if ( std::ranges::find( m_mountedBundles, bundle ) == m_mountedBundles.end( ) )
    {
        spdlog::warn( "Adding asset to a bundle that is not mounted in this manager" );
    }

    bundle->AddAsset( path, type, data );
    const std::string uriStr       = path.ToInteropString( ).Get( );
    m_assetLocationCache[ uriStr ] = bundle;
}

bool BundleManager::Exists( const AssetUri &path )
{
    const std::string uriStr = path.ToInteropString( ).Get( );
    if ( const auto cacheIt = m_assetLocationCache.find( uriStr ); cacheIt != m_assetLocationCache.end( ) )
    {
        return true;
    }

    for ( Bundle *bundle : m_mountedBundles )
    {
        if ( bundle->Exists( path ) )
        {
            m_assetLocationCache[ uriStr ] = bundle;
            return true;
        }
    }

    const InteropString resolvedPath = FileIO::GetResourcePath( path.Path );
    if ( FileIO::FileExists( resolvedPath ) )
    {
        return true;
    }

    return false;
}

void BundleManager::InvalidateCache( )
{
    m_assetLocationCache.clear( );
    m_allAssets.clear( );
    m_assetsByType.clear( );
}

InteropString BundleManager::ResolveToFilesystemPath( const AssetUri &path )
{
    const std::string pathStr = path.Path.Get( );

    if ( const auto cacheIt = m_assetLocationCache.find( pathStr ); cacheIt != m_assetLocationCache.end( ) )
    {
        return InteropString{ };
    }

    const std::filesystem::path searchPath( m_defaultSearchPath.Get( ) );
    const std::filesystem::path assetPath( pathStr );
    const std::filesystem::path fullPath = searchPath / assetPath;

    InteropString resolvedPath = FileIO::GetResourcePath( InteropString( fullPath.string( ).c_str( ) ) );
    if ( FileIO::FileExists( resolvedPath ) )
    {
        return resolvedPath;
    }

    return InteropString{ };
}

AssetUriArray BundleManager::GetAllAssets( ) const
{
    m_allAssets.clear( );

    for ( const Bundle *bundle : m_mountedBundles )
    {
        const AssetUriArray bundleAssets = bundle->GetAllAssets( );
        for ( uint32_t i = 0; i < bundleAssets.NumElements; ++i )
        {
            m_allAssets.push_back( bundleAssets.Elements[ i ] );
        }
    }

    AssetUriArray result;
    result.Elements    = m_allAssets.data( );
    result.NumElements = static_cast<uint32_t>( m_allAssets.size( ) );
    return result;
}

AssetUriArray BundleManager::GetAssetsByType( const AssetType type ) const
{
    m_assetsByType.clear( );

    for ( const Bundle *bundle : m_mountedBundles )
    {
        const AssetUriArray bundleAssets = bundle->GetAssetsByType( type );
        for ( uint32_t i = 0; i < bundleAssets.NumElements; ++i )
        {
            m_assetsByType.push_back( bundleAssets.Elements[ i ] );
        }
    }

    AssetUriArray result;
    result.Elements    = m_assetsByType.data( );
    result.NumElements = static_cast<uint32_t>( m_assetsByType.size( ) );
    return result;
}

BundleArray BundleManager::GetMountedBundles( ) const
{
    BundleArray result{ };
    result.Elements    = const_cast<Bundle **>( m_mountedBundles.data( ) );
    result.NumElements = static_cast<uint32_t>( m_mountedBundles.size( ) );
    return result;
}
