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

#include "BindGroupLayout.h"
#include "Buffer.h"
#include "DenOfIzGraphics/Handle.h"
#include "RayTracing/TopLevelAS.h"
#include "Texture.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Handle to a resource bind group for shader resource binding.
     *
     * A BindGroup binds shader resources (buffers, textures, samplers) to
     * a specific register space defined in a RootSignature. Multiple bind groups
     * can be bound simultaneously, each targeting different register spaces.
     *
     * @par Backend Implementations
     * - DirectX12: Descriptor heap allocations (CBV/SRV/UAV and Sampler heaps)
     * - Vulkan: VkDescriptorSet
     * - Metal: Argument buffers (MTLBuffer)
     * - WebGPU: WGPUBindGroup
     *
     * @par Update Pattern
     * Resources are bound using a BeginUpdate/EndUpdate pattern:
     * 1. Call BeginUpdate to start binding
     * 2. Bind resources using Cbv/Srv/Uav/Sampler functions
     * 3. Call EndUpdate to finalize bindings
     *
     * Root constants should be set via DenOfIz_CommandList_SetRootConstants.
     *
     * @par Usage Example
     * @code
     * // Create bind group from a bind group layout
     * DenOfIz_BindGroupDesc bindGroupDesc = {0};
     * bindGroupDesc.Layout = bindGroupLayout;
     *
     * DenOfIz_BindGroup bindGroup;
     * DenOfIz_LogicalDevice_CreateBindGroup(device, &bindGroupDesc, &bindGroup);
     *
     * // Create and map a uniform buffer
     * DenOfIz_BufferDesc bufferDesc = {0};
     * bufferDesc.Descriptor = DENOFIZ_RESOURCE_DESCRIPTOR_UNIFORM_BUFFER_BIT;
     * bufferDesc.HeapType   = DENOFIZ_HEAP_TYPE_CPU_GPU;
     * bufferDesc.NumBytes   = sizeof(MyConstants);
     *
     * DenOfIz_Buffer uniformBuffer;
     * DenOfIz_LogicalDevice_CreateBuffer(device, &bufferDesc, &uniformBuffer);
     *
     * // Bind resources
     * DenOfIz_BindGroup_BeginUpdate(bindGroup);
     * DenOfIz_BindGroup_Cbv(bindGroup, 0, uniformBuffer);      // b0: constant buffer
     * DenOfIz_BindGroup_SrvTexture(bindGroup, 0, diffuseTex);  // t0: texture
     * DenOfIz_BindGroup_Sampler(bindGroup, 0, linearSampler); // s0: sampler
     * DenOfIz_BindGroup_EndUpdate(bindGroup);
     *
     * // In render loop:
     * DenOfIz_CommandList_BindPipeline(cmd, pipeline);
     * DenOfIz_CommandList_BindGroup(cmd, bindGroup);
     * DenOfIz_CommandList_Draw(cmd, vertexCount, 1, 0, 0);
     * @endcode
     *
     * @see DenOfIz_BindGroupDesc
     * @see DenOfIz_LogicalDevice_CreateBindGroup
     * @see DenOfIz_CommandList_BindGroup
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_BindGroup )

    /**
     * @brief Descriptor for creating a resource bind group.
     */
    typedef struct DenOfIz_BindGroupDesc
    {
        /**
         * @brief Bind group layout defining the resource layout.
         *
         * The bind group will bind resources according to the slots defined
         * in this layout. The layout determines the register space and all
         * binding slots.
         */
        DenOfIz_BindGroupLayout Layout;
    } DenOfIz_BindGroupDesc;

    /**
     * @brief Descriptor for binding a buffer with offset.
     *
     * Used with CbvWithDesc/SrvBufferWithDesc/UavBufferWithDesc for binding
     * a portion of a buffer starting at ResourceOffset.
     */
    typedef struct DenOfIz_BindBufferDesc
    {
        uint32_t       Binding;        /**< Binding slot number */
        DenOfIz_Buffer Resource;       /**< Buffer to bind */
        size_t         ResourceOffset; /**< Byte offset into the buffer */
    } DenOfIz_BindBufferDesc;

    /**
     * @brief Begins a resource binding update session.
     *
     * Must be called before binding any resources (Cbv, Srv, Uav, Sampler).
     * Call EndUpdate when finished binding.
     *
     * @param bindGroup Valid bind group handle.
     *
     * @par Valid Usage
     * - @p bindGroup must be a valid DenOfIz_BindGroup handle
     * - Must not be called while already in an update session
     */
    DZ_API void DenOfIz_BindGroup_BeginUpdate( DenOfIz_BindGroup bindGroup );

    /**
     * @brief Binds a constant buffer view (CBV).
     *
     * @param bindGroup Valid bind group handle.
     * @param binding CBV binding slot (corresponds to HLSL b# register).
     * @param resource Buffer to bind as constant buffer.
     *
     * @par Valid Usage
     * - Must be called between BeginUpdate and EndUpdate
     * - @p resource must have DENOFIZ_RESOURCE_DESCRIPTOR_UNIFORM_BUFFER_BIT
     */
    DZ_API void DenOfIz_BindGroup_Cbv( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Buffer resource );

    /**
     * @brief Binds a constant buffer view with offset.
     *
     * @param bindGroup Valid bind group handle.
     * @param desc Buffer binding descriptor with offset.
     *
     * @par Valid Usage
     * - Must be called between BeginUpdate and EndUpdate
     * - @p desc must not be NULL
     * - @p desc->ResourceOffset must be aligned to constant buffer alignment
     */
    DZ_API void DenOfIz_BindGroup_CbvWithDesc( DenOfIz_BindGroup bindGroup, const DenOfIz_BindBufferDesc *desc );

    /**
     * @brief Binds a buffer as shader resource view (SRV).
     *
     * @param bindGroup Valid bind group handle.
     * @param binding SRV binding slot (corresponds to HLSL t# register).
     * @param resource Buffer to bind as read-only shader resource.
     *
     * @par Valid Usage
     * - Must be called between BeginUpdate and EndUpdate
     * - @p resource must have DENOFIZ_RESOURCE_DESCRIPTOR_BUFFER_BIT
     */
    DZ_API void DenOfIz_BindGroup_SrvBuffer( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Buffer resource );

    /**
     * @brief Binds a buffer as shader resource view with offset.
     *
     * @param bindGroup Valid bind group handle.
     * @param desc Buffer binding descriptor with offset.
     *
     * @par Valid Usage
     * - Must be called between BeginUpdate and EndUpdate
     * - @p desc must not be NULL
     */
    DZ_API void DenOfIz_BindGroup_SrvBufferWithDesc( DenOfIz_BindGroup bindGroup, const DenOfIz_BindBufferDesc *desc );

    /**
     * @brief Binds a ray tracing acceleration structure as SRV.
     *
     * @param bindGroup Valid bind group handle.
     * @param binding SRV binding slot.
     * @param accelerationStructure Top-level acceleration structure to bind.
     *
     * @par Valid Usage
     * - Must be called between BeginUpdate and EndUpdate
     * - @p accelerationStructure must be a valid built TLAS
     */
    DZ_API void DenOfIz_BindGroup_SrvTopLevelAS( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_TopLevelAS accelerationStructure );

    /**
     * @brief Binds a texture as shader resource view.
     *
     * @param bindGroup Valid bind group handle.
     * @param binding SRV binding slot (corresponds to HLSL t# register).
     * @param resource Texture to bind.
     *
     * @par Valid Usage
     * - Must be called between BeginUpdate and EndUpdate
     * - @p resource must have DENOFIZ_RESOURCE_DESCRIPTOR_TEXTURE_BIT
     */
    DZ_API void DenOfIz_BindGroup_SrvTexture( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Texture resource );

    /**
     * @brief Binds an array of textures as shader resource views.
     *
     * @param bindGroup Valid bind group handle.
     * @param binding SRV binding slot for the array start.
     * @param resources Array of textures to bind.
     *
     * @par Valid Usage
     * - Must be called between BeginUpdate and EndUpdate
     * - @p resources must not be NULL
     */
    DZ_API void DenOfIz_BindGroup_SrvArray( DenOfIz_BindGroup bindGroup, uint32_t binding, const DenOfIz_TextureArray *resources );

    /**
     * @brief Binds a single texture to a specific index in a texture array.
     *
     * @param bindGroup Valid bind group handle.
     * @param binding SRV binding slot for the array.
     * @param arrayIndex Index within the array.
     * @param resource Texture to bind at the index.
     * @param[out] outBindGroup Receives the bind group (may be same or new).
     *
     * @par Valid Usage
     * - Must be called between BeginUpdate and EndUpdate
     * - @p arrayIndex must be within array bounds
     * - @p outBindGroup must not be NULL
     */
    DZ_API void DenOfIz_BindGroup_SrvArrayIndex( DenOfIz_BindGroup bindGroup, uint32_t binding, uint32_t arrayIndex, DenOfIz_Texture resource, DenOfIz_BindGroup *outBindGroup );

    /**
     * @brief Binds a buffer as unordered access view (UAV).
     *
     * @param bindGroup Valid bind group handle.
     * @param binding UAV binding slot (corresponds to HLSL u# register).
     * @param resource Buffer to bind for read-write access.
     *
     * @par Valid Usage
     * - Must be called between BeginUpdate and EndUpdate
     * - @p resource must have DENOFIZ_RESOURCE_DESCRIPTOR_RW_BUFFER_BIT
     */
    DZ_API void DenOfIz_BindGroup_UavBuffer( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Buffer resource );

    /**
     * @brief Binds a buffer as UAV with offset.
     *
     * @param bindGroup Valid bind group handle.
     * @param desc Buffer binding descriptor with offset.
     *
     * @par Valid Usage
     * - Must be called between BeginUpdate and EndUpdate
     * - @p desc must not be NULL
     */
    DZ_API void DenOfIz_BindGroup_UavBufferWithDesc( DenOfIz_BindGroup bindGroup, const DenOfIz_BindBufferDesc *desc );

    /**
     * @brief Binds a texture as unordered access view.
     *
     * @param bindGroup Valid bind group handle.
     * @param binding UAV binding slot (corresponds to HLSL u# register).
     * @param resource Texture to bind for read-write access.
     *
     * @par Valid Usage
     * - Must be called between BeginUpdate and EndUpdate
     * - @p resource must have DENOFIZ_RESOURCE_DESCRIPTOR_RW_TEXTURE_BIT
     */
    DZ_API void DenOfIz_BindGroup_UavTexture( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Texture resource );

    /**
     * @brief Binds a texture sampler.
     *
     * @param bindGroup Valid bind group handle.
     * @param binding Sampler binding slot (corresponds to HLSL s# register).
     * @param sampler Sampler to bind.
     *
     * @par Valid Usage
     * - Must be called between BeginUpdate and EndUpdate
     * - @p sampler must be a valid DenOfIz_Sampler handle
     */
    DZ_API void DenOfIz_BindGroup_Sampler( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Sampler sampler );

    /**
     * @brief Ends a resource binding update session.
     *
     * Finalizes all resource bindings made since BeginUpdate. After calling
     * EndUpdate, the bind group is ready to be bound to a command list.
     *
     * @param bindGroup Valid bind group handle.
     *
     * @par Valid Usage
     * - @p bindGroup must be a valid DenOfIz_BindGroup handle
     * - Must be called after BeginUpdate
     */
    DZ_API void DenOfIz_BindGroup_EndUpdate( DenOfIz_BindGroup bindGroup );

    /**
     * @brief Destroys a resource bind group and releases its resources.
     *
     * @param bindGroup Bind group handle to destroy.
     *
     * @par Valid Usage
     * - @p bindGroup must be a valid DenOfIz_BindGroup handle or DENOFIZ_NULL_HANDLE
     * - The bind group must not be in use by a pending command list
     */
    DZ_API void DenOfIz_BindGroup_Destroy( DenOfIz_BindGroup bindGroup );

#ifdef __cplusplus
}
#endif
