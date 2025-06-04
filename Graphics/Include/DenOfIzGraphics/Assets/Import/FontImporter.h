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

#include <DenOfIzGraphics/Assets/Import/IAssetImporter.h>
#include <DenOfIzGraphics/Assets/Serde/Font/FontAssetWriter.h>
#include <memory>

namespace DenOfIz
{
    struct FontImporterImpl;

    struct DZ_API FontImportDesc : ImportDesc
    {
        uint32_t         InitialFontSize = 36;
        uint32_t         AtlasWidth      = 512;
        uint32_t         AtlasHeight     = 512;
        BinaryContainer *TargetContainer = nullptr; // Not required, only useful to load font into memory

        FontImportDesc( ) = default;
        explicit FontImportDesc( const ImportDesc &base ) : ImportDesc( base )
        {
        }
    };

    struct DZ_API FontImporterDesc{ };

    class DZ_API FontImporter final : public IAssetImporter
    {
        ImporterDesc                      m_importerDesc;
        const FontImporterDesc            m_desc;
        std::unique_ptr<FontImporterImpl> m_impl;

    public:
        explicit FontImporter( FontImporterDesc desc );
        ~FontImporter( ) override;

        [[nodiscard]] ImporterDesc GetImporterInfo( ) const override;
        [[nodiscard]] bool         CanProcessFileExtension( const InteropString &extension ) const override;
        ImporterResult             Import( const ImportJobDesc &desc ) override;
        [[nodiscard]] bool         ValidateFile( const InteropString &filePath ) const override;
    };
} // namespace DenOfIz
