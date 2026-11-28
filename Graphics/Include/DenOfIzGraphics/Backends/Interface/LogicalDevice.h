/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
        bool EnableValidationLayers; /**< D3D12 debug layer, Vulkan validation layers and debug labels, Metal GPU capture. */
    } DenOfIz_LogicalDeviceDesc;

    /**
     * Creates the GPU objects of one adapter. Get one from DenOfIz_GraphicsApi_CreateLogicalDevice and load a GPU with LoadPhysicalDevice,
     * or from DenOfIz_GraphicsApi_CreateAndLoadOptimalLogicalDevice, which loads the best one.
     *
     * Create functions return DENOFIZ_ERROR_INVALID_HANDLE for an invalid device, and DENOFIZ_ERROR_INVALID_ARGUMENT for a null desc or
     * output pointer or a desc rejected by validation, which logs the reason. CreateBuffer, CreateTexture and CreateBindGroup return
     * DENOFIZ_ERROR_OUT_OF_DEVICE_MEMORY when the allocation fails.
     *
     * The caller owns every created object and releases it with its Destroy function. Buffers, textures, samplers, bind groups, bind group
     * layouts, root signatures, input layouts, pipelines, query pools, acceleration structures, shader binding tables, local root
     * signatures and shader local data are released late, see ReleaseDestroyedResources. Command queues, command list pools, pipeline
     * caches, fences, semaphores and swap chains are released by their Destroy call, so the GPU must be done with them first.
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_LogicalDevice )

    /** DenOfIz_GraphicsApi_CreateLogicalDevice already calls this. */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateDevice( DenOfIz_LogicalDevice device, const DenOfIz_LogicalDeviceDesc *desc );
    /** The array and the device names are owned by the logical device, valid until the next call or until it is destroyed. */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_ListPhysicalDevices( DenOfIz_LogicalDevice device, DenOfIz_PhysicalDeviceArray *outDevices );
    /** physicalDevice is one of the ListPhysicalDevices elements. Returns DENOFIZ_ERROR_DEVICE_LOST when the device is lost while loading. */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_LoadPhysicalDevice( DenOfIz_LogicalDevice device, const DenOfIz_PhysicalDevice *physicalDevice );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_DeviceInfo( DenOfIz_LogicalDevice device, DenOfIz_PhysicalDevice *outDevice );
    /** Always false on Metal. */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_IsDeviceLost( DenOfIz_LogicalDevice device, bool *outLost );
    /** Then releases the resources destroyed before the last frame boundary, see ReleaseDestroyedResources. */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_WaitIdle( DenOfIz_LogicalDevice device );
    /**
     * Marks a frame boundary for applications that never present.
     *
     * Destroying one of the late released objects listed on DenOfIz_LogicalDevice does not release it right away: the GPU may still be
     * using it, and command lists recorded earlier in the frame may reference it. The object is released once a frame boundary has passed
     * and the GPU has finished everything submitted before that boundary. Their Destroy functions are safe to call from any thread.
     *
     * DenOfIz_SwapChain_Present is a frame boundary, applications that present do not need this function. Off-screen and compute only
     * applications call it once per iteration, after submitting their command lists. It does not block.
     *
     * Everything that is still pending is released when the logical device is destroyed.
     */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_ReleaseDestroyedResources( DenOfIz_LogicalDevice device );

    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateCommandQueue( DenOfIz_LogicalDevice device, const DenOfIz_CommandQueueDesc *desc, DenOfIz_CommandQueue *outQueue );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateCommandListPool( DenOfIz_LogicalDevice device, const DenOfIz_CommandListPoolDesc *desc, DenOfIz_CommandListPool *outPool );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreatePipeline( DenOfIz_LogicalDevice device, const DenOfIz_PipelineDesc *desc, DenOfIz_Pipeline *outPipeline );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreatePipelineCache( DenOfIz_LogicalDevice device, const DenOfIz_PipelineCacheDesc *desc, DenOfIz_PipelineCache *outCache );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateSwapChain( DenOfIz_LogicalDevice device, const DenOfIz_SwapChainDesc *desc, DenOfIz_SwapChain *outSwapChain );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateRootSignature( DenOfIz_LogicalDevice device, const DenOfIz_RootSignatureDesc *desc, DenOfIz_RootSignature *outSignature );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateBindGroupLayout( DenOfIz_LogicalDevice device, const DenOfIz_BindGroupLayoutDesc *desc, DenOfIz_BindGroupLayout *outLayout );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateInputLayout( DenOfIz_LogicalDevice device, const DenOfIz_InputLayoutDesc *desc, DenOfIz_InputLayout *outLayout );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateBindGroup( DenOfIz_LogicalDevice device, const DenOfIz_BindGroupDesc *desc, DenOfIz_BindGroup *outGroup );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateFence( DenOfIz_LogicalDevice device, DenOfIz_Fence *outFence );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateSemaphore( DenOfIz_LogicalDevice device, DenOfIz_Semaphore *outSemaphore );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateBuffer( DenOfIz_LogicalDevice device, const DenOfIz_BufferDesc *desc, DenOfIz_Buffer *outBuffer );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateTexture( DenOfIz_LogicalDevice device, const DenOfIz_TextureDesc *desc, DenOfIz_Texture *outTexture );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateSampler( DenOfIz_LogicalDevice device, const DenOfIz_SamplerDesc *desc, DenOfIz_Sampler *outSampler );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateQueryPool( DenOfIz_LogicalDevice device, const DenOfIz_QueryPoolDesc *desc, DenOfIz_QueryPool *outPool );

    /** Build it with DenOfIz_CommandList_BuildTopLevelAS before use. */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateTopLevelAS( DenOfIz_LogicalDevice device, const DenOfIz_TopLevelASDesc *desc, DenOfIz_TopLevelAS *outAS );

    /** Build it with DenOfIz_CommandList_BuildBottomLevelAS before use. */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateBottomLevelAS( DenOfIz_LogicalDevice device, const DenOfIz_BottomLevelASDesc *desc, DenOfIz_BottomLevelAS *outAS );

    /** Bind its shaders, then DenOfIz_ShaderBindingTable_Build, before DenOfIz_CommandList_DispatchRays. */
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateShaderBindingTable( DenOfIz_LogicalDevice device, const DenOfIz_ShaderBindingTableDesc *desc,
                                                                          DenOfIz_ShaderBindingTable *outTable );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateLocalRootSignature( DenOfIz_LogicalDevice device, const DenOfIz_LocalRootSignatureDesc *desc,
                                                                          DenOfIz_LocalRootSignature *outSignature );
    DZ_API DenOfIz_Result DenOfIz_LogicalDevice_CreateShaderLocalData( DenOfIz_LogicalDevice device, const DenOfIz_ShaderLocalDataDesc *desc, DenOfIz_ShaderLocalData *outData );

    DZ_API void DenOfIz_LogicalDevice_Destroy( DenOfIz_LogicalDevice device );

#ifdef __cplusplus
}
#endif
