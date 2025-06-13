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

#include "DenOfIzGraphics/Assets/Bundle/BundleManager.h"
#include "DenOfIzGraphics/Assets/FileSystem/FileIO.h"
#include "DenOfIzGraphics/Assets/Import/ImporterCommon.h"
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAsset.h"
#include "DenOfIzGraphics/Data/Texture.h"

namespace DenOfIz
{
    struct DZ_API TextureImportDesc
    {
        InteropString SourceFilePath;
        InteropString TargetDirectory;
        InteropString AssetNamePrefix;

        bool GenerateMips        = true;
        bool NormalizeNormalMaps = true;
        bool FlipY               = false;
    };

    class TextureImporter
    {
    public:
        DZ_API TextureImporter( );
        DZ_API ~TextureImporter( );

        DZ_API [[nodiscard]] InteropString      GetName( ) const;
        DZ_API [[nodiscard]] InteropStringArray GetSupportedExtensions( ) const;
        DZ_API [[nodiscard]] bool               CanProcessFileExtension( const InteropString &extension ) const;
        DZ_API ImporterResult                   Import( const TextureImportDesc &desc );
        DZ_API [[nodiscard]] bool               ValidateFile( const InteropString &filePath ) const;

    private:
        InteropString            m_name;
        InteropStringArray       m_supportedExtensions;
        std::unique_ptr<Texture> m_texture = nullptr;
        std::vector<AssetUri>    m_createdAssets;

        struct ImportContext
        {
            TextureImportDesc Desc;
            ImporterResult    Result;
            InteropString     ErrorMessage;
            TextureAsset      TextureAsset{ };
        };

        ImporterResultCode ImportTextureInternal( ImportContext &context );
        void               WriteTextureAsset( const ImportContext &context, const TextureAsset &textureAsset, AssetUri &outAssetUri ) const;
        void               RegisterCreatedAsset( ImportContext &context, const AssetUri &assetUri );
    };
} // namespace DenOfIz
