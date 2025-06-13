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
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAsset.h"
#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"
#include "DenOfIzGraphics/Utilities/DZArena.h"

namespace DenOfIz
{
    
    struct DZ_API ShaderImportDesc
    {
        InteropString     TargetDirectory;
        ShaderProgramDesc ProgramDesc;
        InteropString     OutputShaderName; // Note this is just the file name
    };

    class ShaderImporter
    {
    public:
        DZ_API ShaderImporter();
        DZ_API ~ShaderImporter();

        DZ_API [[nodiscard]] InteropString GetName() const;
        DZ_API [[nodiscard]] InteropStringArray GetSupportedExtensions() const;
        DZ_API [[nodiscard]] bool CanProcessFileExtension(const InteropString &extension) const;
        DZ_API ImporterResult Import(const ShaderImportDesc &desc ) const;
        DZ_API [[nodiscard]] bool ValidateFile(const InteropString &filePath) const;

    private:
        class Impl;
        std::unique_ptr<Impl> m_pImpl;
    };
} // namespace DenOfIz