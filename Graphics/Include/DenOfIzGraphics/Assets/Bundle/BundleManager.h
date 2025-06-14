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

#pragma once

#include <vector>
#include "DenOfIzGraphics/Assets/Bundle/Bundle.h"

namespace DenOfIz
{
    struct DZ_API BundleManagerDesc
    {
        InteropString DefaultSearchPath;
    };

    class BundleManager
    {
        std::vector<Bundle *>                     m_mountedBundles;
        InteropString                             m_defaultSearchPath;
        std::unordered_map<std::string, Bundle *> m_assetLocationCache;
        mutable std::vector<AssetUri>             m_allAssets;
        mutable std::vector<AssetUri>             m_assetsByType;

    public:
        DZ_API explicit BundleManager( const BundleManagerDesc &desc );
        DZ_API ~BundleManager( );

        DZ_API void MountBundle( Bundle *bundle );
        DZ_API void UnmountBundle( Bundle *bundle );
        DZ_API void MountDirectory( const InteropString &directoryPath, bool recursive = true, int priority = 0 );

        DZ_API BinaryReader *OpenReader( const AssetUri &path );
        DZ_API BinaryWriter *OpenWriter( const AssetUri &path );
        DZ_API void          AddAsset( Bundle *bundle, const AssetUri &path, AssetType type, const ByteArrayView &data );
        DZ_API bool          Exists( const AssetUri &path );

        DZ_API void InvalidateCache( );

        DZ_API InteropString ResolveToFilesystemPath( const AssetUri &path );
        DZ_API AssetUriArray GetAllAssets( ) const;
        DZ_API AssetUriArray GetAssetsByType( AssetType type ) const;
        DZ_API BundleArray   GetMountedBundles( ) const;
    };
} // namespace DenOfIz
