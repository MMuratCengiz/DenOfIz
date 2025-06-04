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

#include "DenOfIzGraphics/Assets/Import/AssetPathUtilities.h"

using namespace DenOfIz;

InteropString AssetPathUtilities::SanitizeAssetName( const InteropString &name, bool ensureValidStart, bool trimSpecialChars )
{
    const std::string &allowedSpecialChars = "_-.";
    std::string        sanitized           = name.Get( );

    std::ranges::replace_if( sanitized, [ &allowedSpecialChars ]( const char c ) { return !std::isalnum( c ) && allowedSpecialChars.find( c ) == std::string::npos; }, '_' );
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

InteropString AssetPathUtilities::GetAssetNameFromFilePath( const InteropString &filePath )
{
    const std::filesystem::path p = filePath.Get( );
    return p.stem( ).string( ).c_str( );
}

InteropString AssetPathUtilities::CreateAssetFileName( const InteropString &prefix, const InteropString &name, const InteropString &extension )
{
    InteropString fileName = prefix;

    if ( fileName.NumChars( ) > 0 )
    {
        fileName = fileName.Append( "_" );
    }

    fileName = fileName.Append( name.Get( ) ).Append( "." ).Append( extension.Get( ) );
    return fileName;
}

InteropString AssetPathUtilities::CreateAssetFileName( const InteropString &prefix, const InteropString &name, const InteropString &assetType, const InteropString &extension )
{
    auto fn = InteropString( prefix );
    if ( !prefix.IsEmpty( ) && !name.IsEmpty( ) )
    {
        fn = fn.Append( "_" );
    }
    fn = fn.Append( name.Get( ) );
    if ( !assetType.IsEmpty( ) )
    {
        fn = fn.Append( "_" ).Append( assetType.Get( ) );
    }
    fn = fn.Append( "." ).Append( extension.Get( ) );
    return fn;
}

InteropString AssetPathUtilities::GetFileExtension( const InteropString &filePath )
{
    const std::filesystem::path p = filePath.Get( );
    return InteropString( p.extension( ).string( ).c_str( ) ).ToLower( );
}

InteropString AssetPathUtilities::GetFileNameWithoutExtension( const InteropString &filePath )
{
    const std::filesystem::path p = filePath.Get( );
    return p.stem( ).string( ).c_str( );
}
