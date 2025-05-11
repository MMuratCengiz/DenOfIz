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

#include <DenOfIzGraphics/Assets/Bundle/BundleManager.h>
#include <DenOfIzGraphics/Assets/Import/IAssetImporter.h>
#include <DenOfIzGraphics/Assets/Serde/Font/FontAsset.h>
#include <DenOfIzGraphics/Assets/Serde/Font/FontAssetWriter.h>

#include <freetype/freetype.h>

namespace msdfgen
{
    class FreetypeHandle;
    class FontHandle;
}

namespace DenOfIz
{
    struct DZ_API FontImportDesc : ImportDesc
    {
        uint32_t InitialFontSize = 36;
        uint32_t AtlasWidth      = 512;
        uint32_t AtlasHeight     = 512;

        FontImportDesc( ) = default;
        explicit FontImportDesc( const ImportDesc &base ) : ImportDesc( base )
        {
        }
    };

    struct DZ_API FontImporterDesc{ };

    class FontImporter final : public IAssetImporter
    {
        ImporterDesc             m_importerDesc;
        const FontImporterDesc   m_desc;
        FT_Library               m_ftLibrary;
        msdfgen::FreetypeHandle *m_msdfFtHandle = nullptr;

        struct Rect
        {
            uint32_t X;
            uint32_t Y;
            uint32_t Width;
            uint32_t Height;
        };

        struct ImportContext
        {
            InteropString  SourceFilePath;
            InteropString  TargetDirectory;
            InteropString  AssetNamePrefix;
            FontImportDesc Desc;
            ImporterResult Result;
            InteropString  ErrorMessage;

            FontAsset FontAsset;

            uint32_t CurrentAtlasX = 0;
            uint32_t CurrentAtlasY = 0;
            uint32_t RowHeight     = 0;

            msdfgen::FontHandle *MsdfFont = nullptr;
        };

    public:
        DZ_API explicit FontImporter( FontImporterDesc desc );
        DZ_API ~FontImporter( ) override;

        DZ_API [[nodiscard]] ImporterDesc GetImporterInfo( ) const override;
        DZ_API [[nodiscard]] bool         CanProcessFileExtension( const InteropString &extension ) const override;
        DZ_API ImporterResult             Import( const ImportJobDesc &desc ) override;
        DZ_API [[nodiscard]] bool         ValidateFile( const InteropString &filePath ) const override;

    private:
        ImporterResultCode          ImportFontInternal( ImportContext &context );
        void                        GenerateAtlas( ImportContext &context ) const;
        void                        WriteFontAsset( const ImportContext &context, AssetUri &outAssetUri ) const;
        void                        ExtractFontMetrics( ImportContext &context, FT_Face face );
        [[nodiscard]] InteropString CreateAssetFileName( const InteropString &prefix, const InteropString &name ) const;
        static InteropString        GetAssetNameFromFilePath( const InteropString &filePath );
        static InteropString        SanitizeAssetName( const InteropString &name );
        static Byte                 FloatToByte( const float &f );
    };
} // namespace DenOfIz
