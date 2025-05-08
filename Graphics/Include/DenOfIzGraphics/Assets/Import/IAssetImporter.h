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

#include <DenOfIzGraphics/Assets/Serde/Asset.h>
#include <DenOfIzGraphics/Utilities/Interop.h>

namespace DenOfIz
{
    enum class ImporterResultCode
    {
        Success,
        FileNotFound,
        UnsupportedFormat,
        ImportFailed,
        WriteFailed,
        InvalidParameters,
        ResourceUnavailable
    };

    struct DZ_API ImporterResult
    {
        ImporterResultCode     ResultCode = ImporterResultCode::Success;
        InteropString          ErrorMessage;
        InteropArray<AssetUri> CreatedAssets;
    };

    struct DZ_API ImportDesc
    {
        bool     OverwriteExisting     = true;
        bool     GenerateLODs          = true;
        uint32_t MaxLODCount           = 3;
        Float_3  LODScreenPercentages  = { 1.0f, 0.5f, 0.25f };
        bool     OptimizeMeshes        = true;
        float    ScaleFactor           = 1.0f;
        bool     ImportMaterials       = true;
        bool     ImportTextures        = true;
        bool     ImportAnimations      = true;
        bool     ImportSkeletons       = true;
        bool     CalculateTangentSpace = true;
        bool     ConvertToLeftHanded   = true; // DenOfIz uses a left handed coordinate system, DirectX12 settings

        InteropArray<InteropString> AdditionalOptions;
    };

    struct DZ_API ImporterDesc
    {
        InteropString               Name;
        InteropArray<InteropString> SupportedExtensions;
    };

    struct DZ_API ImportJobDesc
    {
        InteropString SourceFilePath;
        InteropString TargetDirectory;
        InteropString AssetNamePrefix;
        ImportDesc   *Desc;
    };

    class DZ_API IAssetImporter
    {
    public:
        virtual ~IAssetImporter( ) = default;

        /**
         * @brief Get the name and supported extensions for this importer
         * @return ImporterDesc containing name and supported file extensions
         */
        virtual ImporterDesc GetImporterInfo( ) const = 0;

        /**
         * @brief Check if this importer can process files with the given extension
         * @param extension File extension to check (without the dot)
         * @return true if supported, false otherwise
         */
        virtual bool CanProcessFileExtension( const InteropString &extension ) const = 0;

        /**
         * @brief Import a file and convert it to engine assets
         * @param desc Parameters for the import job
         * @return ImporterResult containing success/failure and created assets
         */
        virtual ImporterResult Import( const ImportJobDesc &desc ) = 0;

        /**
         * @brief Check if a file is valid for this importer before attempting import
         * @param filePath Path to the file to validate
         * @return true if file appears valid, false otherwise
         */
        virtual bool ValidateFile( const InteropString &filePath ) const = 0;
    };

} // namespace DenOfIz
