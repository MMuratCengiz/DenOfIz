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
#include <DenOfIzGraphics/Assets/Serde/Shader/ShaderAsset.h>

namespace DenOfIz
{
    struct DZ_API ShaderImportDesc : ImportDesc
    {
        InteropString               EntryPoint = "main";
        ShaderStage                 Stage      = ShaderStage::Pixel;
        InteropArray<InteropString> Defines;

        static ShaderImportDesc CreateFromBase( const ImportDesc &base )
        {
            return ShaderImportDesc( base );
        }
    };

    class ShaderImporter final : public IAssetImporter
    {
        ImporterDesc m_importerDesc;

        struct ImportContext
        {
            ImportJobDesc    JobDesc;
            ShaderAsset      ShaderAsset;
            InteropString    ErrorMessage;
            ImporterResult   Result;
            ShaderImportDesc Options;
        };

    public:
        DZ_API ShaderImporter( );
        DZ_API ~ShaderImporter( ) override;

        DZ_API ImporterDesc   GetImporterInfo( ) const override;
        DZ_API bool           CanProcessFileExtension( const InteropString &extension ) const override;
        DZ_API ImporterResult Import( const ImportJobDesc &desc ) override;
        DZ_API bool           ValidateFile( const InteropString &filePath ) const override;

    private:
        static void          WriteShaderAsset( const ImportContext &context, AssetUri &outAssetUri );
        static InteropString GetAssetNameFromFilePath( const InteropString &filePath );
        static InteropString SanitizeAssetName( const InteropString &name );
        static InteropString CreateAssetFileName( const InteropString &prefix, const InteropString &name );
        static std::string   GetFileExtension( const char *filePath );
        static ShaderStage   InferShaderStageFromExtension( const std::string &fileExtension );
    };
} // namespace DenOfIz
