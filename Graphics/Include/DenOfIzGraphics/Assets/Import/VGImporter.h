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

#include <memory>
#include "DenOfIzGraphics/Assets/Import/ImporterCommon.h"
#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAsset.h"
#include "DenOfIzGraphics/Assets/Vector2d/ThorVGWrapper.h"
#include "DenOfIzGraphics/Utilities/DZArena.h"

namespace DenOfIz
{
    struct DZ_API VGImportDesc
    {
        InteropString SourceFilePath;
        InteropString TargetDirectory;
        InteropString AssetNamePrefix;

        uint32_t      RenderWidth  = 1024;
        uint32_t      RenderHeight = 1024;
        float         Scale        = 1.0f;
        uint32_t      Padding      = 8;
        bool          Premultiply  = true;
        Format        OutputFormat = Format::R8G8B8A8Unorm;
        ThorVGCanvas *Canvas       = nullptr;
    };

    class DZ_API VGImporter
    {
    public:
        VGImporter( );
        ~VGImporter( );

        [[nodiscard]] InteropString      GetName( ) const;
        [[nodiscard]] InteropStringArray GetSupportedExtensions( ) const;
        [[nodiscard]] bool               CanProcessFileExtension( const InteropString &extension ) const;
        ImporterResult                   Import( const VGImportDesc &desc ) const;
        [[nodiscard]] bool               ValidateFile( const InteropString &filePath ) const;

    private:
        class Impl;
        std::unique_ptr<Impl> m_pImpl;
    };

} // namespace DenOfIz
