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

#include "DenOfIzGraphicsInternal/Assets/Import/AssetPathUtilities.h"

#include <algorithm>
#include <filesystem>

#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

using namespace DenOfIz;

std::string AssetPathUtilities::SanitizeAssetName( const std::string &name, const bool ensureValidStart, const bool trimSpecialChars )
{
    const std::string &allowedSpecialChars = "_-.";
    std::string        sanitized           = name;

    std::replace_if(
        sanitized.begin( ), sanitized.end( ), [ &allowedSpecialChars ]( const char c ) { return !std::isalnum( c ) && allowedSpecialChars.find( c ) == std::string::npos; }, '_' );
    if ( trimSpecialChars && !sanitized.empty( ) )
    {
        const std::string specialChars = allowedSpecialChars + "_";
        sanitized.erase( 0, std::min( sanitized.find_first_not_of( specialChars ), sanitized.size( ) - 1 ) );
        const size_t lastNonSpecial = sanitized.find_last_not_of( specialChars );
        if ( lastNonSpecial != std::string::npos )
        {
            sanitized.erase( lastNonSpecial + 1 );
        }
    }

    if ( ensureValidStart && !sanitized.empty( ) && !std::isalpha( sanitized[ 0 ] ) && sanitized[ 0 ] != '_' )
    {
        sanitized = "_" + sanitized;
    }

    if ( sanitized.empty( ) )
    {
        sanitized = "UnnamedAsset";
    }

    return sanitized.c_str( );
}

std::string AssetPathUtilities::GetAssetNameFromFilePath( const DenOfIz_StringView &filePath )
{
    const std::string           filePathStr( filePath.Chars, filePath.NumChars );
    const std::filesystem::path p = filePathStr;
    return p.stem( ).string( ).c_str( );
}

std::string AssetPathUtilities::CreateAssetFileName( const std::string &prefix, const std::string &name, const std::string &extension )
{
    auto fileName = prefix;

    if ( fileName.length( ) > 0 )
    {
        fileName += "_";
    }

    fileName += name + "." + extension;
    return fileName;
}

std::string AssetPathUtilities::CreateAssetFileName( const std::string &prefix, const std::string &name, const std::string &assetType, const std::string &extension )
{
    auto fn = prefix;
    if ( !prefix.empty( ) && !name.empty( ) )
    {
        fn += "_";
    }
    fn += name;
    if ( !assetType.empty( ) )
    {
        fn += "_" + assetType;
    }
    fn += "." + extension;
    return fn;
}

std::string AssetPathUtilities::GetFileExtension( const std::string &filePath )
{
    const std::filesystem::path p = filePath;
    return p.extension( ).string( );
}

std::string AssetPathUtilities::GetFileNameWithoutExtension( const std::string &filePath )
{
    const std::filesystem::path p = filePath;
    return p.stem( ).string( ).c_str( );
}
