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

#include "BindGroup.h"
#include "DenOfIzGraphics/Handle.h"
#include "PipelineBarrierDesc.h"
#include "QueryPool.h"
#include "RayTracing/ShaderBindingTable.h"

/**
 * @brief Describes a render target or depth/stencil attachment for dynamic rendering.
 *
 * Used with DenOfIz_RenderingDesc to configure attachments for BeginRendering.
 */
typedef struct DenOfIz_RenderingAttachmentDesc
{
    DenOfIz_LoadOp  LoadOp;            /**< Action to perform on attachment at render pass start. */
    DenOfIz_StoreOp StoreOp;           /**< Action to perform on attachment at render pass end. */
    DenOfIz_Texture Resource;          /**< Texture resource to use as attachment. */
    DenOfIz_Float4  ClearColor;        /**< Clear value for color attachments when LoadOp is CLEAR. */
    DenOfIz_Float2  ClearDepthStencil; /**< Clear values (depth, stencil) when LoadOp is CLEAR. */
} DenOfIz_RenderingAttachmentDesc;

typedef struct DenOfIz_RenderingAttachmentDescArray
{
    DenOfIz_RenderingAttachmentDesc *Elements;
    uint32_t                         NumElements;
} DenOfIz_RenderingAttachmentDescArray;

/**
 * @brief Describes a dynamic rendering pass configuration.
 *
 * Configures render targets, depth/stencil attachments, and render area for BeginRendering.
 * This uses dynamic rendering (VK_KHR_dynamic_rendering / D3D12 render passes) rather than
 * traditional render pass objects.
 */
typedef struct DenOfIz_RenderingDesc
{
    DenOfIz_ViewMask                     ViewMask;          /**< Mask for multiview rendering. Use DENOFIZ_VIEW_MASK_NONE for standard rendering. */
    DenOfIz_RenderingAttachmentDesc      DepthAttachment;   /**< Depth buffer attachment configuration. */
    DenOfIz_RenderingAttachmentDesc      StencilAttachment; /**< Stencil buffer attachment configuration. */
    float                                RenderAreaWidth;   /**< Width of render area in pixels. If 0, uses attachment dimensions. */
    float                                RenderAreaHeight;  /**< Height of render area in pixels. If 0, uses attachment dimensions. */
    float                                RenderAreaOffsetX; /**< X offset of render area in pixels. */
    float                                RenderAreaOffsetY; /**< Y offset of render area in pixels. */
    uint32_t                             NumLayers;         /**< Number of array layers to render to. Must be >= 1. */
    DenOfIz_RenderingAttachmentDescArray RTAttachments;     /**< Color render target attachments. */
} DenOfIz_RenderingDesc;

/**
 * @brief Describes a buffer-to-buffer copy operation.
 */
typedef struct DenOfIz_CopyBufferRegionDesc
{
    DenOfIz_Buffer DstBuffer; /**< Destination buffer. Must have COPY_DST usage. */
    uint64_t       DstOffset; /**< Byte offset into destination buffer. */
    DenOfIz_Buffer SrcBuffer; /**< Source buffer. Must have COPY_SRC usage. */
    uint64_t       SrcOffset; /**< Byte offset into source buffer. */
    uint64_t       NumBytes;  /**< Number of bytes to copy. Must be > 0. */
} DenOfIz_CopyBufferRegionDesc;

/**
 * @brief Describes a texture-to-texture copy operation.
 */
typedef struct DenOfIz_CopyTextureRegionDesc
{
    DenOfIz_Texture SrcTexture;    /**< Source texture. Must have COPY_SRC usage. */
    DenOfIz_Texture DstTexture;    /**< Destination texture. Must have COPY_DST usage. */
    uint32_t        SrcX;          /**< X offset in source texture (pixels). */
    uint32_t        SrcY;          /**< Y offset in source texture (pixels). */
    uint32_t        SrcZ;          /**< Z offset in source texture (for 3D textures). */
    uint32_t        DstX;          /**< X offset in destination texture (pixels). */
    uint32_t        DstY;          /**< Y offset in destination texture (pixels). */
    uint32_t        DstZ;          /**< Z offset in destination texture (for 3D textures). */
    uint32_t        Width;         /**< Width of region to copy (pixels). */
    uint32_t        Height;        /**< Height of region to copy (pixels). */
    uint32_t        Depth;         /**< Depth of region to copy (for 3D textures). */
    uint32_t        SrcMipLevel;   /**< Mip level in source texture. */
    uint32_t        DstMipLevel;   /**< Mip level in destination texture. */
    uint32_t        SrcArrayLayer; /**< Array layer in source texture. */
    uint32_t        DstArrayLayer; /**< Array layer in destination texture. */
} DenOfIz_CopyTextureRegionDesc;

/**
 * @brief Describes a buffer-to-texture copy operation.
 *
 * Copies data from a buffer (typically a staging buffer) to a texture resource.
 */
typedef struct DenOfIz_CopyBufferToTextureDesc
{
    DenOfIz_Texture DstTexture; /**< Destination texture. Must have COPY_DST usage. */
    DenOfIz_Buffer  SrcBuffer;  /**< Source buffer containing pixel data. */
    size_t          SrcOffset;  /**< Byte offset into source buffer. */
    uint32_t        DstX;       /**< X offset in destination texture (pixels). */
    uint32_t        DstY;       /**< Y offset in destination texture (pixels). */
    uint32_t        DstZ;       /**< Z offset in destination texture (for 3D textures). */
    DenOfIz_Format  Format;     /**< Pixel format of the data. */
    uint32_t        MipLevel;   /**< Destination mip level. */
    uint32_t        ArrayLayer; /**< Destination array layer. */
    uint32_t        RowPitch;   /**< Bytes per row in source buffer. Must be aligned to device requirements. */
    uint32_t        NumRows;    /**< Number of rows to copy. */
} DenOfIz_CopyBufferToTextureDesc;

/**
 * @brief Describes a texture-to-buffer copy operation.
 *
 * Copies pixel data from a texture to a buffer (typically for readback).
 */
typedef struct DenOfIz_CopyTextureToBufferDesc
{
    DenOfIz_Buffer  DstBuffer;  /**< Destination buffer. Must have COPY_DST usage. */
    DenOfIz_Texture SrcTexture; /**< Source texture. Must have COPY_SRC usage. */
    uint32_t        DstOffset;  /**< Byte offset into destination buffer. */
    uint32_t        SrcX;       /**< X offset in source texture (pixels). */
    uint32_t        SrcY;       /**< Y offset in source texture (pixels). */
    uint32_t        SrcZ;       /**< Z offset in source texture (for 3D textures). */
    DenOfIz_Format  Format;     /**< Pixel format of the data. */
    uint32_t        MipLevel;   /**< Source mip level. */
    uint32_t        ArrayLayer; /**< Source array layer. */
    uint32_t        RowPitch;   /**< Bytes per row in destination buffer. */
    uint32_t        NumRows;    /**< Number of rows to copy. */
} DenOfIz_CopyTextureToBufferDesc;

/**
 * @brief Describes a ray tracing dispatch operation.
 */
typedef struct DenOfIz_DispatchRaysDesc
{
    uint32_t                   Width;              /**< Width of ray generation grid (typically screen width). */
    uint32_t                   Height;             /**< Height of ray generation grid (typically screen height). */
    uint32_t                   Depth;              /**< Depth of ray generation grid (typically 1). */
    DenOfIz_ShaderBindingTable ShaderBindingTable; /**< Shader binding table containing ray gen, miss, and hit group shaders. */
} DenOfIz_DispatchRaysDesc;

/**
 * @brief Structure matching GPU indirect draw command layout.
 *
 * Used in indirect draw buffers. Layout matches D3D12_DRAW_INDEXED_ARGUMENTS and VkDrawIndexedIndirectCommand.
 */
typedef struct DenOfIz_DrawIndexedIndirectCommand
{
    uint32_t NumIndices;    /**< Number of indices to draw. */
    uint32_t NumInstances;  /**< Number of instances to draw. */
    uint32_t FirstIndex;    /**< Offset into the index buffer. */
    int32_t  VertexOffset;  /**< Value added to vertex index before indexing into vertex buffer. */
    uint32_t FirstInstance; /**< First instance ID. */
} DenOfIz_DrawIndexedIndirectCommand;

/**
 * @brief Describes command list creation parameters.
 */
typedef struct DenOfIz_CommandListDesc
{
    DenOfIz_QueueType QueueType; /**< Queue type this command list will be submitted to. */
} DenOfIz_CommandListDesc;

/**
 * @brief Describes a top-level acceleration structure build operation.
 */
typedef struct DenOfIz_BuildTopLevelASDesc
{
    DenOfIz_TopLevelAS TopLevelAS;          /**< TLAS to build. */
    DenOfIz_Buffer     ScratchBuffer;       /**< Scratch buffer for build. Size from DenOfIz_TopLevelAS_BuildNumBytes. */
    size_t             ScratchBufferOffset; /**< Byte offset into scratch buffer. */
} DenOfIz_BuildTopLevelASDesc;

/**
 * @brief Describes a top-level acceleration structure update operation.
 *
 * Updates instance transforms without full rebuild. TLAS must have been built with ALLOW_UPDATE flag.
 */
typedef struct DenOfIz_UpdateTopLevelASDesc
{
    DenOfIz_TopLevelAS      TopLevelAS;          /**< TLAS to update. */
    DenOfIz_FloatArrayArray Transforms;          /**< New 3x4 transform matrices for instances. */
    DenOfIz_Buffer          ScratchBuffer;       /**< Scratch buffer for update. */
    size_t                  ScratchBufferOffset; /**< Byte offset into scratch buffer. */
} DenOfIz_UpdateTopLevelASDesc;

/**
 * @brief Describes a bottom-level acceleration structure build operation.
 */
typedef struct DenOfIz_BuildBottomLevelASDesc
{
    DenOfIz_BottomLevelAS BottomLevelAS;       /**< BLAS to build. */
    DenOfIz_Buffer        ScratchBuffer;       /**< Scratch buffer for build. Size from DenOfIz_BottomLevelAS_BuildNumBytes. */
    size_t                ScratchBufferOffset; /**< Byte offset into scratch buffer. */
} DenOfIz_BuildBottomLevelASDesc;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Opaque handle to a command list for recording GPU commands.
     *
     * A command list records rendering, compute, and copy commands for later submission
     * to a command queue. Commands are recorded between Begin() and End() calls, and
     * the completed command list is submitted via DenOfIz_CommandQueue_ExecuteCommandLists.
     *
     * Command lists are obtained from a DenOfIz_CommandListPool, which manages allocation
     * and reuse. A single command list cannot be recorded and executed simultaneously.
     *
     * @par Backend Implementations
     * - **DirectX 12**: ID3D12GraphicsCommandList7 + ID3D12CommandAllocator
     * - **Vulkan**: VkCommandBuffer (from VkCommandPool)
     * - **Metal**: MTLCommandBuffer + MTLRenderCommandEncoder / MTLComputeCommandEncoder / MTLBlitCommandEncoder
     * - **WebGPU**: WGPUCommandEncoder + WGPURenderPassEncoder / WGPUComputePassEncoder
     *
     * @par Usage
     * Command lists must follow this lifecycle:
     * 1. Obtain from pool via DenOfIz_CommandListPool_GetCommandLists
     * 2. Call Begin() to start recording
     * 3. Record commands (rendering, compute, copy, barriers)
     * 4. Call End() to finish recording
     * 5. Submit via DenOfIz_CommandQueue_ExecuteCommandLists
     * 6. Wait for completion before reusing (via fence or DenOfIz_CommandQueue_WaitIdle)
     *
     * @par Example
     * @code
     * DenOfIz_CommandListArray commandLists = {0};
     * DenOfIz_CommandListPool_GetCommandLists(pool, &commandLists);
     * DenOfIz_CommandList cmd = commandLists.Elements[0];
     *
     * DenOfIz_CommandList_Begin(cmd);
     *
     * DenOfIz_RenderingAttachmentDesc rtAttachment = {0};
     * rtAttachment.Resource = renderTarget;
     * rtAttachment.LoadOp   = DENOFIZ_LOAD_OP_CLEAR;
     * rtAttachment.StoreOp  = DENOFIZ_STORE_OP_STORE;
     * rtAttachment.ClearColor = (DenOfIz_Float4){0.0f, 0.0f, 0.0f, 1.0f};
     *
     * DenOfIz_RenderingDesc renderingDesc = {0};
     * renderingDesc.RTAttachments.Elements    = &rtAttachment;
     * renderingDesc.RTAttachments.NumElements = 1;
     * renderingDesc.NumLayers                 = 1;
     *
     * DenOfIz_CommandList_BeginRendering(cmd, &renderingDesc);
     * DenOfIz_CommandList_BindViewport(cmd, 0, 0, width, height);
     * DenOfIz_CommandList_BindScissorRect(cmd, 0, 0, width, height);
     * DenOfIz_CommandList_BindPipeline(cmd, pipeline);
     * DenOfIz_CommandList_BindVertexBuffer(cmd, vertexBuffer, 0, stride, 0);
     * DenOfIz_CommandList_Draw(cmd, 3, 1, 0, 0);
     * DenOfIz_CommandList_EndRendering(cmd);
     *
     * DenOfIz_CommandList_End(cmd);
     *
     * DenOfIz_ExecuteCommandListsDesc execDesc = {0};
     * execDesc.CommandLists.Elements    = &cmd;
     * execDesc.CommandLists.NumElements = 1;
     * execDesc.Signal                   = fence;
     * DenOfIz_CommandQueue_ExecuteCommandLists(queue, &execDesc);
     * @endcode
     *
     * @see DenOfIz_CommandListPool
     * @see DenOfIz_CommandQueue_ExecuteCommandLists
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_CommandList )

    typedef struct DenOfIz_CommandListArray
    {
        DenOfIz_CommandList *Elements;
        size_t               NumElements;
    } DenOfIz_CommandListArray;

    /**
     * @brief Begins recording commands to the command list.
     *
     * Resets the command list and prepares it for recording new commands.
     * Must be called before any other command recording functions.
     *
     * @param commandList Valid command list handle.
     *
     * @par Valid Usage
     * - @p commandList must be a valid DenOfIz_CommandList handle
     * - The command list must not be currently recording (no prior Begin without End)
     * - The command list must not be pending execution on a queue
     */
    DZ_API void DenOfIz_CommandList_Begin( DenOfIz_CommandList commandList );

    /**
     * @brief Begins a dynamic rendering pass.
     *
     * Configures render targets and begins recording rendering commands. All draw commands
     * must occur between BeginRendering and EndRendering.
     *
     * @param commandList Valid command list handle.
     * @param renderingDesc Rendering configuration. Must not be NULL.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state (after Begin, before End)
     * - @p renderingDesc must not be NULL
     * - @p renderingDesc->NumLayers must be >= 1
     * - All attachment textures must have RENDER_TARGET or DEPTH_STENCIL usage
     * - Cannot nest BeginRendering calls
     */
    DZ_API void DenOfIz_CommandList_BeginRendering( DenOfIz_CommandList commandList, const DenOfIz_RenderingDesc *renderingDesc );

    /**
     * @brief Ends the current rendering pass.
     *
     * @param commandList Valid command list handle.
     *
     * @par Valid Usage
     * - Must be called after a matching BeginRendering call
     */
    DZ_API void DenOfIz_CommandList_EndRendering( DenOfIz_CommandList commandList );

    /**
     * @brief Ends command recording and prepares the command list for submission.
     *
     * @param commandList Valid command list handle.
     *
     * @par Valid Usage
     * - Must be called after Begin
     * - All opened rendering passes must be closed with EndRendering
     * - All opened debug markers must be closed with EndDebugMarker
     */
    DZ_API void DenOfIz_CommandList_End( DenOfIz_CommandList commandList );

    /**
     * @brief Binds a pipeline for subsequent draw or dispatch commands.
     *
     * @param commandList Valid command list handle.
     * @param pipeline Pipeline to bind (graphics, compute, or ray tracing).
     *
     * @par Valid Usage
     * - @p commandList must be in recording state
     * - @p pipeline must be a valid DenOfIz_Pipeline handle
     * - For graphics pipelines, must be inside a rendering pass
     */
    DZ_API void DenOfIz_CommandList_BindPipeline( DenOfIz_CommandList commandList, DenOfIz_Pipeline pipeline );

    /**
     * @brief Binds a vertex buffer to a specific slot.
     *
     * @param commandList Valid command list handle.
     * @param buffer Vertex buffer to bind.
     * @param offset Byte offset into the buffer.
     * @param stride Byte stride between vertices.
     * @param slot Vertex buffer slot index (0-based).
     *
     * @par Valid Usage
     * - @p commandList must be in recording state inside a rendering pass
     * - @p buffer must have VERTEX_BUFFER descriptor
     * - @p stride must match the vertex layout expected by the pipeline
     */
    DZ_API void DenOfIz_CommandList_BindVertexBuffer( DenOfIz_CommandList commandList, DenOfIz_Buffer buffer, uint64_t offset, uint32_t stride, uint32_t slot );

    /**
     * @brief Binds an index buffer.
     *
     * @param commandList Valid command list handle.
     * @param buffer Index buffer to bind.
     * @param indexType Index element type (UINT16 or UINT32).
     * @param offset Byte offset into the buffer.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state inside a rendering pass
     * - @p buffer must have INDEX_BUFFER descriptor
     */
    DZ_API void DenOfIz_CommandList_BindIndexBuffer( DenOfIz_CommandList commandList, DenOfIz_Buffer buffer, DenOfIz_IndexType indexType, uint64_t offset );

    /**
     * @brief Sets the viewport for rendering.
     *
     * @param commandList Valid command list handle.
     * @param x Viewport left edge in pixels.
     * @param y Viewport top edge in pixels.
     * @param width Viewport width in pixels.
     * @param height Viewport height in pixels.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state inside a rendering pass
     * - @p width and @p height must be > 0
     */
    DZ_API void DenOfIz_CommandList_BindViewport( DenOfIz_CommandList commandList, float x, float y, float width, float height );

    /**
     * @brief Sets the scissor rectangle for rendering.
     *
     * Pixels outside the scissor rectangle are discarded.
     *
     * @param commandList Valid command list handle.
     * @param x Scissor left edge in pixels.
     * @param y Scissor top edge in pixels.
     * @param width Scissor width in pixels.
     * @param height Scissor height in pixels.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state inside a rendering pass
     */
    DZ_API void DenOfIz_CommandList_BindScissorRect( DenOfIz_CommandList commandList, float x, float y, float width, float height );

    /**
     * @brief Binds a resource bind group (descriptor set) to the pipeline.
     *
     * @param commandList Valid command list handle.
     * @param bindGroup Resource bind group containing shader resources.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state
     * - @p bindGroup must be compatible with the currently bound pipeline's root signature
     * - A pipeline must be bound before drawing/dispatching
     */
    DZ_API void DenOfIz_CommandList_BindGroup( DenOfIz_CommandList commandList, DenOfIz_BindGroup bindGroup );

    /**
     * @brief Sets root constants (push constants) directly on the command list.
     *
     * Root constants are small amounts of data (typically <= 64 bytes) passed
     * directly to shaders without buffer indirection. This is the most efficient
     * way to update small per-draw data.
     *
     * @param commandList Valid command list handle.
     * @param binding Root constant binding slot as declared in the root signature.
     * @param data Byte array containing the constant data.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state
     * - A pipeline must be bound before calling this function
     * - @p binding must match a root constant declared in the pipeline's root signature
     * - @p data size must match the size declared in the root signature
     *
     * @par Backend Notes
     * - DirectX 12: Uses SetGraphicsRoot32BitConstants/SetComputeRoot32BitConstants
     * - Vulkan: Uses vkCmdPushConstants
     * - Metal: Encodes into the argument buffer
     * - WebGPU: Not directly supported (logs warning)
     */
    DZ_API void DenOfIz_CommandList_SetRootConstants( DenOfIz_CommandList commandList, uint32_t binding, const DenOfIz_ByteArray *data );

    /**
     * @brief Inserts resource barriers for synchronization.
     *
     * Transitions resources between usage states and ensures proper synchronization
     * between GPU operations.
     *
     * @param commandList Valid command list handle.
     * @param barrier Barrier configuration. Must not be NULL.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state
     * - @p barrier must not be NULL
     * - Resource states must be valid transitions
     */
    DZ_API void DenOfIz_CommandList_PipelineBarrier( DenOfIz_CommandList commandList, const DenOfIz_PipelineBarrierDesc *barrier );

    /**
     * @brief Draws indexed primitives.
     *
     * @param commandList Valid command list handle.
     * @param indexCount Number of indices to draw.
     * @param instanceCount Number of instances to draw.
     * @param firstIndex Offset into the index buffer.
     * @param vertexOffset Value added to vertex index before indexing into vertex buffer.
     * @param firstInstance First instance ID.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state inside a rendering pass
     * - A graphics pipeline must be bound
     * - Vertex and index buffers must be bound
     * - @p indexCount must be > 0
     */
    DZ_API void DenOfIz_CommandList_DrawIndexed( DenOfIz_CommandList commandList, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset,
                                                 uint32_t firstInstance );

    /**
     * @brief Draws non-indexed primitives.
     *
     * @param commandList Valid command list handle.
     * @param vertexCount Number of vertices to draw.
     * @param instanceCount Number of instances to draw.
     * @param firstVertex First vertex index.
     * @param firstInstance First instance ID.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state inside a rendering pass
     * - A graphics pipeline must be bound
     * - A vertex buffer must be bound
     * - @p vertexCount must be > 0
     */
    DZ_API void DenOfIz_CommandList_Draw( DenOfIz_CommandList commandList, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance );

    /**
     * @brief Copies a region from one buffer to another.
     *
     * @param commandList Valid command list handle.
     * @param copyBufferRegionDesc Copy parameters. Must not be NULL.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state, outside a rendering pass
     * - Source buffer must have COPY_SRC usage
     * - Destination buffer must have COPY_DST usage
     * - Copy regions must not overlap if source and destination are the same buffer
     */
    DZ_API void DenOfIz_CommandList_CopyBufferRegion( DenOfIz_CommandList commandList, const DenOfIz_CopyBufferRegionDesc *copyBufferRegionDesc );

    /**
     * @brief Copies a region from one texture to another.
     *
     * @param commandList Valid command list handle.
     * @param copyTextureRegionDesc Copy parameters. Must not be NULL.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state, outside a rendering pass
     * - Source texture must have COPY_SRC usage
     * - Destination texture must have COPY_DST usage
     */
    DZ_API void DenOfIz_CommandList_CopyTextureRegion( DenOfIz_CommandList commandList, const DenOfIz_CopyTextureRegionDesc *copyTextureRegionDesc );

    /**
     * @brief Copies data from a buffer to a texture.
     *
     * @param commandList Valid command list handle.
     * @param copyBufferToTexture Copy parameters. Must not be NULL.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state, outside a rendering pass
     * - Source buffer must have appropriate layout for texture data
     * - Destination texture must have COPY_DST usage
     */
    DZ_API void DenOfIz_CommandList_CopyBufferToTexture( DenOfIz_CommandList commandList, const DenOfIz_CopyBufferToTextureDesc *copyBufferToTexture );

    /**
     * @brief Copies data from a texture to a buffer.
     *
     * @param commandList Valid command list handle.
     * @param copyTextureToBuffer Copy parameters. Must not be NULL.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state, outside a rendering pass
     * - Source texture must have COPY_SRC usage
     * - Destination buffer must have COPY_DST usage
     */
    DZ_API void DenOfIz_CommandList_CopyTextureToBuffer( DenOfIz_CommandList commandList, const DenOfIz_CopyTextureToBufferDesc *copyTextureToBuffer );

    /**
     * @brief Updates a top-level acceleration structure with new transforms.
     *
     * Performs an in-place update of instance transforms without full rebuild.
     *
     * @param commandList Valid command list handle.
     * @param updateDesc Update parameters. Must not be NULL.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state
     * - TLAS must have been built with ALLOW_UPDATE flag
     * - Number of transforms must match number of instances
     */
    DZ_API void DenOfIz_CommandList_UpdateTopLevelAS( DenOfIz_CommandList commandList, const DenOfIz_UpdateTopLevelASDesc *updateDesc );

    /**
     * @brief Builds a top-level acceleration structure.
     *
     * @param commandList Valid command list handle.
     * @param buildTopLevelASDesc Build parameters. Must not be NULL.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state
     * - Scratch buffer must be large enough (query with DenOfIz_TopLevelAS_BuildNumBytes)
     * - All referenced BLAS must be built
     */
    DZ_API void DenOfIz_CommandList_BuildTopLevelAS( DenOfIz_CommandList commandList, const DenOfIz_BuildTopLevelASDesc *buildTopLevelASDesc );

    /**
     * @brief Builds a bottom-level acceleration structure.
     *
     * @param commandList Valid command list handle.
     * @param buildBottomLevelASDesc Build parameters. Must not be NULL.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state
     * - Scratch buffer must be large enough (query with DenOfIz_BottomLevelAS_BuildNumBytes)
     * - Geometry buffers must be in ACCELERATION_STRUCTURE_GEOMETRY state
     */
    DZ_API void DenOfIz_CommandList_BuildBottomLevelAS( DenOfIz_CommandList commandList, const DenOfIz_BuildBottomLevelASDesc *buildBottomLevelASDesc );

    /**
     * @brief Dispatches ray tracing work.
     *
     * @param commandList Valid command list handle.
     * @param dispatchRaysDesc Dispatch parameters. Must not be NULL.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state
     * - A ray tracing pipeline must be bound
     * - Shader binding table must be built
     * - Required resources must be bound
     */
    DZ_API void DenOfIz_CommandList_DispatchRays( DenOfIz_CommandList commandList, const DenOfIz_DispatchRaysDesc *dispatchRaysDesc );

    /**
     * @brief Dispatches compute work.
     *
     * @param commandList Valid command list handle.
     * @param groupCountX Number of thread groups in X dimension.
     * @param groupCountY Number of thread groups in Y dimension.
     * @param groupCountZ Number of thread groups in Z dimension.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state, outside a rendering pass
     * - A compute pipeline must be bound
     * - At least one dimension must be > 0
     */
    DZ_API void DenOfIz_CommandList_Dispatch( DenOfIz_CommandList commandList, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ );

    /**
     * @brief Dispatches mesh shader work.
     *
     * @param commandList Valid command list handle.
     * @param groupCountX Number of mesh shader thread groups in X dimension.
     * @param groupCountY Number of mesh shader thread groups in Y dimension.
     * @param groupCountZ Number of mesh shader thread groups in Z dimension.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state inside a rendering pass
     * - A mesh shader pipeline must be bound
     * - Device must support mesh shaders
     */
    DZ_API void DenOfIz_CommandList_DispatchMesh( DenOfIz_CommandList commandList, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ );

    /**
     * @brief Draws primitives using indirect arguments from a buffer.
     *
     * @param commandList Valid command list handle.
     * @param buffer Buffer containing draw arguments.
     * @param offset Byte offset into the buffer.
     * @param drawCount Number of draw commands.
     * @param stride Byte stride between draw commands.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state inside a rendering pass
     * - @p buffer must have INDIRECT_BUFFER descriptor
     * - Buffer must contain valid draw argument structures
     */
    DZ_API void DenOfIz_CommandList_DrawIndirect( DenOfIz_CommandList commandList, DenOfIz_Buffer buffer, uint64_t offset, uint32_t drawCount, uint32_t stride );

    /**
     * @brief Draws indexed primitives using indirect arguments from a buffer.
     *
     * @param commandList Valid command list handle.
     * @param buffer Buffer containing indexed draw arguments (DenOfIz_DrawIndexedIndirectCommand).
     * @param offset Byte offset into the buffer.
     * @param drawCount Number of draw commands.
     * @param stride Byte stride between draw commands.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state inside a rendering pass
     * - @p buffer must have INDIRECT_BUFFER descriptor
     * - An index buffer must be bound
     */
    DZ_API void DenOfIz_CommandList_DrawIndexedIndirect( DenOfIz_CommandList commandList, DenOfIz_Buffer buffer, uint64_t offset, uint32_t drawCount, uint32_t stride );

    /**
     * @brief Dispatches compute work using indirect arguments from a buffer.
     *
     * @param commandList Valid command list handle.
     * @param buffer Buffer containing dispatch arguments (3 uint32_t: groupCountX, groupCountY, groupCountZ).
     * @param offset Byte offset into the buffer.
     *
     * @par Valid Usage
     * - @p commandList must be in recording state, outside a rendering pass
     * - @p buffer must have INDIRECT_BUFFER descriptor
     * - A compute pipeline must be bound
     */
    DZ_API void DenOfIz_CommandList_DispatchIndirect( DenOfIz_CommandList commandList, DenOfIz_Buffer buffer, uint64_t offset );

    /**
     * @brief Begins a named debug marker region.
     *
     * Debug markers appear in graphics debugging tools (PIX, RenderDoc, etc.).
     * Markers can be nested.
     *
     * @param commandList Valid command list handle.
     * @param r Red component of marker color (0.0-1.0).
     * @param g Green component of marker color (0.0-1.0).
     * @param b Blue component of marker color (0.0-1.0).
     * @param name Marker name for display in debug tools.
     */
    DZ_API void DenOfIz_CommandList_BeginDebugMarker( DenOfIz_CommandList commandList, float r, float g, float b, DenOfIz_StringView name );

    /**
     * @brief Ends the current debug marker region.
     *
     * @param commandList Valid command list handle.
     *
     * @par Valid Usage
     * - Must be called after a matching BeginDebugMarker
     */
    DZ_API void DenOfIz_CommandList_EndDebugMarker( DenOfIz_CommandList commandList );

    /**
     * @brief Inserts a single debug marker point.
     *
     * Unlike BeginDebugMarker/EndDebugMarker, this marks a single point in time.
     *
     * @param commandList Valid command list handle.
     * @param r Red component of marker color (0.0-1.0).
     * @param g Green component of marker color (0.0-1.0).
     * @param b Blue component of marker color (0.0-1.0).
     * @param name Marker name for display in debug tools.
     */
    DZ_API void DenOfIz_CommandList_InsertDebugMarker( DenOfIz_CommandList commandList, float r, float g, float b, DenOfIz_StringView name );

    /**
     * @brief Begins a GPU query.
     *
     * @param commandList Valid command list handle.
     * @param queryPool Query pool to use.
     * @param queryDesc Query index and parameters.
     *
     * @par Valid Usage
     * - @p queryDesc->Index must be < queryPool's NumQueries
     * - For timestamp queries, BeginQuery writes the start timestamp
     */
    DZ_API void DenOfIz_CommandList_BeginQuery( DenOfIz_CommandList commandList, DenOfIz_QueryPool queryPool, const DenOfIz_QueryDesc *queryDesc );

    /**
     * @brief Ends a GPU query.
     *
     * @param commandList Valid command list handle.
     * @param queryPool Query pool to use.
     * @param queryDesc Query index and parameters.
     *
     * @par Valid Usage
     * - Must be called after a matching BeginQuery (except for timestamp queries)
     */
    DZ_API void DenOfIz_CommandList_EndQuery( DenOfIz_CommandList commandList, DenOfIz_QueryPool queryPool, const DenOfIz_QueryDesc *queryDesc );

    /**
     * @brief Resolves query results to make them available for reading.
     *
     * @param commandList Valid command list handle.
     * @param queryPool Query pool containing queries to resolve.
     * @param startQuery First query index to resolve.
     * @param queryCount Number of queries to resolve.
     *
     * @par Valid Usage
     * - @p startQuery + @p queryCount must be <= queryPool's NumQueries
     * - Queries must have been ended before resolving
     */
    DZ_API void DenOfIz_CommandList_ResolveQuery( DenOfIz_CommandList commandList, DenOfIz_QueryPool queryPool, uint32_t startQuery, uint32_t queryCount );

    /**
     * @brief Resets queries for reuse.
     *
     * @param commandList Valid command list handle.
     * @param queryPool Query pool containing queries to reset.
     * @param startQuery First query index to reset.
     * @param queryCount Number of queries to reset.
     *
     * @par Valid Usage
     * - @p startQuery + @p queryCount must be <= queryPool's NumQueries
     * - Queries must not be in use when reset
     */
    DZ_API void DenOfIz_CommandList_ResetQuery( DenOfIz_CommandList commandList, DenOfIz_QueryPool queryPool, uint32_t startQuery, uint32_t queryCount );

    /**
     * @brief Gets the queue type this command list is associated with.
     *
     * @param commandList Valid command list handle.
     * @param[out] outQueueType Receives the queue type.
     */
    DZ_API void DenOfIz_CommandList_GetQueueType( DenOfIz_CommandList commandList, DenOfIz_QueueType *outQueueType );

    /**
     * @brief Destroys the command list and frees resources.
     *
     * @param commandList Command list handle to destroy.
     *
     * @par Valid Usage
     * - @p commandList must not be currently executing on a queue
     * - After destruction, the handle is invalid
     *
     * @note Command lists are typically managed by their parent pool and destroyed
     *       when the pool is destroyed. Direct destruction is rarely needed.
     */
    DZ_API void DenOfIz_CommandList_Destroy( DenOfIz_CommandList commandList );

#ifdef __cplusplus
}
#endif
