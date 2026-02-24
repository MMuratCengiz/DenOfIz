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

#include <string>
#include "DenOfIzGraphics/Utilities/Common.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

namespace DenOfIz
{
    class AssetPathUtilities
    {
    public:
        static std::string SanitizeAssetName( const std::string &name, bool ensureValidStart = false, bool trimSpecialChars = true );
        static std::string GetAssetNameFromFilePath( const DenOfIz_StringView &filePath );
        static std::string CreateAssetFileName( const std::string &prefix, const std::string &name, const std::string &extension );
        static std::string CreateAssetFileName( const std::string &prefix, const std::string &name, const std::string &assetType, const std::string &extension );
        static std::string GetFileExtension( const std::string &filePath );
        static std::string GetFileNameWithoutExtension( const std::string &filePath );

    private:
        AssetPathUtilities( ) = default;
    };
} // namespace DenOfIz
