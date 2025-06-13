/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program. If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <memory>
#include "DenOfIzGraphics/Assets/Import/ImporterCommon.h"
#include "DenOfIzGraphics/Utilities/DZArena.h"

namespace DenOfIz
{

    struct DZ_API AssimpImportDesc
    {
        InteropString SourceFilePath;
        InteropString TargetDirectory;
        InteropString AssetNamePrefix;

        bool ImportMaterials  = true;
        bool ImportTextures   = true;
        bool ImportAnimations = true;
        bool ImportSkeletons  = true;

        bool     OverwriteExisting        = true;
        bool     GenerateLODs             = true;
        uint32_t MaxLODCount              = 3;
        Float_3  LODScreenPercentages     = { 1.0f, 0.5f, 0.25f };
        bool     OptimizeMeshes           = true;
        float    ScaleFactor              = 1.0f;
        bool     JoinIdenticalVertices    = true;
        bool     PreTransformVertices     = false;
        bool     LimitBoneWeights         = true;
        uint32_t MaxBoneWeightsPerVertex  = 4;
        bool     RemoveRedundantMaterials = true;
        bool     MergeMeshes              = false;
        bool     OptimizeGraph            = true;
        bool     GenerateNormals          = true;
        bool     SmoothNormals            = true;
        float    SmoothNormalsAngle       = 80.0f;
        bool     TriangulateMeshes        = true;
        bool     PreservePivots           = true;
        bool     DropNormals              = false;
        bool     ConvertToLeftHanded      = true; // DenOfIz uses a left handed coordinate system, DirectX12 settings
        bool     CalculateTangentSpace    = true;

        InteropStringArray AdditionalOptions;
    };

    class AssimpImporter
    {
    public:
        DZ_API AssimpImporter( );
        DZ_API ~AssimpImporter( );

        DZ_API [[nodiscard]] InteropString      GetName( ) const;
        DZ_API [[nodiscard]] InteropStringArray GetSupportedExtensions( ) const;
        DZ_API [[nodiscard]] bool               CanProcessFileExtension( const InteropString &extension ) const;
        DZ_API [[nodiscard]] ImporterResult     Import( const AssimpImportDesc &desc ) const;
        DZ_API [[nodiscard]] bool               ValidateFile( const InteropString &filePath ) const;

    private:
        class Impl;
        std::unique_ptr<Impl> m_pImpl;
    };

} // namespace DenOfIz
