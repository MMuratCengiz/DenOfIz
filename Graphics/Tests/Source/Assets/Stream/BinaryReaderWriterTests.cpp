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
#include "gtest/gtest.h"

#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryReader.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"

using namespace DenOfIz;

TEST( BinarySerdeTest, BasicTypes )
{
    BinaryContainer container;
    {
        const BinaryWriter writer( container );
        writer.WriteUInt16( 12345 );
        writer.WriteInt32( -54321 );
        writer.WriteFloat( 3.14159f );
        writer.WriteUInt64( 9876543210ULL );
        writer.WriteString( "Hello Binary!" );
        writer.WriteByte( 0xAB );
    }

    BinaryReader reader( container );
    ASSERT_EQ( reader.ReadUInt16( ), 12345 );
    ASSERT_EQ( reader.ReadInt32( ), -54321 );
    ASSERT_FLOAT_EQ( reader.ReadFloat( ), 3.14159f );
    ASSERT_EQ( reader.ReadUInt64( ), 9876543210ULL );
    ASSERT_STREQ( reader.ReadString( ).Get( ), "Hello Binary!" );
    ASSERT_EQ( reader.ReadByte( ), 0xAB );
}

TEST( BinarySerdeTest, MathTypes )
{
    BinaryContainer container;
    Float_2         vec2 = { 1.0f, 2.0f };
    Float_3         vec3 = { 3.0f, 4.0f, 5.0f };
    Float_4         vec4 = { 6.0f, 7.0f, 8.0f, 9.0f };
    Float_4x4       mat4;

    for ( int i = 0; i < 4; ++i )
    {
        for ( int j = 0; j < 4; ++j )
        {
            mat4.SetElement( i, j, ( i + 1 ) * ( j + 1 ) );
        }
    }

    {
        BinaryWriter writer( container );
        writer.WriteFloat_2( vec2 );
        writer.WriteFloat_3( vec3 );
        writer.WriteFloat_4( vec4 );
        writer.WriteFloat_4x4( mat4 );
    }

    BinaryReader reader( container );
    Float_2      r_vec2 = reader.ReadFloat_2( );
    Float_3      r_vec3 = reader.ReadFloat_3( );
    Float_4      r_vec4 = reader.ReadFloat_4( );
    Float_4x4    r_mat4 = reader.ReadFloat_4x4( );

    ASSERT_FLOAT_EQ( r_vec2.X, vec2.X );
    ASSERT_FLOAT_EQ( r_vec2.Y, vec2.Y );
    ASSERT_FLOAT_EQ( r_vec3.X, vec3.X );
    ASSERT_FLOAT_EQ( r_vec3.Y, vec3.Y );
    ASSERT_FLOAT_EQ( r_vec3.Z, vec3.Z );
    ASSERT_FLOAT_EQ( r_vec4.X, vec4.X );
    ASSERT_FLOAT_EQ( r_vec4.Y, vec4.Y );
    ASSERT_FLOAT_EQ( r_vec4.Z, vec4.Z );
    ASSERT_FLOAT_EQ( r_vec4.W, vec4.W );
    for ( int i = 0; i < 4; ++i )
    {
        for ( int j = 0; j < 4; ++j )
        {
            ASSERT_FLOAT_EQ( r_mat4.GetElement( i, j ), mat4.GetElement( i, j ) );
        }
    }
}

TEST( BinarySerdeTest, SeekingAndPosition )
{
    BinaryContainer container;
    {
        BinaryWriter writer( container );
        writer.WriteUInt32( 111 );
        ASSERT_EQ( writer.Position( ), 4 );
        writer.WriteUInt32( 222 );
        ASSERT_EQ( writer.Position( ), 8 );
        writer.WriteUInt32( 333 );
        ASSERT_EQ( writer.Position( ), 12 );
        writer.Seek( 4 );
        ASSERT_EQ( writer.Position( ), 4 );
        writer.WriteUInt32( 999 );
        ASSERT_EQ( writer.Position( ), 8 );
    }

    BinaryReader reader( container );
    ASSERT_EQ( reader.ReadUInt32( ), 111 );
    ASSERT_EQ( reader.Position( ), 4 );
    ASSERT_EQ( reader.ReadUInt32( ), 999 );
    ASSERT_EQ( reader.Position( ), 8 );
    ASSERT_EQ( reader.ReadUInt32( ), 333 );
    ASSERT_EQ( reader.Position( ), 12 );
    reader.Seek( 0 );
    ASSERT_EQ( reader.Position( ), 0 );
    ASSERT_EQ( reader.ReadUInt32( ), 111 );
}

TEST( BinarySerdeTest, InteropArrayByteSupport )
{
    BinaryContainer container;
    {
        const BinaryWriter writer( container );
        writer.WriteUInt32( 12345 );
        writer.WriteFloat( 3.14159f );
        writer.WriteString( "TestString" );
        writer.Flush( );
    }

    const ByteArrayView byteData = container.GetData( );

    BinaryReader reader( byteData );
    ASSERT_EQ( reader.ReadUInt32( ), 12345 );
    ASSERT_FLOAT_EQ( reader.ReadFloat( ), 3.14159f );
    ASSERT_STREQ( reader.ReadString( ).Get( ), "TestString" );

    reader.LogAsCppArray( "TestData" );
}
