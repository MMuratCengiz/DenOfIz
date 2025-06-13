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

#include <assimp/matrix4x4.h>
#include <assimp/scene.h>
#include <string>
#include <unordered_map>
#include <vector>
#include "DenOfIzGraphics/Assets/Import/AssimpImporter.h"
#include "DenOfIzGraphics/Assets/Import/ImporterCommon.h"
#include "DenOfIzGraphics/Assets/Serde/Asset.h"
#include "DenOfIzGraphics/Assets/Serde/Mesh/MeshAsset.h"
#include "DenOfIzGraphics/Utilities/DZArena.h"

namespace DenOfIz
{
    struct AssimpImportContext
    {
        const aiScene   *Scene = nullptr;
        InteropString    SourceFilePath;
        InteropString    TargetDirectory;
        InteropString    AssetNamePrefix;
        AssimpImportDesc Desc;

        ImporterResult Result;
        MeshAsset      MeshAsset;

        std::unordered_map<std::string, AssetUri>       MaterialNameToAssetUriMap;
        std::unordered_map<std::string, AssetUri>       TexturePathToAssetUriMap;
        std::unordered_map<std::string, uint32_t>       BoneNameToIndexMap;
        std::unordered_map<std::string, aiMatrix4x4>    BoneNameToInverseBindMatrixMap;
        std::unordered_map<uint32_t, const aiNode *>    IndexToAssimpNodeMap;
        std::unordered_map<const aiNode *, aiMatrix4x4> NodeWorldTransformCache;

        std::vector<AssetUri> CreatedAssets;
        AssetUri              SkeletonAssetUri;

        DZArena *MainArena           = nullptr;
        DZArena *TempArena           = nullptr; // For temporary allocations
        uint32_t CurrentSubMeshIndex = 0;
    };

} // namespace DenOfIz
