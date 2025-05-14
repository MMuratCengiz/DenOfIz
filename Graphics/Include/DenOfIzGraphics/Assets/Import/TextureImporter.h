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
#include <DenOfIzGraphics/Assets/FileSystem/FileIO.h>
#include <DenOfIzGraphics/Assets/Import/IAssetImporter.h>
#include <DenOfIzGraphics/Assets/Serde/Texture/TextureAsset.h>
#include <DenOfIzGraphics/Assets/Serde/Texture/TextureAssetWriter.h>
#include <DenOfIzGraphics/Data/Texture.h>
#include <DenOfIzGraphics/Utilities/Utilities.h>

namespace DenOfIz
{
    struct DZ_API TextureImportDesc : ImportDesc
    {
        // Todo not yet implemented
        bool GenerateMips        = true;
        bool NormalizeNormalMaps = true;
        bool FlipY               = false;

        TextureImportDesc( ) = default;
        explicit TextureImportDesc( const ImportDesc &base ) : ImportDesc( base )
        {
        }
    };

    struct DZ_API TextureImporterDesc{ };

    class TextureImporter final : public IAssetImporter
    {
        ImporterDesc              m_importerInfo;
        const TextureImporterDesc m_desc;
        std::unique_ptr<Texture>  m_texture = nullptr;

        struct ImportContext
        {
            InteropString     SourceFilePath;
            InteropString     TargetDirectory;
            InteropString     AssetNamePrefix;
            TextureImportDesc Desc;
            ImporterResult    Result;
            InteropString     ErrorMessage;
            TextureAsset      TextureAsset{ };
        };

    public:
        DZ_API explicit TextureImporter( TextureImporterDesc desc );
        DZ_API ~TextureImporter( ) override;

        DZ_API [[nodiscard]] ImporterDesc GetImporterInfo( ) const override;
        DZ_API [[nodiscard]] bool         CanProcessFileExtension( const InteropString &extension ) const override;
        DZ_API ImporterResult             Import( const ImportJobDesc &desc ) override;
        DZ_API [[nodiscard]] bool         ValidateFile( const InteropString &filePath ) const override;

    private:
        ImporterResultCode ImportTextureInternal( ImportContext &context );
        void               WriteTextureAsset( const ImportContext &context, const TextureAsset &textureAsset, AssetUri &outAssetUri ) const;

        static void RegisterCreatedAsset( ImportContext &context, const AssetUri &assetUri );
    };
} // namespace DenOfIz
