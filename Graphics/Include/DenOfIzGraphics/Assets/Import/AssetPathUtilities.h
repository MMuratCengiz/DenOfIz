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

#include "DenOfIzGraphics/Utilities/Common.h"
#include <algorithm>
#include <cctype>
#include <filesystem>
#include <string>

namespace DenOfIz
{
    class DZ_API AssetPathUtilities
    {
    public:
        static InteropString SanitizeAssetName( const InteropString &name, bool ensureValidStart = false, bool trimSpecialChars = true );
        static InteropString GetAssetNameFromFilePath( const InteropString &filePath );
        static InteropString CreateAssetFileName( const InteropString &prefix, const InteropString &name, const InteropString &extension );
        static InteropString CreateAssetFileName( const InteropString &prefix, const InteropString &name, const InteropString &assetType, const InteropString &extension );
        static InteropString GetFileExtension( const InteropString &filePath );
        static InteropString GetFileNameWithoutExtension( const InteropString &filePath );

    private:
        AssetPathUtilities( ) = default;
    };
} // namespace DenOfIz
