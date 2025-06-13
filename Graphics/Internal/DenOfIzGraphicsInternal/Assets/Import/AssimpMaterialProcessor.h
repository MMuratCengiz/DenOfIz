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

#include <assimp/material.h>
#include <assimp/scene.h>
#include "AssimpImportContext.h"
#include "DenOfIzGraphics/Assets/Serde/Material/MaterialAsset.h"
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAsset.h"
#include "DenOfIzGraphics/Data/Texture.h"

namespace DenOfIz
{
    class AssimpMaterialProcessor
    {
    public:
        AssimpMaterialProcessor( );
        ~AssimpMaterialProcessor( );

        ImporterResultCode ProcessAllMaterials( AssimpImportContext &context ) const;
        ImporterResultCode ProcessMaterial( AssimpImportContext &context, const aiMaterial *material, AssetUri &outMaterialUri ) const;
        ImporterResultCode ProcessMaterialTextures( AssimpImportContext &context, const aiMaterial *material, MaterialAsset &materialAsset ) const;

    private:
        bool ProcessTexture( AssimpImportContext &context, const aiMaterial *material, aiTextureType textureType, const InteropString &semanticName, AssetUri &outAssetUri ) const;
        ImporterResultCode WriteTextureAsset( AssimpImportContext &context, const aiTexture *texture, const std::string &path, const InteropString &semanticName,
                                              AssetUri &outAssetUri ) const;
        ImporterResultCode WriteMaterialAsset( AssimpImportContext &context, MaterialAsset &materialAsset, AssetUri &outAssetUri ) const;
        void               ExtractMaterialProperties( const aiMaterial *material, MaterialAsset &materialAsset ) const;

        [[nodiscard]] Float_4 ConvertColor( const aiColor4D &color ) const;
        [[nodiscard]] Float_3 ConvertColor3( const aiColor3D &color ) const;

        TextureExtension IdentifyTextureFormat( const aiTexture *texture, const InteropArray<Byte> &data ) const;
    };
} // namespace DenOfIz
