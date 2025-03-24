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

#include <DenOfIzGraphics/Assets/Serde/Asset.h>
#include <DenOfIzGraphics/Assets/Stream/BinaryReader.h>
#include <DenOfIzGraphics/Assets/Stream/BinaryWriter.h>
#include <DenOfIzGraphics/Utilities/Interop.h>
#include <fstream>
#include <unordered_map>

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
        Physics
    };

    struct DZ_API AssetEntry
    {
        AssetType     Type;
        uint64_t      Offset;
        uint64_t      NumBytes;
        InteropString Name;
        InteropString Path;
    };

    struct DZ_API BundleHeader : AssetHeader
    {
        static constexpr uint64_t BundleHeaderMagic = 0x445A42554E444C; // "DZBUNDL"
        static constexpr uint32_t Latest            = 1;

        uint32_t NumAssets = 0;
        uint64_t TOCOffset = 0;

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
    };

    class DZ_API Bundle
    {
        BundleDesc                                  m_desc;
        std::unordered_map<std::string, AssetEntry> m_assetEntries;
        std::fstream                               *m_bundleFile;
        bool                                        m_isDirty;

        void LoadTableOfContents( );
        void WriteEmptyHeader( ) const;

    public:
        DZ_API Bundle( const BundleDesc &desc );
        DZ_API ~Bundle( );

        BinaryReader      *OpenReader( const AssetUri &assetUri );
        BinaryWriter      *OpenWriter( const AssetUri &assetUri );
        void               AddAsset( const AssetUri &assetUri, AssetType type, const InteropArray<Byte> &data );
        bool               Save( );
        [[nodiscard]] bool Exists( const AssetUri &assetUri ) const;
    };
} // namespace DenOfIz
