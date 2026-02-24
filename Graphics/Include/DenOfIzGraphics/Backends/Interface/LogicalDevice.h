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
#include "Buffer.h"
#include "CommandListPool.h"
#include "CommandQueue.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Result.h"
#include "Fence.h"
#include "InputLayout.h"
#include "Pipeline.h"
#include "PipelineCache.h"
#include "QueryPool.h"
#include "RayTracing/BottomLevelAS.h"
#include "RayTracing/LocalRootSignature.h"
#include "RayTracing/ShaderBindingTable.h"
#include "RayTracing/ShaderLocalData.h"
#include "RayTracing/TopLevelAS.h"
#include "RootSignature.h"
#include "Semaphore.h"
#include "SwapChain.h"
#include "Texture.h"

#ifdef _WIN32
#undef CreateSemaphore
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DenOfIz_LogicalDeviceDesc
    {
        bool AutoSync;
        bool EnableValidationLayers;
    } DenOfIz_LogicalDeviceDesc;

    DENOFIZ_DEFINE_HANDLE( DenOfIz_LogicalDevice )

    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateDevice( DenOfIz_LogicalDevice device, const DenOfIz_LogicalDeviceDesc *desc );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_ListPhysicalDevices( DenOfIz_LogicalDevice device, DenOfIz_PhysicalDeviceArray *outDevices );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_LoadPhysicalDevice( DenOfIz_LogicalDevice device, const DenOfIz_PhysicalDevice *physicalDevice );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_DeviceInfo( DenOfIz_LogicalDevice device, DenOfIz_PhysicalDevice *outDevice );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_IsDeviceLost( DenOfIz_LogicalDevice device, bool *outLost );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_WaitIdle( DenOfIz_LogicalDevice device );

    /**
     * @brief Creates a command queue for submitting work to the GPU.
     *
     * @param device Valid logical device handle.
     * @param desc Queue configuration. Must not be NULL.
     * @param[out] outQueue Receives the created command queue handle.
     * @return DENOFIZ_SUCCESS on success, error code otherwise.
     *
     * @par Valid Usage
     * - @p device must be a valid DenOfIz_LogicalDevice handle
     * - @p desc must not be NULL
     * - @p outQueue must not be NULL
     * - If RequirePresentationSupport is true, queue must support swapchain present
     *
     * @par Example
     * @code
     * DenOfIz_CommandQueueDesc queueDesc = {0};
     * queueDesc.QueueType = DENOFIZ_QUEUE_TYPE_GRAPHICS;
     * queueDesc.Priority  = DENOFIZ_QUEUE_PRIORITY_NORMAL;
     * queueDesc.Flags.RequirePresentationSupport = true;
     *
     * DenOfIz_CommandQueue graphicsQueue;
     * DenOfIz_LogicalDevice_CreateCommandQueue(device, &queueDesc, &graphicsQueue);
     * @endcode
     *
     * @see DenOfIz_CommandQueueDesc
     * @see DenOfIz_CommandQueue_Destroy
     */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateCommandQueue( DenOfIz_LogicalDevice device, const DenOfIz_CommandQueueDesc *desc, DenOfIz_CommandQueue *outQueue );

    /**
     * @brief Creates a command list pool that allocates and manages command lists.
     *
     * A command list pool allocates a fixed number of command lists associated with a queue.
     * Command lists from the pool are reused across frames after synchronization.
     *
     * @param device Valid logical device handle.
     * @param desc Pool configuration. Must not be NULL.
     * @param[out] outPool Receives the created pool handle.
     * @return DENOFIZ_SUCCESS on success, error code otherwise.
     *
     * @par Valid Usage
     * - @p device must be a valid DenOfIz_LogicalDevice handle
     * - @p desc must not be NULL
     * - @p desc->CommandQueue must be a valid command queue
     * - @p desc->NumCommandLists must be > 0
     * - @p outPool must not be NULL
     *
     * @par Example
     * @code
     * DenOfIz_CommandListPoolDesc poolDesc = {0};
     * poolDesc.CommandQueue    = graphicsQueue;
     * poolDesc.NumCommandLists = 3;
     *
     * DenOfIz_CommandListPool pool;
     * DenOfIz_LogicalDevice_CreateCommandListPool(device, &poolDesc, &pool);
     *
     * DenOfIz_CommandListArray commandLists = {0};
     * DenOfIz_CommandListPool_GetCommandLists(pool, &commandLists);
     *
     * DenOfIz_CommandList cmd = commandLists.Elements[frameIndex];
     * DenOfIz_CommandList_Begin(cmd);
     * // ... record commands ...
     * DenOfIz_CommandList_End(cmd);
     * @endcode
     *
     * @see DenOfIz_CommandListPoolDesc
     * @see DenOfIz_CommandListPool_GetCommandLists
     * @see DenOfIz_CommandListPool_Destroy
     */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateCommandListPool( DenOfIz_LogicalDevice device, const DenOfIz_CommandListPoolDesc *desc, DenOfIz_CommandListPool *outPool );

    /**
     * @brief Creates a GPU pipeline from the provided descriptor.
     *
     * Creates a graphics, compute, ray tracing, or mesh shader pipeline based on
     * the BindPoint specified in the descriptor. The pipeline encapsulates shaders
     * and fixed-function state.
     *
     * @param device Valid logical device handle.
     * @param desc Pipeline configuration. Must not be NULL.
     * @param[out] outPipeline Receives the created pipeline handle.
     * @return DENOFIZ_SUCCESS on success, error code otherwise.
     *
     * @par Valid Usage
     * - @p device must be a valid DenOfIz_LogicalDevice handle
     * - @p desc must not be NULL
     * - @p desc->RootSignature must be valid
     * - @p desc->ShaderProgram must be valid and compiled
     * - For GRAPHICS: InputLayout and Graphics.RenderTargets must be configured
     * - For RAYTRACING: RayTracing.HitGroups must have valid hit group definitions
     * - @p outPipeline must not be NULL
     *
     * @par Example
     * @code
     * // Graphics pipeline
     * DenOfIz_PipelineDesc pipelineDesc = {0};
     * pipelineDesc.BindPoint      = DENOFIZ_BIND_POINT_GRAPHICS;
     * pipelineDesc.ShaderProgram  = program;
     * pipelineDesc.RootSignature  = rootSignature;
     * pipelineDesc.InputLayout    = inputLayout;
     *
     * DenOfIz_RenderTargetDesc rtDesc = {
     *     .Blend = { .RenderTargetWriteMask = 0x0F },
     *     .Format = DENOFIZ_FORMAT_B8G8R8A8_UNORM
     * };
     * pipelineDesc.Graphics.RenderTargets.Elements    = &rtDesc;
     * pipelineDesc.Graphics.RenderTargets.NumElements = 1;
     * pipelineDesc.Graphics.CullMode          = DENOFIZ_CULL_MODE_BACK_FACE;
     * pipelineDesc.Graphics.PrimitiveTopology = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;
     *
     * DenOfIz_Pipeline pipeline;
     * DenOfIz_LogicalDevice_CreatePipeline(device, &pipelineDesc, &pipeline);
     * @endcode
     *
     * @see DenOfIz_PipelineDesc
     * @see DenOfIz_Pipeline
     * @see DenOfIz_Pipeline_Destroy
     */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreatePipeline( DenOfIz_LogicalDevice device, const DenOfIz_PipelineDesc *desc, DenOfIz_Pipeline *outPipeline );

    /**
     * @brief Creates a pipeline cache for storing compiled pipeline state.
     *
     * Pipeline caches persist compiled shader bytecode across sessions, reducing
     * pipeline creation time on subsequent launches. Pass the cache to pipelines
     * via DenOfIz_PipelineDesc.PipelineCache.
     *
     * @param device Valid logical device handle.
     * @param desc Cache configuration. Use empty desc for new cache, or provide
     *             previously saved data.
     * @param[out] outCache Receives the created cache handle.
     * @return DENOFIZ_SUCCESS on success, error code otherwise.
     *
     * @par Valid Usage
     * - @p device must be a valid DenOfIz_LogicalDevice handle
     * - @p desc must not be NULL
     * - If @p desc->Data is non-NULL, @p desc->DataSize must match the data size
     * - @p outCache must not be NULL
     *
     * @par Example
     * @code
     * // Create empty cache (first run) or load from file
     * DenOfIz_PipelineCacheDesc cacheDesc = {0};
     * // Optional: cacheDesc.Data = loadedData; cacheDesc.DataSize = loadedSize;
     *
     * DenOfIz_PipelineCache cache;
     * DenOfIz_LogicalDevice_CreatePipelineCache(device, &cacheDesc, &cache);
     *
     * // Use with pipeline creation
     * DenOfIz_PipelineDesc pipelineDesc = {0};
     * pipelineDesc.PipelineCache = cache;
     * // ... configure pipeline ...
     * @endcode
     *
     * @see DenOfIz_PipelineCache
     * @see DenOfIz_PipelineCache_GetData
     */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreatePipelineCache( DenOfIz_LogicalDevice device, const DenOfIz_PipelineCacheDesc *desc, DenOfIz_PipelineCache *outCache );

    /**
     * @brief Creates a swap chain for presenting rendered frames to a window.
     *
     * @param device Valid logical device handle.
     * @param desc Swap chain configuration. Must not be NULL.
     * @param[out] outSwapChain Receives the created swap chain handle.
     * @return DENOFIZ_SUCCESS on success, error code otherwise.
     *
     * @par Valid Usage
     * - @p device must be a valid DenOfIz_LogicalDevice handle
     * - @p desc must not be NULL
     * - @p desc->WindowHandle must be a valid window handle
     * - @p desc->Width and @p desc->Height must be > 0
     * - @p desc->NumBuffers should be 2 or 3 (double/triple buffering)
     * - @p desc->CommandQueue must support presentation (RequirePresentationSupport flag)
     * - @p outSwapChain must not be NULL
     *
     * @par Example
     * @code
     * DenOfIz_SwapChainDesc swapChainDesc = {0};
     * swapChainDesc.WindowHandle      = windowHandle;
     * swapChainDesc.Width             = 1920;
     * swapChainDesc.Height            = 1080;
     * swapChainDesc.NumBuffers        = 3;
     * swapChainDesc.BackBufferFormat  = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
     * swapChainDesc.DepthBufferFormat = DENOFIZ_FORMAT_D32_FLOAT;
     * swapChainDesc.CommandQueue      = graphicsQueue;
     * swapChainDesc.AllowTearing      = true;
     * swapChainDesc.ColorSpace        = DENOFIZ_COLOR_SPACE_SRGB;
     * swapChainDesc.SampleCount       = DENOFIZ_MSAA_SAMPLE_COUNT_1;
     *
     * DenOfIz_SwapChain swapChain;
     * DenOfIz_LogicalDevice_CreateSwapChain(device, &swapChainDesc, &swapChain);
     * @endcode
     *
     * @see DenOfIz_SwapChainDesc
     * @see DenOfIz_SwapChain_Destroy
     */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateSwapChain( DenOfIz_LogicalDevice device, const DenOfIz_SwapChainDesc *desc, DenOfIz_SwapChain *outSwapChain );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateRootSignature( DenOfIz_LogicalDevice device, const DenOfIz_RootSignatureDesc *desc, DenOfIz_RootSignature *outSignature );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateBindGroupLayout( DenOfIz_LogicalDevice device, const DenOfIz_BindGroupLayoutDesc *desc, DenOfIz_BindGroupLayout *outLayout );

    /**
     * @brief Creates a vertex input layout from a layout descriptor.
     *
     * Defines how vertex data is read from vertex buffers and mapped to shader inputs.
     * The layout is typically obtained from shader reflection.
     *
     * @param device Valid logical device handle.
     * @param desc Input layout configuration. Must not be NULL.
     * @param[out] outLayout Receives the created input layout handle.
     * @return DENOFIZ_SUCCESS on success, error code otherwise.
     *
     * @par Valid Usage
     * - @p device must be a valid DenOfIz_LogicalDevice handle
     * - @p desc must not be NULL
     * - @p outLayout must not be NULL
     * - Input groups must have valid element arrays if NumElements > 0
     *
     * @par Example
     * @code
     * // From shader reflection (recommended):
     * DenOfIz_ShaderReflectDesc reflectDesc = {0};
     * DenOfIz_ShaderProgram_Reflect(program, &reflectDesc);
     *
     * DenOfIz_InputLayout inputLayout;
     * DenOfIz_LogicalDevice_CreateInputLayout(device, &reflectDesc.InputLayout, &inputLayout);
     *
     * // Use in pipeline creation:
     * DenOfIz_PipelineDesc pipelineDesc = {0};
     * pipelineDesc.InputLayout   = inputLayout;
     * pipelineDesc.RootSignature = rootSignature;
     * pipelineDesc.ShaderProgram = program;
     * DenOfIz_LogicalDevice_CreatePipeline(device, &pipelineDesc, &pipeline);
     * @endcode
     *
     * @see DenOfIz_InputLayout
     * @see DenOfIz_InputLayoutDesc
     * @see DenOfIz_InputLayout_Destroy
     */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateInputLayout( DenOfIz_LogicalDevice device, const DenOfIz_InputLayoutDesc *desc, DenOfIz_InputLayout *outLayout );

    /**
     * @brief Creates a resource bind group for binding shader resources.
     *
     * A bind group manages shader resource bindings (buffers, textures, samplers)
     * for a specific register space. Multiple bind groups can be created targeting
     * different spaces for modular resource management.
     *
     * @param device Valid logical device handle.
     * @param desc Bind group configuration. Must not be NULL.
     * @param[out] outGroup Receives the created bind group handle.
     * @return DENOFIZ_SUCCESS on success, error code otherwise.
     *
     * @par Valid Usage
     * - @p device must be a valid DenOfIz_LogicalDevice handle
     * - @p desc must not be NULL
     * - @p desc->RootSignature must be a valid root signature handle
     * - @p desc->RegisterSpace must be valid for the root signature
     * - @p outGroup must not be NULL
     *
     * @par Example
     * @code
     * // Create bind group for per-frame resources (space 0)
     * DenOfIz_BindGroupDesc bindGroupDesc = {0};
     * bindGroupDesc.RootSignature = rootSignature;
     * bindGroupDesc.RegisterSpace = 0;
     *
     * DenOfIz_BindGroup perFrameBindGroup;
     * DenOfIz_LogicalDevice_CreateBindGroup(device, &bindGroupDesc, &perFrameBindGroup);
     *
     * // Bind resources
     * DenOfIz_BindGroup_BeginUpdate(perFrameBindGroup);
     * DenOfIz_BindGroup_Cbv(perFrameBindGroup, 0, viewProjBuffer);
     * DenOfIz_BindGroup_EndUpdate(perFrameBindGroup);
     *
     * // Create another bind group for per-material resources (space 1)
     * bindGroupDesc.RegisterSpace = 1;
     * DenOfIz_BindGroup perMaterialBindGroup;
     * DenOfIz_LogicalDevice_CreateBindGroup(device, &bindGroupDesc, &perMaterialBindGroup);
     * @endcode
     *
     * @see DenOfIz_BindGroupDesc
     * @see DenOfIz_BindGroup
     * @see DenOfIz_BindGroup_Destroy
     */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateBindGroup( DenOfIz_LogicalDevice device, const DenOfIz_BindGroupDesc *desc, DenOfIz_BindGroup *outGroup );

    /**
     * @brief Creates a fence for CPU-GPU synchronization.
     *
     * Fences allow the CPU to wait for GPU work to complete. Pass the fence to
     * ExecuteCommandLists via the Signal field, then call Wait to block until
     * the GPU signals completion.
     *
     * @param device Valid logical device handle.
     * @param[out] outFence Receives the created fence handle.
     * @return DENOFIZ_SUCCESS on success, error code otherwise.
     *
     * @par Valid Usage
     * - @p device must be a valid DenOfIz_LogicalDevice handle
     * - @p outFence must not be NULL
     *
     * @par Example
     * @code
     * DenOfIz_Fence fence;
     * DenOfIz_LogicalDevice_CreateFence(device, &fence);
     *
     * // Submit work with fence
     * DenOfIz_ExecuteCommandListsDesc execDesc = {0};
     * execDesc.Signal                   = fence;
     * execDesc.CommandLists.Elements    = &commandList;
     * execDesc.CommandLists.NumElements = 1;
     * DenOfIz_CommandQueue_ExecuteCommandLists(queue, &execDesc);
     *
     * // Wait for completion
     * DenOfIz_Fence_Wait(fence);
     * DenOfIz_Fence_Reset(fence);
     * @endcode
     *
     * @see DenOfIz_Fence
     * @see DenOfIz_Fence_Wait
     * @see DenOfIz_Fence_Destroy
     */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateFence( DenOfIz_LogicalDevice device, DenOfIz_Fence *outFence );

    /**
     * @brief Creates a semaphore for GPU-GPU synchronization.
     *
     * Semaphores coordinate execution order between command queue submissions.
     * One submission signals the semaphore, another waits on it.
     *
     * @param device Valid logical device handle.
     * @param[out] outSemaphore Receives the created semaphore handle.
     * @return DENOFIZ_SUCCESS on success, error code otherwise.
     *
     * @par Valid Usage
     * - @p device must be a valid DenOfIz_LogicalDevice handle
     * - @p outSemaphore must not be NULL
     *
     * @par Example
     * @code
     * DenOfIz_Semaphore renderSemaphore;
     * DenOfIz_LogicalDevice_CreateSemaphore(device, &renderSemaphore);
     *
     * // First submission signals the semaphore
     * DenOfIz_ExecuteCommandListsDesc exec1 = {0};
     * exec1.CommandLists.Elements        = &shadowPass;
     * exec1.CommandLists.NumElements     = 1;
     * exec1.SignalSemaphores.Elements    = &renderSemaphore;
     * exec1.SignalSemaphores.NumElements = 1;
     * DenOfIz_CommandQueue_ExecuteCommandLists(queue, &exec1);
     *
     * // Second submission waits on the semaphore
     * DenOfIz_ExecuteCommandListsDesc exec2 = {0};
     * exec2.CommandLists.Elements      = &mainPass;
     * exec2.CommandLists.NumElements   = 1;
     * exec2.WaitSemaphores.Elements    = &renderSemaphore;
     * exec2.WaitSemaphores.NumElements = 1;
     * DenOfIz_CommandQueue_ExecuteCommandLists(queue, &exec2);
     * @endcode
     *
     * @see DenOfIz_Semaphore
     * @see DenOfIz_Semaphore_Destroy
     */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateSemaphore( DenOfIz_LogicalDevice device, DenOfIz_Semaphore *outSemaphore );
    /**
     * @brief Creates a buffer resource for vertex, index, uniform, or structured data.
     *
     * @param device Valid logical device handle.
     * @param desc Buffer configuration. Must not be NULL.
     * @param[out] outBuffer Receives the created buffer handle.
     * @return DENOFIZ_SUCCESS on success, DENOFIZ_ERROR_INVALID_ARGUMENT if validation fails.
     *
     * @par Valid Usage
     * - @p device must be a valid DenOfIz_LogicalDevice handle
     * - @p desc must not be NULL
     * - @p desc->NumBytes must be > 0
     * - Structured buffers require StructureDesc.Stride and NumElements to be set
     * - @p outBuffer must not be NULL
     *
     * @par Example
     * @code
     * DenOfIz_BufferDesc bufferDesc = {0};
     * bufferDesc.Descriptor   = DENOFIZ_RESOURCE_DESCRIPTOR_VERTEX_BUFFER_BIT;
     * bufferDesc.NumBytes     = vertexCount * sizeof(Vertex);
     * bufferDesc.HeapType     = DENOFIZ_HEAP_TYPE_GPU;
     * bufferDesc.Usages       = DENOFIZ_RESOURCE_USAGE_COPY_DST_BIT;
     * bufferDesc.DebugName    = DENOFIZ_STRING("VertexBuffer");
     *
     * DenOfIz_Buffer vertexBuffer;
     * DenOfIz_LogicalDevice_CreateBuffer(device, &bufferDesc, &vertexBuffer);
     * @endcode
     *
     * @see DenOfIz_BufferDesc
     * @see DenOfIz_Buffer_Destroy
     */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateBuffer( DenOfIz_LogicalDevice device, const DenOfIz_BufferDesc *desc, DenOfIz_Buffer *outBuffer );

    /**
     * @brief Creates a texture resource for image data.
     *
     * @param device Valid logical device handle.
     * @param desc Texture configuration. Must not be NULL.
     * @param[out] outTexture Receives the created texture handle.
     * @return DENOFIZ_SUCCESS on success, DENOFIZ_ERROR_INVALID_ARGUMENT if validation fails.
     *
     * @par Valid Usage
     * - @p device must be a valid DenOfIz_LogicalDevice handle
     * - @p desc must not be NULL and pass validation (see DenOfIz_TextureDesc)
     * - @p outTexture must not be NULL
     *
     * @par Example
     * @code
     * DenOfIz_TextureDesc textureDesc = {0};
     * textureDesc.Format     = DENOFIZ_FORMAT_R8G8B8A8_UNORM;
     * textureDesc.Usage      = DENOFIZ_TEXTURE_USAGE_RENDER_ATTACHMENT_BIT;
     * textureDesc.HeapType   = DENOFIZ_HEAP_TYPE_GPU;
     * textureDesc.Width      = 512;
     * textureDesc.Height     = 512;
     * textureDesc.Depth      = 1;
     * textureDesc.ArraySize  = 1;
     * textureDesc.MipLevels  = 1;
     * textureDesc.DebugName  = DENOFIZ_STRING("Diffuse");
     *
     * DenOfIz_Texture texture;
     * DenOfIz_LogicalDevice_CreateTexture(device, &textureDesc, &texture);
     * @endcode
     *
     * @see DenOfIz_TextureDesc
     * @see DenOfIz_TextureResource_Destroy
     */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateTexture( DenOfIz_LogicalDevice device, const DenOfIz_TextureDesc *desc, DenOfIz_Texture *outTexture );

    /**
     * @brief Creates a texture sampler.
     *
     * @param device Valid logical device handle.
     * @param desc Sampler configuration. Must not be NULL.
     * @param[out] outSampler Receives the created sampler handle.
     * @return DENOFIZ_SUCCESS on success, error code otherwise.
     *
     * @par Valid Usage
     * - @p device must be a valid DenOfIz_LogicalDevice handle
     * - @p desc must not be NULL
     * - @p outSampler must not be NULL
     *
     * @par Example
     * @code
     * DenOfIz_SamplerDesc samplerDesc = {0};
     * samplerDesc.MagFilter     = DENOFIZ_FILTER_LINEAR;
     * samplerDesc.MinFilter     = DENOFIZ_FILTER_LINEAR;
     * samplerDesc.MipmapMode    = DENOFIZ_MIPMAP_MODE_LINEAR;
     * samplerDesc.AddressModeU  = DENOFIZ_SAMPLER_ADDRESS_MODE_REPEAT;
     * samplerDesc.AddressModeV  = DENOFIZ_SAMPLER_ADDRESS_MODE_REPEAT;
     * samplerDesc.AddressModeW  = DENOFIZ_SAMPLER_ADDRESS_MODE_REPEAT;
     * samplerDesc.MaxAnisotropy = 16.0f;
     * samplerDesc.MinLod        = 0.0f;
     * samplerDesc.MaxLod        = 1000.0f;
     *
     * DenOfIz_Sampler sampler;
     * DenOfIz_LogicalDevice_CreateSampler(device, &samplerDesc, &sampler);
     * @endcode
     *
     * @see DenOfIz_SamplerDesc
     * @see DenOfIz_Sampler_Destroy
     */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateSampler( DenOfIz_LogicalDevice device, const DenOfIz_SamplerDesc *desc, DenOfIz_Sampler *outSampler );

    /**
     * @brief Creates a query pool for GPU performance measurement.
     *
     * Query pools contain a fixed number of queries of a single type (occlusion,
     * timestamp, or pipeline statistics). Queries are recorded during command
     * list execution and results retrieved after GPU completion.
     *
     * @param device Valid logical device handle.
     * @param desc Query pool configuration. Must not be NULL.
     * @param[out] outPool Receives the created query pool handle.
     * @return DENOFIZ_SUCCESS on success, error code otherwise.
     *
     * @par Valid Usage
     * - @p device must be a valid DenOfIz_LogicalDevice handle
     * - @p desc must not be NULL
     * - @p desc->NumQueries must be > 0
     * - For pipeline statistics, PipelineStatisticFlags must be set
     * - @p outPool must not be NULL
     *
     * @par Example
     * @code
     * // Create timestamp query pool for frame timing
     * DenOfIz_QueryPoolDesc queryPoolDesc = {0};
     * queryPoolDesc.Type       = DENOFIZ_QUERY_TYPE_TIMESTAMP;
     * queryPoolDesc.NumQueries = 2;  // Start and end timestamps
     *
     * DenOfIz_QueryPool queryPool;
     * DenOfIz_LogicalDevice_CreateQueryPool(device, &queryPoolDesc, &queryPool);
     *
     * // In command list:
     * DenOfIz_QueryDesc queryDesc = { .Index = 0 };
     * DenOfIz_CommandList_ResetQuery(cmd, queryPool, 0, 2);
     * DenOfIz_CommandList_BeginQuery(cmd, queryPool, &queryDesc);
     * // ... render ...
     * queryDesc.Index = 1;
     * DenOfIz_CommandList_EndQuery(cmd, queryPool, &queryDesc);
     * DenOfIz_CommandList_ResolveQuery(cmd, queryPool, 0, 2);
     *
     * // After GPU completion:
     * DenOfIz_QueryData data;
     * DenOfIz_QueryPool_GetQueryData(queryPool, 0, &data);
     * @endcode
     *
     * @see DenOfIz_QueryPoolDesc
     * @see DenOfIz_QueryPool
     * @see DenOfIz_QueryPool_Destroy
     */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateQueryPool( DenOfIz_LogicalDevice device, const DenOfIz_QueryPoolDesc *desc, DenOfIz_QueryPool *outPool );

    /**
     * @brief Creates a top-level acceleration structure for ray tracing.
     *
     * A TLAS contains instances of bottom-level acceleration structures with per-instance
     * transforms. It must be built on the GPU via DenOfIz_CommandList_BuildTopLevelAS
     * before use.
     *
     * @param device Valid logical device handle.
     * @param desc TLAS configuration with instances.
     * @param[out] outAS Receives the created TLAS handle.
     * @return DENOFIZ_SUCCESS on success, error code otherwise.
     *
     * @par Valid Usage
     * - @p device must be a valid DenOfIz_LogicalDevice handle
     * - @p desc must not be NULL
     * - @p desc->Instances.NumElements must be > 0
     * - All referenced BLASes must be valid handles
     * - @p outAS must not be NULL
     *
     * @par Example
     * @code
     * DenOfIz_ASInstanceDesc instance = {0};
     * instance.BLAS      = bottomLevelAS;
     * instance.Transform = DENOFIZ_FLOAT4X4_IDENTITY;
     * instance.Mask      = 0xFF;
     *
     * DenOfIz_TopLevelASDesc tlasDesc = {0};
     * tlasDesc.BuildFlags            = DENOFIZ_AS_BUILD_PREFER_FAST_TRACE_BIT;
     * tlasDesc.Instances.Elements    = &instance;
     * tlasDesc.Instances.NumElements = 1;
     *
     * DenOfIz_TopLevelAS tlas;
     * DenOfIz_LogicalDevice_CreateTopLevelAS(device, &tlasDesc, &tlas);
     * @endcode
     *
     * @see DenOfIz_TopLevelAS
     * @see DenOfIz_CommandList_BuildTopLevelAS
     */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateTopLevelAS( DenOfIz_LogicalDevice device, const DenOfIz_TopLevelASDesc *desc, DenOfIz_TopLevelAS *outAS );

    /**
     * @brief Creates a bottom-level acceleration structure for ray tracing.
     *
     * A BLAS contains the actual geometry (triangles or AABBs) for ray intersection.
     * Multiple BLASes can be instanced in a TLAS with different transforms. The BLAS
     * must be built on the GPU via DenOfIz_CommandList_BuildBottomLevelAS before use.
     *
     * @param device Valid logical device handle.
     * @param desc BLAS configuration with geometry.
     * @param[out] outAS Receives the created BLAS handle.
     * @return DENOFIZ_SUCCESS on success, error code otherwise.
     *
     * @par Valid Usage
     * - @p device must be a valid DenOfIz_LogicalDevice handle
     * - @p desc must not be NULL
     * - @p desc->Geometries.NumElements must be > 0
     * - Vertex/index buffers must have ACCELERATION_STRUCTURE_GEOMETRY usage flag
     * - @p outAS must not be NULL
     *
     * @par Example
     * @code
     * DenOfIz_ASGeometryDesc geometry = {0};
     * geometry.Type                   = DENOFIZ_HIT_GROUP_TYPE_TRIANGLES;
     * geometry.Flags                  = DENOFIZ_BLAS_GEOMETRY_OPAQUE_BIT;
     * geometry.Triangles.VertexBuffer = vertexBuffer;
     * geometry.Triangles.VertexFormat = DENOFIZ_FORMAT_R32G32B32_FLOAT;
     * geometry.Triangles.VertexStride = sizeof(float) * 3;
     * geometry.Triangles.NumVertices  = 3;
     * geometry.Triangles.IndexBuffer  = indexBuffer;
     * geometry.Triangles.IndexType    = DENOFIZ_INDEX_TYPE_UINT16;
     * geometry.Triangles.NumIndices   = 3;
     *
     * DenOfIz_BottomLevelASDesc blasDesc = {0};
     * blasDesc.BuildFlags             = DENOFIZ_AS_BUILD_PREFER_FAST_TRACE_BIT;
     * blasDesc.Geometries.Elements    = &geometry;
     * blasDesc.Geometries.NumElements = 1;
     *
     * DenOfIz_BottomLevelAS blas;
     * DenOfIz_LogicalDevice_CreateBottomLevelAS(device, &blasDesc, &blas);
     * @endcode
     *
     * @see DenOfIz_BottomLevelAS
     * @see DenOfIz_CommandList_BuildBottomLevelAS
     */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateBottomLevelAS( DenOfIz_LogicalDevice device, const DenOfIz_BottomLevelASDesc *desc, DenOfIz_BottomLevelAS *outAS );

    /**
     * @brief Creates a shader binding table for ray tracing pipeline execution.
     *
     * The shader binding table (SBT) connects ray tracing shaders with their shader local data and
     * organizes them for GPU execution. It must be populated with shader bindings and built before
     * use in DenOfIz_CommandList_DispatchRays.
     *
     * @param device Valid logical device handle.
     * @param desc Shader binding table descriptor. Must not be NULL.
     * @param[out] outTable Receives the created shader binding table handle.
     * @return DENOFIZ_SUCCESS on success, error code otherwise.
     *
     * @par Valid Usage
     * - @p device must be a valid DenOfIz_LogicalDevice handle
     * - @p desc must not be NULL
     * - @p outTable must not be NULL
     * - @p desc->Pipeline must be a valid ray tracing pipeline handle
     * - @p desc->SizeDesc shader counts must be > 0 for at least NumRayGenerationShaders
     * - Device must support ray tracing
     *
     * @par Example
     * @code
     * // Create ray tracing pipeline (omitted for brevity)
     * DenOfIz_Pipeline rtPipeline;
     * // ... create ray tracing pipeline ...
     *
     * // Create shader local data for hit group
     * DenOfIz_ShaderLocalData materialData;
     * // ... create and populate shader local data ...
     *
     * // Create shader binding table
     * DenOfIz_ShaderBindingTableDesc sbtDesc = {0};
     * sbtDesc.Pipeline = rtPipeline;
     * sbtDesc.SizeDesc.NumRayGenerationShaders = 1;
     * sbtDesc.SizeDesc.NumMissShaders = 1;
     * sbtDesc.SizeDesc.NumHitGroups = 1;
     * sbtDesc.MaxHitGroupDataBytes = 16;
     *
     * DenOfIz_ShaderBindingTable sbt;
     * DenOfIz_LogicalDevice_CreateShaderBindingTable(device, &sbtDesc, &sbt);
     *
     * // Bind shaders
     * DenOfIz_RayGenerationBindingDesc rayGenDesc = {0};
     * rayGenDesc.ShaderName = DENOFIZ_STRING("MyRaygenShader");
     * DenOfIz_ShaderBindingTable_BindRayGenerationShader(sbt, &rayGenDesc);
     *
     * DenOfIz_MissBindingDesc missDesc = {0};
     * missDesc.ShaderName = DENOFIZ_STRING("MyMissShader");
     * DenOfIz_ShaderBindingTable_BindMissShader(sbt, &missDesc);
     *
     * DenOfIz_HitGroupBindingDesc hitGroupDesc = {0};
     * hitGroupDesc.HitGroupExportName = DENOFIZ_STRING("MyHitGroup");
     * hitGroupDesc.Data = materialData;
     * DenOfIz_ShaderBindingTable_BindHitGroup(sbt, &hitGroupDesc);
     *
     * // Build SBT
     * DenOfIz_ShaderBindingTable_Build(sbt);
     * @endcode
     *
     * @see DenOfIz_ShaderBindingTableDesc
     * @see DenOfIz_ShaderBindingTable
     * @see DenOfIz_ShaderBindingTable_Build
     * @see DenOfIz_ShaderBindingTable_Destroy
     */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateShaderBindingTable( DenOfIz_LogicalDevice device, const DenOfIz_ShaderBindingTableDesc *desc,
                                                                          DenOfIz_ShaderBindingTable *outTable );

    /**
     * @brief Creates a local root signature for ray tracing per-shader resource bindings.
     *
     * Local root signatures define resource layouts (CBV, SRV, UAV, samplers) unique to individual
     * shaders in a ray tracing pipeline. They are typically obtained from shader reflection and used
     * to create ShaderLocalData objects for binding per-shader resources.
     *
     * @param device Valid logical device handle.
     * @param desc Local root signature descriptor. Must not be NULL.
     * @param[out] outSignature Receives the created local root signature handle.
     * @return DENOFIZ_SUCCESS on success, error code otherwise.
     *
     * @par Valid Usage
     * - @p device must be a valid DenOfIz_LogicalDevice handle
     * - @p desc must not be NULL
     * - @p outSignature must not be NULL
     * - @p desc->ResourceBindings must contain valid resource binding descriptors
     * - Device must support ray tracing
     *
     * @par Example
     * @code
     * // Reflect ray tracing shader program
     * DenOfIz_ShaderReflectDesc reflection = {0};
     * DenOfIz_ShaderProgram_Reflect(rayTracingProgram, &reflection);
     *
     * // Create local root signature from reflection (for hit group at index 1)
     * DenOfIz_LocalRootSignature hitGroupLayout;
     * DenOfIz_LogicalDevice_CreateLocalRootSignature(
     *     device,
     *     &reflection.LocalRootSignatures.Elements[1],
     *     &hitGroupLayout
     * );
     * @endcode
     *
     * @see DenOfIz_LocalRootSignatureDesc
     * @see DenOfIz_LocalRootSignature
     * @see DenOfIz_ShaderProgram_Reflect
     * @see DenOfIz_LocalRootSignature_Destroy
     */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateLocalRootSignature( DenOfIz_LogicalDevice device, const DenOfIz_LocalRootSignatureDesc *desc,
                                                                          DenOfIz_LocalRootSignature *outSignature );

    /**
     * @brief Creates shader local data for binding per-shader resources in ray tracing.
     *
     * Shader local data binds resources (constant buffers, textures, samplers) unique to individual
     * shaders in a ray tracing pipeline. The data is embedded in the shader binding table and allows
     * per-shader resource customization (e.g., per-geometry materials).
     *
     * @param device Valid logical device handle.
     * @param desc Shader local data descriptor. Must not be NULL.
     * @param[out] outData Receives the created shader local data handle.
     * @return DENOFIZ_SUCCESS on success, error code otherwise.
     *
     * @par Valid Usage
     * - @p device must be a valid DenOfIz_LogicalDevice handle
     * - @p desc must not be NULL
     * - @p outData must not be NULL
     * - @p desc->Layout must be a valid DenOfIz_LocalRootSignature handle
     * - Device must support ray tracing
     *
     * @par Example
     * @code
     * // Create local root signature from shader reflection
     * DenOfIz_ShaderReflectDesc reflection = {0};
     * DenOfIz_ShaderProgram_Reflect(rayTracingProgram, &reflection);
     *
     * DenOfIz_LocalRootSignature hitGroupLayout;
     * DenOfIz_LogicalDevice_CreateLocalRootSignature(
     *     device,
     *     &reflection.LocalRootSignatures.Elements[1],
     *     &hitGroupLayout
     * );
     *
     * // Create shader local data
     * DenOfIz_ShaderLocalDataDesc localDataDesc = {0};
     * localDataDesc.Layout = hitGroupLayout;
     * DenOfIz_ShaderLocalData materialData;
     * DenOfIz_LogicalDevice_CreateShaderLocalData(device, &localDataDesc, &materialData);
     *
     * // Bind resources
     * float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
     * DenOfIz_ByteArrayView colorData = {0};
     * colorData.Elements = (const Byte*)color;
     * colorData.NumElements = sizeof(color);
     *
     * DenOfIz_ShaderLocalData_Begin(materialData);
     * DenOfIz_ShaderLocalData_CbvData(materialData, 0, &colorData);
     * DenOfIz_ShaderLocalData_End(materialData);
     * @endcode
     *
     * @see DenOfIz_ShaderLocalDataDesc
     * @see DenOfIz_ShaderLocalData
     * @see DenOfIz_LocalRootSignature
     * @see DenOfIz_ShaderLocalData_Destroy
     */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateShaderLocalData( DenOfIz_LogicalDevice device, const DenOfIz_ShaderLocalDataDesc *desc, DenOfIz_ShaderLocalData *outData );

    DZ_API void DenOfIz_LogicalDevice_Destroy( DenOfIz_LogicalDevice device );

#ifdef __cplusplus
}
#endif
