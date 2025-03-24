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

#include <DenOfIzGraphics/Assets/Bundle/BundleManager.h>
#include <filesystem>

using namespace DenOfIz;

BundleManager::BundleManager( const BundleManagerDesc &desc ) : m_defaultSearchPath( desc.DefaultSearchPath )
{
}

BundleManager::~BundleManager( ) = default;

void BundleManager::MountBundle( Bundle *bundle, const int priority )
{
    auto it = m_mountedBundles.begin( );
    while ( it != m_mountedBundles.end( ) && priority <= 0 )
    {
        ++it;
    }

    m_mountedBundles.insert( it, bundle );
    InvalidateCache( );
}

BinaryReader *BundleManager::OpenReader( const AssetUri &path )
{
    const std::string pathStr = path.Path.Get( );

    if ( const auto cacheIt = m_assetLocationCache.find( pathStr ); cacheIt != m_assetLocationCache.end( ) )
    {
        return cacheIt->second->OpenReader( path );
    }

    for ( Bundle *bundle : m_mountedBundles )
    {
        if ( bundle->Exists( path ) )
        {
            m_assetLocationCache[ pathStr ] = bundle;
            return bundle->OpenReader( path );
        }
    }

    const std::string fsPath = m_defaultSearchPath.Get( ) + std::string( "/" ) + pathStr;
    if ( std::filesystem::exists( fsPath ) )
    {
        return new BinaryReader( InteropString( fsPath.c_str( ) ) );
    }

    return nullptr;
}

void BundleManager::InvalidateCache( )
{
    m_assetLocationCache.clear( );
}
