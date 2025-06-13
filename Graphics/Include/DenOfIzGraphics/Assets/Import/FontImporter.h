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

#include <memory>
#include "DenOfIzGraphics/Assets/Import/ImporterCommon.h"
#include "DenOfIzGraphics/Assets/Serde/Font/FontAssetWriter.h"

namespace DenOfIz
{
    struct FontImporterImpl;

    struct DZ_API FontImportDesc
    {
        InteropString SourceFilePath;
        InteropString TargetDirectory;
        InteropString AssetNamePrefix;

        uint32_t         InitialFontSize = 36;
        uint32_t         AtlasWidth      = 512;
        uint32_t         AtlasHeight     = 512;
        BinaryContainer *TargetContainer = nullptr; // Not required, only useful to load font into memory
    };

    class DZ_API FontImporter
    {
    public:
        FontImporter( );
        ~FontImporter( );

        [[nodiscard]] InteropString      GetName( ) const;
        [[nodiscard]] InteropStringArray GetSupportedExtensions( ) const;
        [[nodiscard]] bool               CanProcessFileExtension( const InteropString &extension ) const;
        ImporterResult                   Import( const FontImportDesc &desc ) const;
        [[nodiscard]] bool               ValidateFile( const InteropString &filePath ) const;

    private:
        InteropString                     m_name;
        InteropStringArray                m_supportedExtensions;
        std::unique_ptr<FontImporterImpl> m_impl;
    };
} // namespace DenOfIz
