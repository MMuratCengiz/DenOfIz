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

    DENOFIZ_DEFINE_HANDLE( DenOfIz_BinaryReader )

    typedef struct DenOfIz_BinaryReaderDesc
    {
        uint64_t NumBytes; // 0 means reads the whole contents
    } DenOfIz_BinaryReaderDesc;

    DZ_API DenOfIz_BinaryReader DenOfIz_BinaryReader_CreateFromContainer( DenOfIz_BinaryContainer container, const DenOfIz_BinaryReaderDesc *desc );
    DZ_API DenOfIz_BinaryReader DenOfIz_BinaryReader_CreateFromFile( DenOfIz_StringView filePath, const DenOfIz_BinaryReaderDesc *desc );
    DZ_API DenOfIz_BinaryReader DenOfIz_BinaryReader_CreateFromData( DenOfIz_ByteArrayView data, const DenOfIz_BinaryReaderDesc *desc );
    DZ_API void                 DenOfIz_BinaryReader_Destroy( DenOfIz_BinaryReader reader );

    DZ_API int                ReadByte( DenOfIz_BinaryReader reader );
    DZ_API int                DenOfIz_BinaryReader_Read( DenOfIz_BinaryReader reader, DenOfIz_ByteArray buffer, uint32_t offset, uint32_t count );
    DZ_API DenOfIz_ByteArray  DenOfIz_BinaryReader_ReadAllBytes( DenOfIz_BinaryReader reader );
    DZ_API DenOfIz_ByteArray  DenOfIz_BinaryReader_ReadBytes( DenOfIz_BinaryReader reader, uint32_t count );
    DZ_API uint16_t           DenOfIz_BinaryReader_ReadUInt16( DenOfIz_BinaryReader reader );
    DZ_API uint32_t           DenOfIz_BinaryReader_ReadUInt32( DenOfIz_BinaryReader reader );
    DZ_API uint64_t           DenOfIz_BinaryReader_ReadUInt64( DenOfIz_BinaryReader reader );
    DZ_API int16_t            DenOfIz_BinaryReader_ReadInt16( DenOfIz_BinaryReader reader );
    DZ_API int32_t            DenOfIz_BinaryReader_ReadInt32( DenOfIz_BinaryReader reader );
    DZ_API int64_t            DenOfIz_BinaryReader_ReadInt64( DenOfIz_BinaryReader reader );
    DZ_API float              DenOfIz_BinaryReader_ReadFloat( DenOfIz_BinaryReader reader );
    DZ_API double             DenOfIz_BinaryReader_ReadDouble( DenOfIz_BinaryReader reader );
    DZ_API DenOfIz_StringView DenOfIz_BinaryReader_ReadString( DenOfIz_BinaryReader reader );
    DZ_API DenOfIz_UShort2    DenOfIz_BinaryReader_ReadUInt16_2( DenOfIz_BinaryReader reader );
    DZ_API DenOfIz_UShort3    DenOfIz_BinaryReader_ReadUInt16_3( DenOfIz_BinaryReader reader );
    DZ_API DenOfIz_UShort4    DenOfIz_BinaryReader_ReadUInt16_4( DenOfIz_BinaryReader reader );
    DZ_API DenOfIz_Short2     DenOfIz_BinaryReader_ReadInt16_2( DenOfIz_BinaryReader reader );
    DZ_API DenOfIz_Short3     DenOfIz_BinaryReader_ReadInt16_3( DenOfIz_BinaryReader reader );
    DZ_API DenOfIz_Short4     DenOfIz_BinaryReader_ReadInt16_4( DenOfIz_BinaryReader reader );
    DZ_API DenOfIz_UInt2      DenOfIz_BinaryReader_ReadUInt32_2( DenOfIz_BinaryReader reader );
    DZ_API DenOfIz_UInt3      DenOfIz_BinaryReader_ReadUInt32_3( DenOfIz_BinaryReader reader );
    DZ_API DenOfIz_UInt4      DenOfIz_BinaryReader_ReadUInt32_4( DenOfIz_BinaryReader reader );
    DZ_API DenOfIz_Int2       DenOfIz_BinaryReader_ReadInt32_2( DenOfIz_BinaryReader reader );
    DZ_API DenOfIz_Int3       DenOfIz_BinaryReader_ReadInt32_3( DenOfIz_BinaryReader reader );
    DZ_API DenOfIz_Int4       DenOfIz_BinaryReader_ReadInt32_4( DenOfIz_BinaryReader reader );
    DZ_API DenOfIz_Float2     DenOfIz_BinaryReader_ReadFloat_2( DenOfIz_BinaryReader reader );
    DZ_API DenOfIz_Float3     DenOfIz_BinaryReader_ReadFloat_3( DenOfIz_BinaryReader reader );
    DZ_API DenOfIz_Float4     DenOfIz_BinaryReader_ReadFloat_4( DenOfIz_BinaryReader reader );
    DZ_API DenOfIz_Float4x4   DenOfIz_BinaryReader_ReadFloat_4x4( DenOfIz_BinaryReader reader );
    DZ_API uint64_t           DenOfIz_BinaryReader_Position( DenOfIz_BinaryReader reader );
    DZ_API void               DenOfIz_BinaryReader_Seek( DenOfIz_BinaryReader reader, uint64_t position );
    DZ_API void               DenOfIz_BinaryReader_Skip( DenOfIz_BinaryReader reader, uint64_t count );

#ifdef __cplusplus
}
#endif
