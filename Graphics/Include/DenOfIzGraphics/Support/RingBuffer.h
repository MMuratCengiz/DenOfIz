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

#include "DenOfIzGraphics/Backends/Interface/LogicalDevice.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Support/GPUBufferView.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_RingBuffer )

    typedef struct DenOfIz_RingBufferDesc
    {
        DenOfIz_LogicalDevice LogicalDevice;
        size_t                DataNumBytes;
        size_t                NumEntries;
        size_t                MaxChunkNumBytes;
        bool                  IsStructuredBuffer;
    } DenOfIz_RingBufferDesc;

    DZ_API DenOfIz_RingBuffer    DenOfIz_RingBuffer_Create( const DenOfIz_RingBufferDesc *desc );
    DZ_API void                  DenOfIz_RingBuffer_Destroy( DenOfIz_RingBuffer ringBuffer );
    DZ_API DenOfIz_GPUBufferView DenOfIz_RingBuffer_GetBufferView( DenOfIz_RingBuffer ringBuffer, size_t index );
    DZ_API Byte                 *DenOfIz_RingBuffer_GetMappedMemory( DenOfIz_RingBuffer ringBuffer, size_t index );
    DZ_API size_t                DenOfIz_RingBuffer_GetAlignedNumBytes( DenOfIz_RingBuffer ringBuffer );
    DZ_API size_t                DenOfIz_RingBuffer_GetTotalNumBytes( DenOfIz_RingBuffer ringBuffer );

#ifdef __cplusplus
}
#endif
