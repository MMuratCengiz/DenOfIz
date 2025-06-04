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

#include "DenOfIzGraphics/Assets/Import/IAssetImporter.h"
#include "DenOfIzGraphics/Utilities/Interop.h"

namespace DenOfIz
{
    class DZ_API ModifyAssetCallback
    {
    public:
        virtual ~ModifyAssetCallback( ) = default;
        virtual InteropString ModifyPath( const InteropString &oldPath )
        {
            return oldPath;
        }
    };

    class DZ_API FilterAssetCallback
    {
    public:
        virtual ~FilterAssetCallback( ) = default;
        virtual bool ShouldProcessAsset( const InteropString &path )
        {
            return true;
        }
    };

    struct DZ_API AddImporterDesc
    {
        InteropString   ImporterName;
        IAssetImporter *Importer;
        ImportDesc     *ImportDesc;
        InteropString   TargetDirectory;
        // This can be overridden via ModifyAssetCallback
        InteropString AssetPrefix;
    };

    class AssetScanner
    {
        InteropArray<IAssetImporter *>      m_importers;
        InteropArray<ImportDesc *>          m_importDescs; // Size matches with m_importers
        InteropArray<ModifyAssetCallback *> m_modifyAssetCallbacks;
        InteropArray<FilterAssetCallback *> m_filterAssetCallbacks;

    public:
        DZ_API AssetScanner( )  = default;
        DZ_API ~AssetScanner( ) = default;

        /// Desc must match the type IAssetImporter implementation expects
        DZ_API void AddImporter( IAssetImporter *importer, ImportDesc *desc );
        DZ_API void RegisterModifyAssetCallback( ModifyAssetCallback *callback );
        DZ_API void RegisterFilterAssetCallback( FilterAssetCallback *callback );
        DZ_API void Scan( const InteropString &directoryToScan, const InteropString &targetDirectory );
    };
} // namespace DenOfIz
