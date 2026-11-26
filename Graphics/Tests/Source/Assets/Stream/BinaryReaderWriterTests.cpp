/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/
#include "gtest/gtest.h"

#include <cstring>
#include <string_view>
#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryReader.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"

namespace
{
    std::string_view View( const DenOfIz_StringView str )
    {
        return { str.Chars, str.NumChars };
    }
} // namespace

TEST( BinarySerdeTest, BasicTypes )
{
    DenOfIz_BinaryContainer container = DenOfIz_BinaryContainer_Create( );
    DenOfIz_BinaryWriter    writer    = DenOfIz_BinaryWriter_CreateFromContainer( container );
    DenOfIz_BinaryWriter_WriteUInt16( writer, 12345 );
    DenOfIz_BinaryWriter_WriteInt32( writer, -54321 );
    DenOfIz_BinaryWriter_WriteFloat( writer, 3.14159f );
    DenOfIz_BinaryWriter_WriteUInt64( writer, 9876543210ULL );
    DenOfIz_BinaryWriter_WriteString( writer, DENOFIZ_STRING( "Hello Binary!" ) );
    DenOfIz_BinaryWriter_WriteByte( writer, 0xAB );
    DenOfIz_BinaryWriter_Destroy( writer );

    DenOfIz_BinaryReader reader = DenOfIz_BinaryReader_CreateFromContainer( container, nullptr );
    EXPECT_EQ( DenOfIz_BinaryReader_ReadUInt16( reader ), 12345 );
    EXPECT_EQ( DenOfIz_BinaryReader_ReadInt32( reader ), -54321 );
    EXPECT_FLOAT_EQ( DenOfIz_BinaryReader_ReadFloat( reader ), 3.14159f );
    EXPECT_EQ( DenOfIz_BinaryReader_ReadUInt64( reader ), 9876543210ULL );
    EXPECT_EQ( View( DenOfIz_BinaryReader_ReadString( reader ) ), "Hello Binary!" );
    EXPECT_EQ( DenOfIz_BinaryReader_ReadByte( reader ), 0xAB );
    DenOfIz_BinaryReader_Destroy( reader );
    DenOfIz_BinaryContainer_Destroy( container );
}

TEST( BinarySerdeTest, MathTypes )
{
    const DenOfIz_Float2 vec2 = { 1.0f, 2.0f };
    const DenOfIz_Float3 vec3 = { 3.0f, 4.0f, 5.0f };
    const DenOfIz_Float4 vec4 = { 6.0f, 7.0f, 8.0f, 9.0f };
    DenOfIz_Float4x4     mat4{ };
    float               *mat4Values = &mat4._11;
    for ( int i = 0; i < 16; ++i )
    {
        mat4Values[ i ] = static_cast<float>( ( i / 4 + 1 ) * ( i % 4 + 1 ) );
    }

    DenOfIz_BinaryContainer container = DenOfIz_BinaryContainer_Create( );
    DenOfIz_BinaryWriter    writer    = DenOfIz_BinaryWriter_CreateFromContainer( container );
    DenOfIz_BinaryWriter_WriteFloat_2( writer, vec2 );
    DenOfIz_BinaryWriter_WriteFloat_3( writer, vec3 );
    DenOfIz_BinaryWriter_WriteFloat_4( writer, vec4 );
    DenOfIz_BinaryWriter_WriteFloat_4x4( writer, mat4 );
    DenOfIz_BinaryWriter_Destroy( writer );

    DenOfIz_BinaryReader   reader = DenOfIz_BinaryReader_CreateFromContainer( container, nullptr );
    const DenOfIz_Float2   rVec2  = DenOfIz_BinaryReader_ReadFloat_2( reader );
    const DenOfIz_Float3   rVec3  = DenOfIz_BinaryReader_ReadFloat_3( reader );
    const DenOfIz_Float4   rVec4  = DenOfIz_BinaryReader_ReadFloat_4( reader );
    const DenOfIz_Float4x4 rMat4  = DenOfIz_BinaryReader_ReadFloat_4x4( reader );
    DenOfIz_BinaryReader_Destroy( reader );
    DenOfIz_BinaryContainer_Destroy( container );

    EXPECT_FLOAT_EQ( rVec2.X, vec2.X );
    EXPECT_FLOAT_EQ( rVec2.Y, vec2.Y );
    EXPECT_FLOAT_EQ( rVec3.X, vec3.X );
    EXPECT_FLOAT_EQ( rVec3.Y, vec3.Y );
    EXPECT_FLOAT_EQ( rVec3.Z, vec3.Z );
    EXPECT_FLOAT_EQ( rVec4.X, vec4.X );
    EXPECT_FLOAT_EQ( rVec4.Y, vec4.Y );
    EXPECT_FLOAT_EQ( rVec4.Z, vec4.Z );
    EXPECT_FLOAT_EQ( rVec4.W, vec4.W );
    const float *rMat4Values = &rMat4._11;
    for ( int i = 0; i < 16; ++i )
    {
        EXPECT_FLOAT_EQ( rMat4Values[ i ], mat4Values[ i ] );
    }
}

TEST( BinarySerdeTest, SeekingAndPosition )
{
    DenOfIz_BinaryContainer container = DenOfIz_BinaryContainer_Create( );
    DenOfIz_BinaryWriter    writer    = DenOfIz_BinaryWriter_CreateFromContainer( container );
    DenOfIz_BinaryWriter_WriteUInt32( writer, 111 );
    EXPECT_EQ( DenOfIz_BinaryWriter_Position( writer ), 4 );
    DenOfIz_BinaryWriter_WriteUInt32( writer, 222 );
    EXPECT_EQ( DenOfIz_BinaryWriter_Position( writer ), 8 );
    DenOfIz_BinaryWriter_WriteUInt32( writer, 333 );
    EXPECT_EQ( DenOfIz_BinaryWriter_Position( writer ), 12 );
    DenOfIz_BinaryWriter_Seek( writer, 4 );
    EXPECT_EQ( DenOfIz_BinaryWriter_Position( writer ), 4 );
    DenOfIz_BinaryWriter_WriteUInt32( writer, 999 );
    EXPECT_EQ( DenOfIz_BinaryWriter_Position( writer ), 8 );
    DenOfIz_BinaryWriter_Destroy( writer );

    DenOfIz_BinaryReader reader = DenOfIz_BinaryReader_CreateFromContainer( container, nullptr );
    EXPECT_EQ( DenOfIz_BinaryReader_ReadUInt32( reader ), 111 );
    EXPECT_EQ( DenOfIz_BinaryReader_Position( reader ), 4 );
    EXPECT_EQ( DenOfIz_BinaryReader_ReadUInt32( reader ), 999 );
    EXPECT_EQ( DenOfIz_BinaryReader_Position( reader ), 8 );
    EXPECT_EQ( DenOfIz_BinaryReader_ReadUInt32( reader ), 333 );
    EXPECT_EQ( DenOfIz_BinaryReader_Position( reader ), 12 );
    DenOfIz_BinaryReader_Seek( reader, 0 );
    EXPECT_EQ( DenOfIz_BinaryReader_Position( reader ), 0 );
    EXPECT_EQ( DenOfIz_BinaryReader_ReadUInt32( reader ), 111 );
    DenOfIz_BinaryReader_Destroy( reader );
    DenOfIz_BinaryContainer_Destroy( container );
}

TEST( BinarySerdeTest, ReadFromData )
{
    DenOfIz_BinaryContainer container = DenOfIz_BinaryContainer_Create( );
    DenOfIz_BinaryWriter    writer    = DenOfIz_BinaryWriter_CreateFromContainer( container );
    DenOfIz_BinaryWriter_WriteUInt32( writer, 12345 );
    DenOfIz_BinaryWriter_WriteFloat( writer, 3.14159f );
    DenOfIz_BinaryWriter_WriteString( writer, DENOFIZ_STRING( "TestString" ) );
    DenOfIz_BinaryWriter_Flush( writer );
    DenOfIz_BinaryWriter_Destroy( writer );

    DenOfIz_BinaryReader reader = DenOfIz_BinaryReader_CreateFromData( DenOfIz_BinaryContainer_GetData( container ), nullptr );
    EXPECT_EQ( DenOfIz_BinaryReader_ReadUInt32( reader ), 12345 );
    EXPECT_FLOAT_EQ( DenOfIz_BinaryReader_ReadFloat( reader ), 3.14159f );
    EXPECT_EQ( View( DenOfIz_BinaryReader_ReadString( reader ) ), "TestString" );
    DenOfIz_BinaryReader_Destroy( reader );
    DenOfIz_BinaryContainer_Destroy( container );
}
