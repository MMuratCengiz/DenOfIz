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

#include <fstream>
#include <unordered_map>
#include "DenOfIzGraphics/Assets/Serde/Asset.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryReader.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"
#include "DenOfIzGraphics/Utilities/Interop.h"

namespace DenOfIz
{
    enum class AssetType
    {
        Unknown,
        Mesh,
        Material,
        Texture,
        Animation,
        Skeleton,
        Physics,
        Shader,
        Font
    };

    struct DZ_API AssetTypeArray
    {
        AssetType *Elements;
        uint32_t   NumElements;
    };

    struct DZ_API AssetEntry
    {
        AssetType     Type;
        uint64_t      Offset;
        uint64_t      NumBytes;
        InteropString Name;
        InteropString Path;
    };

    struct DZ_API AssetEntryArray
    {
        AssetEntry *Elements;
        uint32_t    NumElements;
    };

    struct DZ_API BundleHeader : AssetHeader
    {
        static constexpr uint64_t BundleHeaderMagic = 0x445A42554E444C; // "DZBUNDL"
        static constexpr uint32_t Latest            = 1;

        uint32_t NumAssets    = 0;
        uint64_t TOCOffset    = 0;
        bool     IsCompressed = false;

        BundleHeader( ) : AssetHeader( BundleHeaderMagic, Latest, 0 )
        {
        }
    };

    struct DZ_API BundleTOCEntry
    {
        uint32_t AssetTypeId;
        uint64_t Offset;
        uint64_t NumBytes;
        uint32_t PathLength;
    };

    struct DZ_API BundleDesc
    {
        InteropString Path;
        bool          CreateIfNotExists = false;
        bool          Compress          = false;
    };

    struct DZ_API BundleDirectoryDesc
    {
        InteropString  DirectoryPath;
        InteropString  OutputBundlePath;
        bool           Recursive = true;
        bool           Compress  = false;
        AssetTypeArray AssetTypeFilter; // Empty/Null means include all types
    };

    struct DZ_API BundleAssetFilter
    {
        AssetTypeArray Types;
        InteropString  ExtensionFilter; // Empty means include all extensions
        InteropString  PathFilter;      // Empty means include all paths
    };

    class Bundle
    {
        BundleDesc                                  m_desc;
        std::unordered_map<std::string, AssetEntry> m_assetEntries;
        std::fstream                               *m_bundleFile;
        bool                                        m_isDirty;
        bool                                        m_isCompressed;
        mutable std::vector<AssetUri>               m_allAssets;
        mutable std::vector<AssetUri>               m_assetsByType;

        void             LoadTableOfContents( );
        void             WriteEmptyHeader( ) const;
        static AssetType DetermineAssetTypeFromExtension( const InteropString &extension );

    public:
        DZ_API explicit Bundle( const BundleDesc &desc );
        DZ_API explicit Bundle( const BundleDirectoryDesc &directoryDesc );
        DZ_API ~Bundle( );

        DZ_API BinaryReader                      *OpenReader( const AssetUri &assetUri );
        DZ_API BinaryWriter                      *OpenWriter( const AssetUri &assetUri );
        DZ_API void                               AddAsset( const AssetUri &assetUri, AssetType type, const ByteArrayView &data );
        DZ_API bool                               Save( );
        DZ_API [[nodiscard]] bool                 Exists( const AssetUri &assetUri ) const;
        DZ_API [[nodiscard]] AssetUriArray        GetAllAssets( ) const;
        DZ_API [[nodiscard]] AssetUriArray        GetAssetsByType( AssetType type ) const;
        DZ_API [[nodiscard]] bool                 IsCompressed( ) const;
        DZ_API [[nodiscard]] const InteropString &GetPath( ) const;

        DZ_API static Bundle *CreateFromDirectory( const BundleDirectoryDesc &directoryDesc );
    };

    struct DZ_API BundleArray
    {
        Bundle **Elements;
        uint32_t NumElements;
    };
} // namespace DenOfIz
