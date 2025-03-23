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

// BundleManager.h
#pragma once
#include <DenOfIzGraphics/Assets/Bundle.h>
#include <vector>

namespace DenOfIz
{
    struct DZ_API BundleManagerDesc
    {
        InteropString DefaultSearchPath;  // Default path to look for assets
    };

    class DZ_API BundleManager
    {
    private:
        std::vector<Bundle*> m_mountedBundles;
        InteropString m_defaultSearchPath;
        std::unordered_map<std::string, Bundle*> m_assetLocationCache;

    public:
        DZ_API explicit BundleManager(const BundleManagerDesc& desc);
        DZ_API ~BundleManager();

        // Mount/unmount bundles
        DZ_API void MountBundle(Bundle* bundle, int priority = 0);
        DZ_API void UnmountBundle(Bundle* bundle);

        // Asset access
        DZ_API BinaryReader* OpenReader(const AssetPath& path);
        DZ_API BinaryWriter* OpenWriter(const AssetPath& path, AssetType type);
        DZ_API bool Exists(const AssetPath& path);

        // Asset management
        DZ_API AssetType GetAssetType(const AssetPath& path);
        DZ_API void InvalidateCache();  // Call when bundles change

        // Helper to convert asset paths to filesystem paths
        DZ_API InteropString ResolveToFilesystemPath(const AssetPath& path);
    };
}