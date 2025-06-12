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
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAsset.h"
#include "DenOfIzGraphics/Assets/Vector2d/ThorVGWrapper.h"

namespace DenOfIz
{
    struct DZ_API VGImportDesc : ImportDesc
    {
        uint32_t      RenderWidth  = 1024;
        uint32_t      RenderHeight = 1024;
        float         Scale        = 1.0f;
        uint32_t      Padding      = 8;
        bool          Premultiply  = true;
        Format        OutputFormat = Format::R8G8B8A8Unorm;
        ThorVGCanvas *Canvas       = nullptr;
    };

    struct DZ_API VGImporterDesc
    {
    };

    class DZ_API VGImporter final : public IAssetImporter
    {
        VGImporterDesc          m_desc;
        ImporterDesc            m_importerInfo;
        InteropArray<uint32_t>  m_renderBuffer;
        std::vector<AssetUri>   m_createdAssets;
        std::vector<TextureMip> m_mips;

    public:
        explicit VGImporter( VGImporterDesc desc = { } );
        ~VGImporter( ) override;

        [[nodiscard]] ImporterDesc GetImporterInfo( ) const override;
        [[nodiscard]] bool         CanProcessFileExtension( const InteropString &extension ) const override;
        ImporterResult             Import( const ImportJobDesc &desc ) override;
        [[nodiscard]] bool         ValidateFile( const InteropString &filePath ) const override;

    private:
        struct ImportContext
        {
            InteropString  SourceFilePath;
            InteropString  TargetDirectory;
            InteropString  AssetNamePrefix;
            VGImportDesc   Desc;
            ImporterResult Result;
            InteropString  ErrorMessage;
            TextureAsset   TextureAsset;
        };

        ImporterResultCode ImportVGInternal( ImportContext &context );
        void               WriteTextureAsset( const ImportContext &context, const TextureAsset &textureAsset, AssetUri &outAssetUri ) const;
        void               RegisterCreatedAsset( ImportContext &context, const AssetUri &assetUri );
    };

} // namespace DenOfIz
