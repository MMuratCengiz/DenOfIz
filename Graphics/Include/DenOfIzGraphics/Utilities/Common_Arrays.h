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

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "Common_Macro.h"

typedef unsigned char Byte;

typedef struct DenOfIz_ByteArray
{
    Byte  *Elements;
    size_t NumElements;
} DenOfIz_ByteArray;

typedef struct DenOfIz_ByteArrayView
{
    const Byte *Elements;
    size_t      NumElements;
} DenOfIz_ByteArrayView;

typedef struct DenOfIz_ByteArrayArray
{
    DenOfIz_ByteArray *Elements;
    uint32_t           NumElements;
} DenOfIz_ByteArrayArray;

typedef struct DenOfIz_BoolArray
{
    bool  *Elements;
    size_t NumElements;
} DenOfIz_BoolArray;

typedef struct DenOfIz_FloatArray
{
    float *Elements;
    size_t NumElements;
} DenOfIz_FloatArray;

typedef struct DenOfIz_FloatArrayArray
{
    DenOfIz_FloatArray *Elements;
    size_t              NumElements;
} DenOfIz_FloatArrayArray;

typedef struct DenOfIz_Int32Array
{
    int32_t *Elements;
    size_t   NumElements;
} DenOfIz_Int32Array;

typedef struct DenOfIz_UInt16Array
{
    uint16_t *Elements;
    size_t    NumElements;
} DenOfIz_UInt16Array;

typedef struct DenOfIz_UInt32Array
{
    uint32_t *Elements;
    size_t    NumElements;
} DenOfIz_UInt32Array;

typedef struct DenOfIz_UInt32ArrayView
{
    const uint32_t *Elements;
    size_t          NumElements;
} DenOfIz_UInt32ArrayView;

typedef struct DenOfIz_Int16Array
{
    int16_t *Elements;
    size_t   NumElements;
} DenOfIz_Int16Array;

typedef struct DenOfIz_StringView
{
    const char *Chars;
    size_t      NumChars;
} DenOfIz_StringView;

#define DENOFIZ_STRING( str ) { str, strlen( str ) }
#define DENOFIZ_NULL_STRING DenOfIz_StringView{ NULL, 0 }

typedef struct DenOfIz_StringViewArray
{
    DenOfIz_StringView *Elements;
    size_t              NumElements;
} DenOfIz_StringViewArray;
