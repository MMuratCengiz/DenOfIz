/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <stddef.h>
#include <stdint.h>

/* Plain interop types without math functions, convert them to the types of your math library. */

typedef struct DenOfIz_Float2
{
    float X;
    float Y;
} DenOfIz_Float2;

typedef struct DenOfIz_Float2Array
{
    DenOfIz_Float2 *Elements;
    size_t          NumElements;
} DenOfIz_Float2Array;

typedef struct DenOfIz_Float3
{
    float X;
    float Y;
    float Z;
} DenOfIz_Float3;

typedef struct DenOfIz_Float3Array
{
    DenOfIz_Float3 *Elements;
    size_t          NumElements;
} DenOfIz_Float3Array;

typedef struct DenOfIz_Float4
{
    float X;
    float Y;
    float Z;
    float W;
} DenOfIz_Float4;

typedef DenOfIz_Float4 DenOfIz_Quaternion;

typedef struct DenOfIz_Float4Array
{
    DenOfIz_Float4 *Elements;
    size_t          NumElements;
} DenOfIz_Float4Array;

typedef struct DenOfIz_Short2
{
    int16_t X;
    int16_t Y;
} DenOfIz_Short2;

typedef struct DenOfIz_Short3
{
    int16_t X;
    int16_t Y;
    int16_t Z;
} DenOfIz_Short3;

typedef struct DenOfIz_Short4
{
    int16_t X;
    int16_t Y;
    int16_t Z;
    int16_t W;
} DenOfIz_Short4;

typedef struct DenOfIz_Int2
{
    int X;
    int Y;
} DenOfIz_Int2;

typedef struct DenOfIz_Int3
{
    int X;
    int Y;
    int Z;
} DenOfIz_Int3;

typedef struct DenOfIz_Int4
{
    int X;
    int Y;
    int Z;
    int W;
} DenOfIz_Int4;

typedef struct DenOfIz_UShort2
{
    uint16_t X;
    uint16_t Y;
} DenOfIz_UShort2;

typedef struct DenOfIz_UShort3
{
    uint16_t X;
    uint16_t Y;
    uint16_t Z;
} DenOfIz_UShort3;

typedef struct DenOfIz_UShort4
{
    uint16_t X;
    uint16_t Y;
    uint16_t Z;
    uint16_t W;
} DenOfIz_UShort4;

typedef struct DenOfIz_UInt2
{
    uint32_t X;
    uint32_t Y;
} DenOfIz_UInt2;

typedef struct DenOfIz_UInt3
{
    uint32_t X;
    uint32_t Y;
    uint32_t Z;
} DenOfIz_UInt3;

typedef struct DenOfIz_UInt4
{
    uint32_t X;
    uint32_t Y;
    uint32_t Z;
    uint32_t W;
} DenOfIz_UInt4;

typedef struct DenOfIz_Float4x4
{
    float _11;
    float _12;
    float _13;
    float _14;
    float _21;
    float _22;
    float _23;
    float _24;
    float _31;
    float _32;
    float _33;
    float _34;
    float _41;
    float _42;
    float _43;
    float _44;
} DenOfIz_Float4x4;

#define DENOFIZ_FLOAT4X4_IDENTITY { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f }

typedef struct DenOfIz_Float4x4Array
{
    DenOfIz_Float4x4 *Elements;
    size_t            NumElements;
} DenOfIz_Float4x4Array;
