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

#include "DenOfIzGraphicsInternal/Assets/Serde/Common/AssetReaderHelpers.h"
#include "DenOfIzGraphicsInternal/Utilities/DZArenaHelper.h"

using namespace DenOfIz;

AssetDataStream AssetReaderHelpers::ReadAssetDataStream( BinaryReader *reader )
{
    AssetDataStream stream;
    stream.Offset   = reader->ReadUInt64( );
    stream.NumBytes = reader->ReadUInt64( );
    return stream;
}

UserPropertyArray AssetReaderHelpers::ReadUserProperties( DZArena *arena, BinaryReader *reader )
{
    const uint32_t    numProperties = reader->ReadUInt32( );
    UserPropertyArray properties{ };
    DZArenaArrayHelper<UserPropertyArray, UserProperty>::AllocateAndConstructArray( *arena, properties, numProperties );
    for ( uint32_t i = 0; i < numProperties; ++i )
    {
        properties.Elements[ i ] = ReadUserProperty( reader );
    }
    return properties;
}

UserProperty AssetReaderHelpers::ReadUserProperty( BinaryReader *reader )
{
    UserProperty property{ };
    property.PropertyType = static_cast<UserProperty::Type>( reader->ReadUInt32( ) );
    property.Name         = reader->ReadString( );
    switch ( property.PropertyType )
    {
    case UserProperty::Type::String:
        property.StringValue = reader->ReadString( );
        break;
    case UserProperty::Type::Int:
        property.IntValue = reader->ReadInt32( );
        break;
    case UserProperty::Type::Float:
        property.FloatValue = reader->ReadFloat( );
        break;
    case UserProperty::Type::Bool:
        property.BoolValue = reader->ReadByte( ) != 0;
        break;
    case UserProperty::Type::Float2:
        property.Vector2Value = reader->ReadFloat_2( );
        break;
    case UserProperty::Type::Float3:
        property.Vector3Value = reader->ReadFloat_3( );
        break;
    case UserProperty::Type::Float4:
        property.Vector4Value = reader->ReadFloat_4( );
        break;
    case UserProperty::Type::Color:
        property.ColorValue = reader->ReadFloat_4( );
        break;
    case UserProperty::Type::Float4x4:
        property.TransformValue = reader->ReadFloat_4x4( );
        break;
    }

    return property;
}
