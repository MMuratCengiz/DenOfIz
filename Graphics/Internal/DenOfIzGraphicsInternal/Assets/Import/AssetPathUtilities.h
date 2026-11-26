/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
