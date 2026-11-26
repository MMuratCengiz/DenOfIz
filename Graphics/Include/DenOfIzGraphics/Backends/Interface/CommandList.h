/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include "BindGroup.h"
#include "DenOfIzGraphics/Handle.h"
#include "PipelineBarrierDesc.h"
#include "QueryPool.h"
#include "RayTracing/ShaderBindingTable.h"

typedef struct DenOfIz_RenderingAttachmentDesc
{
    DenOfIz_LoadOp  LoadOp;
    DenOfIz_StoreOp StoreOp; /**< DX12 always stores. */
    DenOfIz_Texture Resource;
    DenOfIz_Float4  ClearColor;        /**< RGBA, used when LoadOp is CLEAR. */
    DenOfIz_Float2  ClearDepthStencil; /**< Depth in X, stencil in Y, used when LoadOp is CLEAR. */
} DenOfIz_RenderingAttachmentDesc;

typedef struct DenOfIz_RenderingAttachmentDescArray
{
    DenOfIz_RenderingAttachmentDesc *Elements;
    uint32_t                         NumElements;
} DenOfIz_RenderingAttachmentDescArray;

/**
 * The render area, in pixels, is used by Vulkan only, DX12 and Metal render to the whole attachments. On Vulkan a RenderAreaHeight of 0
 * covers the attachments, and the area is clamped to the smallest attachment.
 */
typedef struct DenOfIz_RenderingDesc
{
    DenOfIz_ViewMask                     ViewMask; /**< Ignored, Vulkan multiview is set with the graphics pipeline's ViewMask. */
    DenOfIz_RenderingAttachmentDesc      DepthAttachment;
    DenOfIz_RenderingAttachmentDesc      StencilAttachment; /**< Null Resource: a depth format with stencil uses DepthAttachment's ops and clear value for stencil. */
    float                                RenderAreaWidth;
    float                                RenderAreaHeight;
    float                                RenderAreaOffsetX;
    float                                RenderAreaOffsetY;
    uint32_t                             NumLayers; /**< 0 is 1. Ignored on DX12. */
    DenOfIz_RenderingAttachmentDescArray RTAttachments;
} DenOfIz_RenderingDesc;

typedef struct DenOfIz_CopyBufferRegionDesc
{
    DenOfIz_Buffer DstBuffer;
    uint64_t       DstOffset;
    DenOfIz_Buffer SrcBuffer;
    uint64_t       SrcOffset;
    uint64_t       NumBytes;
} DenOfIz_CopyBufferRegionDesc;

/** Offsets and sizes are in texels of the mip level. */
typedef struct DenOfIz_CopyTextureRegionDesc
{
    DenOfIz_Texture SrcTexture;
    DenOfIz_Texture DstTexture;
    uint32_t        SrcX;
    uint32_t        SrcY;
    uint32_t        SrcZ;
    uint32_t        DstX;
    uint32_t        DstY;
    uint32_t        DstZ;
    uint32_t        Width;
    uint32_t        Height;
    uint32_t        Depth; /**< At least 1, 1 for 2D textures. */
    uint32_t        SrcMipLevel;
    uint32_t        DstMipLevel;
    uint32_t        SrcArrayLayer;
    uint32_t        DstArrayLayer;
} DenOfIz_CopyTextureRegionDesc;

/**
 * Uploads one whole mip level of one array layer. The source rows (rows of texel blocks for compressed formats) start every
 * Align(row size, DenOfIz_DeviceConstants.BufferTextureRowAlignment) bytes from SrcOffset, which must be a multiple of
 * BufferTextureAlignment. RowPitch and NumRows are not used.
 */
typedef struct DenOfIz_CopyBufferToTextureDesc
{
    DenOfIz_Texture DstTexture;
    DenOfIz_Buffer  SrcBuffer;
    size_t          SrcOffset;
    uint32_t        DstX;
    uint32_t        DstY;
    uint32_t        DstZ;
    DenOfIz_Format  Format; /**< Format of the source texels, gives the row size. Normally the texture's format. */
    uint32_t        MipLevel;
    uint32_t        ArrayLayer;
    uint32_t        RowPitch;
    uint32_t        NumRows;
} DenOfIz_CopyBufferToTextureDesc;

/**
 * Reads back one whole mip level of one array layer to DstOffset. The destination row layout differs per backend: DX12 aligns rows to
 * DenOfIz_DeviceConstants.BufferTextureRowAlignment, Vulkan packs them tightly, Metal uses RowPitch and NumRows.
 */
typedef struct DenOfIz_CopyTextureToBufferDesc
{
    DenOfIz_Buffer  DstBuffer;
    DenOfIz_Texture SrcTexture;
    uint32_t        DstOffset;
    uint32_t        SrcX;
    uint32_t        SrcY;
    uint32_t        SrcZ;
    DenOfIz_Format  Format;
    uint32_t        MipLevel;
    uint32_t        ArrayLayer;
    uint32_t        RowPitch;
    uint32_t        NumRows;
} DenOfIz_CopyTextureToBufferDesc;

typedef struct DenOfIz_DispatchRaysDesc
{
    uint32_t                   Width;
    uint32_t                   Height;
    uint32_t                   Depth;
    DenOfIz_ShaderBindingTable ShaderBindingTable;
} DenOfIz_DispatchRaysDesc;

/** One DrawIndexedIndirect command, laid out like D3D12_DRAW_INDEXED_ARGUMENTS, VkDrawIndexedIndirectCommand and MTLDrawIndexedPrimitivesIndirectArguments. */
typedef struct DenOfIz_DrawIndexedIndirectCommand
{
    uint32_t NumIndices;
    uint32_t NumInstances;
    uint32_t FirstIndex;
    int32_t  VertexOffset; /**< Added to each index before fetching the vertex. */
    uint32_t FirstInstance;
} DenOfIz_DrawIndexedIndirectCommand;

typedef struct DenOfIz_CommandListDesc
{
    DenOfIz_QueueType QueueType;
} DenOfIz_CommandListDesc;

typedef struct DenOfIz_BuildTopLevelASDesc
{
    DenOfIz_TopLevelAS TopLevelAS;
    DenOfIz_Buffer     ScratchBuffer; /**< At least DenOfIz_TopLevelAS_BuildNumBytes bytes from ScratchBufferOffset. */
    size_t             ScratchBufferOffset;
} DenOfIz_BuildTopLevelASDesc;

/**
 * Refits a built TLAS to the instance transforms last set with DenOfIz_TopLevelAS_UpdateInstanceTransforms. The TLAS must have been
 * created with DENOFIZ_AS_BUILD_ALLOW_UPDATE_BIT. Metal rebuilds it instead.
 */
typedef struct DenOfIz_UpdateTopLevelASDesc
{
    DenOfIz_TopLevelAS      TopLevelAS;
    DenOfIz_FloatArrayArray Transforms; /**< Ignored, see DenOfIz_TopLevelAS_UpdateInstanceTransforms. */
    DenOfIz_Buffer          ScratchBuffer;
    size_t                  ScratchBufferOffset;
} DenOfIz_UpdateTopLevelASDesc;

typedef struct DenOfIz_BuildBottomLevelASDesc
{
    DenOfIz_BottomLevelAS BottomLevelAS;
    DenOfIz_Buffer        ScratchBuffer; /**< At least DenOfIz_BottomLevelAS_BuildNumBytes bytes from ScratchBufferOffset. */
    size_t                ScratchBufferOffset;
} DenOfIz_BuildBottomLevelASDesc;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * Records commands for one queue type. Command lists come from, and are owned by, a DenOfIz_CommandListPool.
     *
     * Record between Begin and End, then submit with DenOfIz_CommandQueue_ExecuteCommandLists. Begin resets the list: its previous
     * submission must have finished on the GPU (fence or WaitIdle).
     *
     * Draws go between BeginRendering and EndRendering. Copies, dispatches, ray tracing, acceleration structure builds and ResetQuery go
     * outside. On Metal a rendering pass is a render encoder: set vertex buffers, viewport and scissor after each BeginRendering, and a copy,
     * dispatch or compute pipeline bind inside the pass ends it.
     *
     * Bind groups and root constants are applied at the next draw or dispatch, matched to the bound pipeline by register space and binding,
     * so they can be set before or after BindPipeline and stay set across pipeline changes.
     *
     * Copy sources and destinations need the COPY_SRC and COPY_DST usage bits, indirect argument buffers DENOFIZ_BUFFER_USAGE_INDIRECT_BIT.
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_CommandList )

    typedef struct DenOfIz_CommandListArray
    {
        DenOfIz_CommandList *Elements;
        size_t               NumElements;
    } DenOfIz_CommandListArray;

    DZ_API void DenOfIz_CommandList_Begin( DenOfIz_CommandList commandList );
    DZ_API void DenOfIz_CommandList_BeginRendering( DenOfIz_CommandList commandList, const DenOfIz_RenderingDesc *renderingDesc );
    DZ_API void DenOfIz_CommandList_EndRendering( DenOfIz_CommandList commandList );
    DZ_API void DenOfIz_CommandList_End( DenOfIz_CommandList commandList );
    DZ_API void DenOfIz_CommandList_BindPipeline( DenOfIz_CommandList commandList, DenOfIz_Pipeline pipeline );

    /** A stride of 0 uses the input layout's stride. Only DX12 reads stride, Vulkan and Metal always use the pipeline's input layout. */
    DZ_API void DenOfIz_CommandList_BindVertexBuffer( DenOfIz_CommandList commandList, DenOfIz_Buffer buffer, uint64_t offset, uint32_t stride, uint32_t slot );
    DZ_API void DenOfIz_CommandList_BindIndexBuffer( DenOfIz_CommandList commandList, DenOfIz_Buffer buffer, DenOfIz_IndexType indexType, uint64_t offset );

    /** Pixels from the top left corner of the render target, depth range 0 to 1, on every backend. Ignored when width or height is not positive. */
    DZ_API void DenOfIz_CommandList_BindViewport( DenOfIz_CommandList commandList, float x, float y, float width, float height );

    /** Pixels from the top left corner of the render target. Ignored when width or height is not positive. */
    DZ_API void DenOfIz_CommandList_BindScissorRect( DenOfIz_CommandList commandList, float x, float y, float width, float height );

    /** Replaces the bind group bound at the same register space. */
    DZ_API void DenOfIz_CommandList_BindGroup( DenOfIz_CommandList commandList, DenOfIz_BindGroup bindGroup );

    /**
     * Copies data for the root constant whose Binding is binding in the pipeline's DenOfIz_RootSignatureDesc.RootConstants. The size must be
     * a multiple of 4 and at most its NumBytes. DX12 applies root constants to graphics and compute pipelines only.
     */
    DZ_API void DenOfIz_CommandList_SetRootConstants( DenOfIz_CommandList commandList, uint32_t binding, const DenOfIz_ByteArray *data );

    /** No-op on Metal. */
    DZ_API void DenOfIz_CommandList_PipelineBarrier( DenOfIz_CommandList commandList, const DenOfIz_PipelineBarrierDesc *barrier );
    DZ_API void DenOfIz_CommandList_DrawIndexed( DenOfIz_CommandList commandList, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset,
                                                 uint32_t firstInstance );
    DZ_API void DenOfIz_CommandList_Draw( DenOfIz_CommandList commandList, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance );
    DZ_API void DenOfIz_CommandList_CopyBufferRegion( DenOfIz_CommandList commandList, const DenOfIz_CopyBufferRegionDesc *copyBufferRegionDesc );
    DZ_API void DenOfIz_CommandList_CopyTextureRegion( DenOfIz_CommandList commandList, const DenOfIz_CopyTextureRegionDesc *copyTextureRegionDesc );
    DZ_API void DenOfIz_CommandList_CopyBufferToTexture( DenOfIz_CommandList commandList, const DenOfIz_CopyBufferToTextureDesc *copyBufferToTexture );
    DZ_API void DenOfIz_CommandList_CopyTextureToBuffer( DenOfIz_CommandList commandList, const DenOfIz_CopyTextureToBufferDesc *copyTextureToBuffer );
    DZ_API void DenOfIz_CommandList_UpdateTopLevelAS( DenOfIz_CommandList commandList, const DenOfIz_UpdateTopLevelASDesc *updateDesc );
    DZ_API void DenOfIz_CommandList_BuildTopLevelAS( DenOfIz_CommandList commandList, const DenOfIz_BuildTopLevelASDesc *buildTopLevelASDesc );
    DZ_API void DenOfIz_CommandList_BuildBottomLevelAS( DenOfIz_CommandList commandList, const DenOfIz_BuildBottomLevelASDesc *buildBottomLevelASDesc );
    DZ_API void DenOfIz_CommandList_DispatchRays( DenOfIz_CommandList commandList, const DenOfIz_DispatchRaysDesc *dispatchRaysDesc );
    DZ_API void DenOfIz_CommandList_Dispatch( DenOfIz_CommandList commandList, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ );
    DZ_API void DenOfIz_CommandList_DispatchMesh( DenOfIz_CommandList commandList, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ );

    /**
     * Reads drawCount commands of four uint32 (vertex count, instance count, first vertex, first instance) from offset, stride bytes apart.
     * A stride of 0 means tightly packed on DX12 and Vulkan only, pass the real stride for Metal.
     */
    DZ_API void DenOfIz_CommandList_DrawIndirect( DenOfIz_CommandList commandList, DenOfIz_Buffer buffer, uint64_t offset, uint32_t drawCount, uint32_t stride );

    /** Like DrawIndirect, with DenOfIz_DrawIndexedIndirectCommand commands. */
    DZ_API void DenOfIz_CommandList_DrawIndexedIndirect( DenOfIz_CommandList commandList, DenOfIz_Buffer buffer, uint64_t offset, uint32_t drawCount, uint32_t stride );

    /** Reads three uint32 group counts (X, Y, Z) from offset. */
    DZ_API void DenOfIz_CommandList_DispatchIndirect( DenOfIz_CommandList commandList, DenOfIz_Buffer buffer, uint64_t offset );

    /**
     * Markers nest, the color components are 0 to 1 and name must be null terminated. Recorded by DX12 only in debug builds with PIX and by
     * Vulkan only with EnableValidationLayers. Metal ignores the color, and a marker must begin and end within one rendering pass or one
     * run of copy or compute work.
     */
    DZ_API void DenOfIz_CommandList_BeginDebugMarker( DenOfIz_CommandList commandList, float r, float g, float b, DenOfIz_StringView name );
    DZ_API void DenOfIz_CommandList_EndDebugMarker( DenOfIz_CommandList commandList );

    /** A single point, see BeginDebugMarker. */
    DZ_API void DenOfIz_CommandList_InsertDebugMarker( DenOfIz_CommandList commandList, float r, float g, float b, DenOfIz_StringView name );

    /**
     * A timestamp query holds a pair: BeginQuery and EndQuery with the same Index write its begin and end timestamps. Occlusion and pipeline
     * statistics queries count the work recorded between them. On Metal occlusion queries work inside a rendering pass only, a timestamp
     * recorded on a graphics queue before any rendering pass, copy or dispatch is dropped, and pipeline statistics are not recorded.
     */
    DZ_API void DenOfIz_CommandList_BeginQuery( DenOfIz_CommandList commandList, DenOfIz_QueryPool queryPool, const DenOfIz_QueryDesc *queryDesc );
    DZ_API void DenOfIz_CommandList_EndQuery( DenOfIz_CommandList commandList, DenOfIz_QueryPool queryPool, const DenOfIz_QueryDesc *queryDesc );

    /** Makes results readable with DenOfIz_QueryPool_GetQueryData once the GPU finished. Needed on DX12, a no-op on Vulkan and Metal. */
    DZ_API void DenOfIz_CommandList_ResolveQuery( DenOfIz_CommandList commandList, DenOfIz_QueryPool queryPool, uint32_t startQuery, uint32_t queryCount );

    /** Needed on Vulkan before queries are first used and before each reuse, a no-op on DX12 and Metal. */
    DZ_API void DenOfIz_CommandList_ResetQuery( DenOfIz_CommandList commandList, DenOfIz_QueryPool queryPool, uint32_t startQuery, uint32_t queryCount );
    DZ_API void DenOfIz_CommandList_GetQueueType( DenOfIz_CommandList commandList, DenOfIz_QueueType *outQueueType );

    /** A no-op: every command list is owned by its pool and freed by DenOfIz_CommandListPool_Destroy. */
    DZ_API void DenOfIz_CommandList_Destroy( DenOfIz_CommandList commandList );

#ifdef __cplusplus
}
#endif
