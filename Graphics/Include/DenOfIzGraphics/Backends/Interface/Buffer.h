/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_Buffer )

    /** Makes the buffer a structured buffer when Stride is not zero. Stride * NumElements must fit in NumBytes. */
    typedef struct DenOfIz_StructuredBufferDesc
    {
        uint64_t Offset;
        uint64_t NumElements;
        uint64_t Stride; /**< Bytes per element. */
    } DenOfIz_StructuredBufferDesc;

    /** STORAGE and ACCELERATION_STRUCTURE usage require HEAP_TYPE_GPU. NumBytes must not be zero. */
    typedef struct DenOfIz_BufferDesc
    {
        size_t                       NumBytes;
        DenOfIz_BufferUsageFlags     Usage;
        DenOfIz_HeapType             HeapType;
        DenOfIz_StructuredBufferDesc StructureDesc;
        DenOfIz_Format               Format; /**< Element format of typed views. UNDEFINED with STORAGE usage and Stride 0 is a raw (byte address) buffer. */
        DenOfIz_StringView           DebugName;
    } DenOfIz_BufferDesc;

    typedef struct DenOfIz_BufferSlice
    {
        DenOfIz_Buffer Buffer;
        uint64_t       Offset; /**< Bytes. */
        uint64_t       NumBytes;
    } DenOfIz_BufferSlice;

    /** Maps a buffer on a CPU visible heap (any HeapType except GPU). The pointer stays valid until UnmapMemory. */
    DZ_API void DenOfIz_Buffer_MapMemory( DenOfIz_Buffer buffer, void **outMappedMemory );
    DZ_API void DenOfIz_Buffer_UnmapMemory( DenOfIz_Buffer buffer );
    /** Allocated size: the desc's NumBytes rounded up to the backend's buffer alignment. */
    DZ_API void DenOfIz_Buffer_NumBytes( DenOfIz_Buffer buffer, size_t *outNumBytes );
    /** The mapped pointer, NULL when the buffer is not mapped. */
    DZ_API void DenOfIz_Buffer_Data( DenOfIz_Buffer buffer, const void **outData );
    /** Views the mapped memory, it is not a copy: Elements is NULL unless the buffer is mapped and stays valid until it is unmapped. */
    DZ_API void DenOfIz_Buffer_GetData( DenOfIz_Buffer buffer, DenOfIz_ByteArray *outData );
    /** Copies data to offset 0, mapping the buffer if needed, and unmaps it afterwards unless keepMapped. data must fit in the buffer. */
    DZ_API void DenOfIz_Buffer_SetData( DenOfIz_Buffer buffer, const DenOfIz_ByteArrayView *data, bool keepMapped );
    /** Copies data to a byte offset, mapping the buffer if needed. The buffer stays mapped. */
    DZ_API void DenOfIz_Buffer_WriteData( DenOfIz_Buffer buffer, const DenOfIz_ByteArrayView *data, uint32_t bufferOffset );
    /** Release is deferred until the GPU is done with the buffer, see DenOfIz_LogicalDevice_ReleaseDestroyedResources. */
    DZ_API void DenOfIz_Buffer_Destroy( DenOfIz_Buffer buffer );

#ifdef __cplusplus
}
#endif
