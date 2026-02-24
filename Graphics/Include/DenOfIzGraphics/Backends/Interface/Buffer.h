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

#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Buffer handle for GPU memory.
     *
     * @par Backend implementations
     * - DirectX 12: ID3D12Resource2 + D3D12MA::Allocation
     * - Vulkan: VkBuffer + VmaAllocation
     * - Metal: id<MTLBuffer>
     * - WebGPU: WGPUBufferX
     *
     * @par Heap types
     * - DENOFIZ_HEAP_TYPE_GPU: Device-local, fastest for GPU. Use staging buffer for uploads.
     * - DENOFIZ_HEAP_TYPE_CPU_GPU: CPU-writable, GPU-readable. Good for dynamic uniform buffers.
     * - DENOFIZ_HEAP_TYPE_GPU_CPU: GPU-writable, CPU-readable. For readback operations.
     * - DENOFIZ_HEAP_TYPE_CPU: CPU-only. For staging buffers.
     *
     * @par Example: Uploading vertex data
     * @code
     * // For CPU_GPU heap, map and write directly:
     * void *mapped;
     * DenOfIz_Buffer_MapMemory(buffer, &mapped);
     * memcpy(mapped, vertices, sizeof(vertices));
     * DenOfIz_Buffer_UnmapMemory(buffer);
     *
     * // For GPU heap, use DenOfIz_BatchResourceCopy to stage and copy.
     * @endcode
     *
     * @see DenOfIz_LogicalDevice_CreateBuffer
     * @see DenOfIz_BufferDesc
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_Buffer )

    /**
     * @brief Structured buffer configuration for storage buffer access.
     *
     * Used when Usage includes STORAGE_BIT and Stride > 0.
     */
    typedef struct DenOfIz_StructuredBufferDesc
    {
        uint64_t Offset;      /**< Byte offset to first element in buffer. */
        uint64_t NumElements; /**< Number of structure elements. */
        uint64_t Stride;      /**< Size of each structure element in bytes. */
    } DenOfIz_StructuredBufferDesc;

    /**
     * @brief Descriptor for creating a buffer resource.
     *
     * @par Validation Rules
     * - NumBytes must be > 0
     * - STORAGE usage with StructureDesc.Stride > 0 creates a structured buffer
     * - GPU-only buffers without COPY_DST usage cannot receive data
     *
     * @par Usage flags (DenOfIz_BufferUsageFlags)
     * - VERTEX_BIT: Can be bound as vertex buffer
     * - INDEX_BIT: Can be bound as index buffer
     * - UNIFORM_BIT: Can be bound as constant/uniform buffer
     * - STORAGE_BIT: Can be bound as storage/structured buffer (read-write)
     * - INDIRECT_BIT: Can be used for indirect draw/dispatch arguments
     * - COPY_SRC_BIT/COPY_DST_BIT: Can be source/destination of copy operations
     *
     * @see DenOfIz_LogicalDevice_CreateBuffer
     */
    typedef struct DenOfIz_BufferDesc
    {
        size_t                       NumBytes;      /**< Total buffer size in bytes. Must be > 0. */
        DenOfIz_BufferUsageFlags     Usage;         /**< Buffer usage flags (VERTEX, INDEX, UNIFORM, STORAGE, etc.). */
        DenOfIz_HeapType             HeapType;      /**< Memory heap type (GPU, CPU_GPU, etc.). */
        DenOfIz_StructuredBufferDesc StructureDesc; /**< Configuration for structured buffers (when STORAGE is used). */
        DenOfIz_Format               Format;        /**< Element format for typed buffers (e.g. R32_FLOAT). */
        DenOfIz_StringView           DebugName;     /**< Debug name for graphics debuggers. */
    } DenOfIz_BufferDesc;

    /**
     * @brief Represents a sub-region of a buffer.
     */
    typedef struct DenOfIz_BufferSlice
    {
        DenOfIz_Buffer Buffer;   /**< The buffer this slice references. */
        uint64_t       Offset;   /**< Byte offset from buffer start. */
        uint64_t       NumBytes; /**< Size of the slice in bytes. */
    } DenOfIz_BufferSlice;

    /**
     * @brief Maps buffer memory for CPU access.
     *
     * Returns a pointer to the buffer's memory that can be read or written by the CPU.
     * The buffer remains mapped until DenOfIz_Buffer_UnmapMemory() is called.
     *
     * @param buffer Valid buffer handle.
     * @param[out] outMappedMemory Receives pointer to the mapped memory.
     *
     * @par Valid Usage
     * - @p buffer must be a valid handle
     * - @p buffer must be created with DENOFIZ_HEAP_TYPE_CPU_GPU or DENOFIZ_HEAP_TYPE_GPU_CPU
     * - @p outMappedMemory must not be NULL
     * - For GPU-only buffers (DENOFIZ_HEAP_TYPE_GPU), mapping will fail
     *
     * @note On some backends (Metal), buffers are persistently mapped and this is a no-op.
     *
     * @see DenOfIz_Buffer_UnmapMemory
     */
    DZ_API void DenOfIz_Buffer_MapMemory( DenOfIz_Buffer buffer, void **outMappedMemory );

    /**
     * @brief Unmaps previously mapped buffer memory.
     *
     * @param buffer Valid buffer handle.
     *
     * @par Valid Usage
     * - @p buffer must be a valid handle
     * - @p buffer must have been previously mapped with MapMemory
     */
    DZ_API void DenOfIz_Buffer_UnmapMemory( DenOfIz_Buffer buffer );

    /**
     * @brief Returns the size of the buffer in bytes.
     *
     * @param buffer Valid buffer handle.
     * @param[out] outNumBytes Receives the buffer size.
     *
     * @par Valid Usage
     * - @p buffer must be a valid handle
     * - @p outNumBytes must not be NULL
     */
    DZ_API void DenOfIz_Buffer_NumBytes( DenOfIz_Buffer buffer, size_t *outNumBytes );

    /**
     * @brief Returns pointer to the buffer's mapped memory (if mapped).
     *
     * @param buffer Valid buffer handle.
     * @param[out] outData Receives pointer to mapped data, or NULL if not mapped.
     *
     * @par Valid Usage
     * - @p buffer must be a valid handle
     * - @p outData must not be NULL
     */
    DZ_API void DenOfIz_Buffer_Data( DenOfIz_Buffer buffer, const void **outData );

    /**
     * @brief Reads buffer data into a byte array.
     *
     * Maps the buffer, copies data, and unmaps. For frequent reads, prefer keeping
     * the buffer mapped.
     *
     * @param buffer Valid buffer handle.
     * @param[out] outData Receives the buffer contents as a byte array.
     *
     * @par Valid Usage
     * - @p buffer must be a valid handle with CPU-readable heap type
     * - @p outData must not be NULL
     */
    DZ_API void DenOfIz_Buffer_GetData( DenOfIz_Buffer buffer, DenOfIz_ByteArray *outData );

    /**
     * @brief Writes data to the buffer, replacing all contents.
     *
     * Maps the buffer, copies data, and optionally keeps it mapped for subsequent writes.
     *
     * @param buffer Valid buffer handle.
     * @param data Data to write. Must not be NULL.
     * @param keepMapped If true, buffer remains mapped after the write.
     *
     * @par Valid Usage
     * - @p buffer must be a valid handle with CPU-writable heap type
     * - @p data must not be NULL
     * - @p data->NumElements must be <= buffer's NumBytes
     */
    DZ_API void DenOfIz_Buffer_SetData( DenOfIz_Buffer buffer, const DenOfIz_ByteArrayView *data, bool keepMapped );

    /**
     * @brief Writes data to a specific offset in the buffer.
     *
     * Unlike SetData, this allows partial buffer updates.
     *
     * @param buffer Valid buffer handle.
     * @param data Data to write. Must not be NULL.
     * @param bufferOffset Byte offset in the buffer to start writing.
     *
     * @par Valid Usage
     * - @p buffer must be a valid handle with CPU-writable heap type
     * - @p data must not be NULL
     * - @p bufferOffset + data->NumElements must be <= buffer's NumBytes
     */
    DZ_API void DenOfIz_Buffer_WriteData( DenOfIz_Buffer buffer, const DenOfIz_ByteArrayView *data, uint32_t bufferOffset );

    /**
     * @brief Destroys the buffer and releases GPU memory.
     *
     * @param buffer Buffer handle to destroy. May be DENOFIZ_NULL_HANDLE.
     *
     * @par Valid Usage
     * - All command lists referencing this buffer must have completed
     * - Buffer must not be mapped when destroyed
     * - After this call, @p buffer is invalid and must not be used
     */
    DZ_API void DenOfIz_Buffer_Destroy( DenOfIz_Buffer buffer );

#ifdef __cplusplus
}
#endif
