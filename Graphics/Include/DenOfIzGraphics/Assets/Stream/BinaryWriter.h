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

#include "BinaryContainer.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/Common_Macro.h"
#include "DenOfIzGraphics/Utilities/InteropMath.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_BinaryWriter )

    DZ_API DenOfIz_BinaryWriter DenOfIz_BinaryWriter_CreateFromContainer( DenOfIz_BinaryContainer container );
    DZ_API DenOfIz_BinaryWriter DenOfIz_BinaryWriter_CreateFromFile( DenOfIz_StringView filePath );
    DZ_API void                 DenOfIz_BinaryWriter_Destroy( DenOfIz_BinaryWriter writer );

    DZ_API void     DenOfIz_BinaryWriter_WriteByte( DenOfIz_BinaryWriter writer, Byte value );
    DZ_API void     DenOfIz_BinaryWriter_Write( DenOfIz_BinaryWriter writer, DenOfIz_ByteArrayView buffer, uint32_t offset, uint32_t count );
    DZ_API void     DenOfIz_BinaryWriter_WriteBytes( DenOfIz_BinaryWriter writer, DenOfIz_ByteArrayView buffer );
    DZ_API void     DenOfIz_BinaryWriter_WriteUInt16( DenOfIz_BinaryWriter writer, uint16_t value );
    DZ_API void     DenOfIz_BinaryWriter_WriteUInt32( DenOfIz_BinaryWriter writer, uint32_t value );
    DZ_API void     DenOfIz_BinaryWriter_WriteUInt64( DenOfIz_BinaryWriter writer, uint64_t value );
    DZ_API void     DenOfIz_BinaryWriter_WriteInt16( DenOfIz_BinaryWriter writer, int16_t value );
    DZ_API void     DenOfIz_BinaryWriter_WriteInt32( DenOfIz_BinaryWriter writer, int32_t value );
    DZ_API void     DenOfIz_BinaryWriter_WriteInt64( DenOfIz_BinaryWriter writer, int64_t value );
    DZ_API void     DenOfIz_BinaryWriter_WriteFloat( DenOfIz_BinaryWriter writer, float value );
    DZ_API void     DenOfIz_BinaryWriter_WriteDouble( DenOfIz_BinaryWriter writer, double value );
    DZ_API void     DenOfIz_BinaryWriter_WriteString( DenOfIz_BinaryWriter writer, DenOfIz_StringView value );
    DZ_API void     DenOfIz_BinaryWriter_WriteUInt16_2( DenOfIz_BinaryWriter writer, DenOfIz_UShort2 value );
    DZ_API void     DenOfIz_BinaryWriter_WriteUInt16_3( DenOfIz_BinaryWriter writer, DenOfIz_UShort3 value );
    DZ_API void     DenOfIz_BinaryWriter_WriteUInt16_4( DenOfIz_BinaryWriter writer, DenOfIz_UShort4 value );
    DZ_API void     DenOfIz_BinaryWriter_WriteInt16_2( DenOfIz_BinaryWriter writer, DenOfIz_Short2 value );
    DZ_API void     DenOfIz_BinaryWriter_WriteInt16_3( DenOfIz_BinaryWriter writer, DenOfIz_Short3 value );
    DZ_API void     DenOfIz_BinaryWriter_WriteInt16_4( DenOfIz_BinaryWriter writer, DenOfIz_Short4 value );
    DZ_API void     DenOfIz_BinaryWriter_WriteUInt32_2( DenOfIz_BinaryWriter writer, DenOfIz_UInt2 value );
    DZ_API void     DenOfIz_BinaryWriter_WriteUInt32_3( DenOfIz_BinaryWriter writer, DenOfIz_UInt3 value );
    DZ_API void     DenOfIz_BinaryWriter_WriteUInt32_4( DenOfIz_BinaryWriter writer, DenOfIz_UInt4 value );
    DZ_API void     DenOfIz_BinaryWriter_WriteInt32_2( DenOfIz_BinaryWriter writer, DenOfIz_Int2 value );
    DZ_API void     DenOfIz_BinaryWriter_WriteInt32_3( DenOfIz_BinaryWriter writer, DenOfIz_Int3 value );
    DZ_API void     DenOfIz_BinaryWriter_WriteInt32_4( DenOfIz_BinaryWriter writer, DenOfIz_Int4 value );
    DZ_API void     DenOfIz_BinaryWriter_WriteFloat_2( DenOfIz_BinaryWriter writer, DenOfIz_Float2 value );
    DZ_API void     DenOfIz_BinaryWriter_WriteFloat_3( DenOfIz_BinaryWriter writer, DenOfIz_Float3 value );
    DZ_API void     DenOfIz_BinaryWriter_WriteFloat_4( DenOfIz_BinaryWriter writer, DenOfIz_Float4 value );
    DZ_API void     DenOfIz_BinaryWriter_WriteFloat_4x4( DenOfIz_BinaryWriter writer, DenOfIz_Float4x4 value );
    DZ_API uint64_t DenOfIz_BinaryWriter_Position( DenOfIz_BinaryWriter writer );
    DZ_API void     DenOfIz_BinaryWriter_Seek( DenOfIz_BinaryWriter writer, uint64_t position );
    DZ_API void     DenOfIz_BinaryWriter_Flush( DenOfIz_BinaryWriter writer );

#ifdef __cplusplus
}
#endif
