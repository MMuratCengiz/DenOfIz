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

#include "DenOfIzGraphics/Assets/Serde/Asset.h"

using namespace DenOfIz;

AssetUri AssetUri::Parse( const InteropString &uri )
{
    AssetUri result{ };

    const std::string str      = uri.Get( );
    const std::string protocol = result.Scheme.Get( );

    if ( const std::string protocolPrefix = std::string( result.Scheme.Get( ) ) + "://"; str.find( protocolPrefix ) == 0 )
    {
        result.Path = str.substr( protocolPrefix.length( ) ).c_str( );
    }
    else
    {
        result.Path = uri;
    }

    return result;
}

InteropString AssetUri::ToInteropString( ) const
{
    std::string result = Scheme.Get( );
    result += "://";
    result += Path.Get( );
    return { result.c_str( ) };
}

bool AssetUri::Equals( const AssetUri &other ) const
{
    return ToInteropString( ).Equals( other.ToInteropString( ) );
}

AssetUri AssetUri::Create( const InteropString &path )
{
    AssetUri result;
    result.Path = path;
    return result;
}
