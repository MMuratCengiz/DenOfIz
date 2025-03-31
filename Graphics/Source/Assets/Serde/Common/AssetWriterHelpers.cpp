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

#include <DenOfIzGraphics/Assets/Serde/Common/AssetWriterHelpers.h>

using namespace DenOfIz;

void AssetWriterHelpers::WriteAssetDataStream( const BinaryWriter *writer, const AssetDataStream &stream )
{
    writer->WriteUInt64( stream.Offset );
    writer->WriteUInt64( stream.NumBytes );
}

void AssetWriterHelpers::WriteProperties( const BinaryWriter *writer, const InteropArray<UserProperty> &properties )
{
    writer->WriteUInt32( properties.NumElements( ) );
    for ( size_t i = 0; i < properties.NumElements( ); ++i )
    {
        const UserProperty &prop = properties.GetElement( i );
        WriteUserProperty( writer, prop );
    }
}

void AssetWriterHelpers::WriteUserProperty( const BinaryWriter *writer, const UserProperty &property )
{
    writer->WriteUInt32( static_cast<uint32_t>( property.PropertyType ) );
    writer->WriteString( property.Name );
    switch ( property.PropertyType )
    {
    case UserProperty::Type::String:
        writer->WriteString( property.StringValue );
        break;
    case UserProperty::Type::Int:
        writer->WriteInt32( property.IntValue );
        break;
    case UserProperty::Type::Float:
        writer->WriteFloat( property.FloatValue );
        break;
    case UserProperty::Type::Bool:
        writer->WriteByte( property.BoolValue ? 1 : 0 );
        break;
    case UserProperty::Type::Float2:
        writer->WriteFloat_2( property.Vector2Value );
        break;
    case UserProperty::Type::Float3:
        writer->WriteFloat_3( property.Vector3Value );
        break;
    case UserProperty::Type::Float4:
    case UserProperty::Type::Color:
        writer->WriteFloat_4( property.ColorValue );
        break;
    case UserProperty::Type::Float4x4:
        writer->WriteFloat_4x4( property.TransformValue );
        break;
    }
}
