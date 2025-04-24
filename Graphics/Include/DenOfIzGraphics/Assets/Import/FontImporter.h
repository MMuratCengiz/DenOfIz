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
#include <msdfgen-ext.h>
#include <msdfgen.h>

namespace DenOfIz
{
    struct DZ_API FontImportDesc : ImportDesc
    {
        uint32_t PixelSize    = 24;
        bool     AntiAliasing = true;
        uint32_t AtlasWidth   = 512;
        uint32_t AtlasHeight  = 512;

        FontImportDesc( ) = default;
        explicit FontImportDesc( const ImportDesc &base ) : ImportDesc( base )
        {
        }
        static FontImportDesc CreateFromBase( const ImportDesc &base )
        {
            return FontImportDesc( base );
        }
    };

    struct DZ_API FontImporterDesc{ };

    class DZ_API FontImporter final : public IAssetImporter
    {
        const InteropString m_fileExtension = "dzfont";

        ImporterDesc             m_importerDesc;
        const FontImporterDesc   m_desc;
        FT_Library               m_ftLibrary;
        msdfgen::FreetypeHandle *m_msdfFtHandle   = nullptr;
        float                    m_msdfPixelRange = 4.0f;

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
            FontImportDesc Options;
            ImporterResult Result;
            InteropString  ErrorMessage;

            FontAsset          FontAsset;
            InteropArray<Byte> AtlasBitmap;

            uint32_t CurrentAtlasX = 0;
            uint32_t CurrentAtlasY = 0;
            uint32_t RowHeight     = 0;
        };

    public:
        explicit FontImporter( FontImporterDesc desc );
        ~FontImporter( ) override;

        [[nodiscard]] ImporterDesc GetImporterInfo( ) const override;
        [[nodiscard]] bool         CanProcessFileExtension( const InteropString &extension ) const override;
        ImporterResult             Import( const ImportJobDesc &desc ) override;
        [[nodiscard]] bool         ValidateFile( const InteropString &filePath ) const override;

    private:
        ImporterResultCode   ImportFontInternal( ImportContext &context );
        void                 PreloadCharacterSet( ImportContext &context, FT_Face face );
        bool                 LoadGlyph( ImportContext &context, FT_Face face, uint32_t codePoint );
        bool                 GenerateMsdfForGlyph( FontGlyph &glyphDesc, msdfgen::FontHandle *msdfFont, uint32_t codePoint, uint32_t pixelSize ) const;
        Rect                 AllocateSpace( ImportContext &context, uint32_t width, uint32_t height );
        void                 CopyMsdfDataToAtlas( ImportContext &context, const FontGlyph &glyphDesc, const Rect &rect );
        void                 WriteFontAsset( const ImportContext &context, AssetUri &outAssetUri );
        void                 ExtractFontMetrics( ImportContext &context, FT_Face face );
        InteropString        CreateAssetFileName( const InteropString &prefix, const InteropString &name );
        static InteropString GetAssetNameFromFilePath( const InteropString &filePath );
        static InteropString SanitizeAssetName( const InteropString &name );
    };
} // namespace DenOfIz
