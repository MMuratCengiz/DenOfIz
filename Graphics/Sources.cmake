set(DENOFIZ_BACKENDS_INTERFACE_SOURCES
        Source/Backends/Interface/RayTracing/BottomLevelAS.cpp
        Source/Backends/Interface/RayTracing/TopLevelAS.cpp
        Source/Backends/Interface/RayTracing/LocalRootSignature.cpp
        Source/Backends/Interface/RayTracing/ShaderLocalData.cpp
        Source/Backends/Interface/RayTracing/ShaderBindingTable.cpp
        Source/Backends/Interface/Buffer.cpp
        Source/Backends/Interface/CommandList.cpp
        Source/Backends/Interface/CommandListPool.cpp
        Source/Backends/Interface/CommandQueue.cpp
        Source/Backends/Interface/Fence.cpp
        Source/Backends/Interface/InputLayout.cpp
        Source/Backends/Interface/LogicalDevice.cpp
        Source/Backends/Interface/NativeInterop.cpp
        Source/Backends/Interface/Pipeline.cpp
        Source/Backends/Interface/PipelineCache.cpp
        Source/Backends/Interface/QueryPool.cpp
        Source/Backends/Interface/BindGroup.cpp
        Source/Backends/Interface/BindGroupLayout.cpp
        Source/Backends/Interface/RootSignature.cpp
        Source/Backends/Interface/Semaphore.cpp
        Source/Backends/Interface/SwapChain.cpp
        Source/Backends/Interface/Texture.cpp
        Source/Backends/Interface/CommonData.cpp
        Source/Backends/Common/ShaderProgram.cpp
        Source/Backends/Common/GraphicsWindowHandle.cpp
        Source/Backends/Common/InternalAutoSync.cpp
        Source/Backends/GraphicsApi.cpp
)

set(DENOFIZ_SUPPORT_SOURCES
        Source/Support/FrameSync.cpp
        Source/Support/ResourceTracking.cpp
        Source/Support/RingBuffer.cpp
)

set(DENOFIZ_DATA_SOURCES
        Source/Data/TextureData.cpp
        Source/Data/BatchResourceCopy.cpp
        Source/Data/Geometry.cpp
)

set(DENOFIZ_ASSETS_STREAM_SOURCES
        Source/Assets/Stream/BinaryContainer.cpp
        Source/Assets/Stream/BinaryContainerImpl.cpp
        Source/Assets/Stream/BinaryReader.cpp
        Source/Assets/Stream/BinaryWriter.cpp
)

set(DENOFIZ_ASSETS_SERDE_SOURCES
        Source/Assets/Serde/Font/FontAsset.cpp
        Source/Assets/Serde/Font/FontAssetReader.cpp
        Source/Assets/Serde/Font/FontAssetWriter.cpp
        Source/Assets/Serde/Shader/ShaderAsset.cpp
        Source/Assets/Serde/Shader/ShaderAssetReader.cpp
        Source/Assets/Serde/Shader/ShaderAssetWriter.cpp
        Source/Assets/Serde/Texture/TextureAsset.cpp
        Source/Assets/Serde/Texture/TextureAssetReader.cpp
        Source/Assets/Serde/Texture/TextureAssetWriter.cpp
        Source/Assets/Serde/Common/AssetReaderHelpers.cpp
        Source/Assets/Serde/Common/AssetWriterHelpers.cpp
)

set(DENOFIZ_ASSETS_IMPORT_SOURCES
        Source/Assets/Import/OzzExporter.cpp
        Source/Assets/Import/OzzExporterRuntime.cpp
        Source/Assets/Import/ShaderImporter.cpp
        Source/Assets/Import/TextureImporter.cpp
        Source/Assets/Import/AssetPathUtilities.cpp
        Source/Assets/Import/FontImporter.cpp
)

set(DENOFIZ_ASSETS_FONT_SOURCES
        Source/Assets/Font/Font.cpp
        Source/Assets/Font/FontLibrary.cpp
        Source/Assets/Font/TextRenderer.cpp
        Source/Assets/Font/TextBatch.cpp
        Source/Assets/Font/TextLayout.cpp
        Source/Assets/Font/TextLayoutCache.cpp
        Source/Assets/Font/EmbeddedFonts.cpp
)

set(DENOFIZ_ASSETS_SHADERS_SOURCES
        Source/Assets/FileSystem/FileIO.cpp
        Source/Assets/Shaders/EmbeddedShaders.cpp
)

if (NOT TARGET_PLATFORM_WEB)
    list(APPEND DENOFIZ_ASSETS_SHADERS_SOURCES
            Source/Assets/Shaders/ShaderCompiler.cpp
            Source/Assets/Shaders/DxcEnumConverter.cpp
            Source/Assets/Shaders/ReflectionDebugOutput.cpp
            Source/Assets/Shaders/ShaderReflectionHelper.cpp
            Source/Assets/Shaders/SPIRVToWGSL.cpp
    )
endif ()

if (APPLE OR (WIN32 AND NOT CPU_ARCHITECTURE STREQUAL "arm64"))
    list(APPEND DENOFIZ_ASSETS_SHADERS_SOURCES Source/Assets/Shaders/DxilToMsl.cpp)
endif ()

set(DENOFIZ_ASSETS_SOURCES
        ${DENOFIZ_ASSETS_STREAM_SOURCES}
        ${DENOFIZ_ASSETS_SERDE_SOURCES}
        ${DENOFIZ_ASSETS_IMPORT_SOURCES}
        ${DENOFIZ_ASSETS_FONT_SOURCES}
        ${DENOFIZ_ASSETS_SHADERS_SOURCES}
)

set(DENOFIZ_ANIMATION_SOURCES
        Source/Animation/Ozz.cpp
)

set(DENOFIZ_INPUT_SOURCES
        Source/Input/Clipboard.cpp
        Source/Input/Controller.cpp
        Source/Input/InputSystem.cpp
        Source/Input/Window.cpp
        Source/Input/Display.cpp
)

if (NOT TARGET_PLATFORM_WEB)
    list(APPEND DENOFIZ_INPUT_SOURCES
            Source/Input/AudioMixer.cpp
            Source/Input/AudioSource.cpp
            Source/Input/AudioTrack.cpp
    )
endif ()

set(DENOFIZ_UI_SOURCES
        Source/UI/ClayContext.cpp
        Source/UI/Clay.cpp
        Source/UI/ClayRenderer.cpp
        Source/UI/ClayTextCache.cpp
        Source/UI/UIShapes.cpp
        Source/UI/UIShapeCache.cpp
        Source/UI/UITextVertexCache.cpp
)

set(DENOFIZ_UTILITIES_SOURCES
        Source/Utilities/FrameDebugRenderer.cpp
        Source/Utilities/Engine.cpp
        Source/Utilities/StepTimer.cpp
        Source/Utilities/InteropUtilities.cpp
        Source/Utilities/CommonDataUtilities.cpp
        Source/Utilities/StringStore.cpp
        Source/Utilities/Utilities.cpp
        Source/Utilities/InteropMathConverter.cpp
        Source/Utilities/StbDsImplementation.cpp
        Source/Assets/FileSystem/FSConfig.cpp
)

set(DENOFIZ_GRAPHICS_VULKAN_SOURCES
        Source/Backends/Vulkan/VulkanLogicalDevice.cpp
        Source/Backends/Vulkan/VulkanCommandList.cpp
        Source/Backends/Vulkan/VulkanCommandQueue.cpp
        Source/Backends/Vulkan/VulkanCommandPool.cpp
        Source/Backends/Vulkan/VulkanEnumConverter.cpp
        Source/Backends/Vulkan/VulkanBindGroup.cpp
        Source/Backends/Vulkan/VulkanBindGroupLayout.cpp
        Source/Backends/Vulkan/VulkanInputLayout.cpp
        Source/Backends/Vulkan/VulkanPipeline.cpp
        Source/Backends/Vulkan/VulkanPipelineCache.cpp
        Source/Backends/Vulkan/VulkanQueryPool.cpp
        Source/Backends/Vulkan/VulkanRootSignature.cpp
        Source/Backends/Vulkan/VulkanSwapChain.cpp
        Source/Backends/Vulkan/RayTracing/VulkanBottomLevelAS.cpp
        Source/Backends/Vulkan/RayTracing/VulkanShaderBindingTable.cpp
        Source/Backends/Vulkan/RayTracing/VulkanLocalRootSignature.cpp
        Source/Backends/Vulkan/RayTracing/VulkanShaderLocalData.cpp
        Source/Backends/Vulkan/RayTracing/VulkanTopLevelAS.cpp
        Source/Backends/Vulkan/VulkanFence.cpp
        Source/Backends/Vulkan/VulkanSemaphore.cpp
        Source/Backends/Vulkan/VulkanBuffer.cpp
        Source/Backends/Vulkan/VulkanTexture.cpp
        Source/Backends/Vulkan/VulkanPipelineBarrierHelper.cpp
        Source/Backends/Vulkan/VulkanDescriptorPoolManager.cpp
)

set(DENOFIZ_GRAPHICS_DIRECTX12_SOURCES
        Source/Backends/DirectX12/DX12LogicalDevice.cpp
        Source/Backends/DirectX12/DX12EnumConverter.cpp
        Source/Backends/DirectX12/DX12Pipeline.cpp
        Source/Backends/DirectX12/DX12PipelineCache.cpp
        Source/Backends/DirectX12/DX12SwapChain.cpp
        Source/Backends/DirectX12/DX12RootSignature.cpp
        Source/Backends/DirectX12/DX12InputLayout.cpp
        Source/Backends/DirectX12/DX12BindGroup.cpp
        Source/Backends/DirectX12/DX12BindGroupLayout.cpp
        Source/Backends/DirectX12/DX12CommandList.cpp
        Source/Backends/DirectX12/DX12CommandQueue.cpp
        Source/Backends/DirectX12/DX12DescriptorHeap.cpp
        Source/Backends/DirectX12/DX12IndirectSignatureCache.cpp
        Source/Backends/DirectX12/DX12CommandListPool.cpp
        Source/Backends/DirectX12/DX12BarrierHelper.cpp
        Source/Backends/DirectX12/RayTracing/DX12BottomLevelAS.cpp
        Source/Backends/DirectX12/RayTracing/DX12ShaderBindingTable.cpp
        Source/Backends/DirectX12/RayTracing/DX12LocalRootSignature.cpp
        Source/Backends/DirectX12/RayTracing/DX12ShaderLocalData.cpp
        Source/Backends/DirectX12/RayTracing/DX12TopLevelAS.cpp
        Source/Backends/DirectX12/DX12Buffer.cpp
        Source/Backends/DirectX12/DX12Fence.cpp
        Source/Backends/DirectX12/DX12Texture.cpp
        Source/Backends/DirectX12/DX12Semaphore.cpp
        Source/Backends/DirectX12/DX12QueryPool.cpp
)

set(DENOFIZ_GRAPHICS_METAL_SOURCES
        Source/Backends/Metal/MetalCapture.mm
        Source/Backends/Metal/MetalArgumentBuffer.mm
        Source/Backends/Metal/MetalBuffer.mm
        Source/Backends/Metal/MetalCommandList.mm
        Source/Backends/Metal/MetalCommandQueue.mm
        Source/Backends/Metal/MetalCommandListPool.mm
        Source/Backends/Metal/MetalEnumConverter.mm
        Source/Backends/Metal/MetalFence.mm
        Source/Backends/Metal/MetalInputLayout.mm
        Source/Backends/Metal/MetalLogicalDevice.mm
        Source/Backends/Metal/MetalPipeline.mm
        Source/Backends/Metal/MetalPipelineCache.mm
        Source/Backends/Metal/MetalBindGroup.mm
        Source/Backends/Metal/MetalBindGroupLayout.mm
        Source/Backends/Metal/MetalRootSignature.mm
        Source/Backends/Metal/MetalSemaphore.mm
        Source/Backends/Metal/MetalSwapChain.mm
        Source/Backends/Metal/MetalTexture.mm
        Source/Backends/Metal/RayTracing/MetalBottomLevelAS.mm
        Source/Backends/Metal/RayTracing/MetalShaderBindingTable.mm
        Source/Backends/Metal/RayTracing/MetalLocalRootSignature.mm
        Source/Backends/Metal/RayTracing/MetalShaderLocalData.mm
        Source/Backends/Metal/RayTracing/MetalTopLevelAS.mm
        Source/Backends/Metal/MetalQueryPool.mm
)

set(DENOFIZ_GRAPHICS_WEBGPU_SOURCES
        Source/Backends/WebGPU/WebGPUEnumConverter.cpp
        Source/Backends/WebGPU/WebGPULogicalDevice.cpp
        Source/Backends/WebGPU/WebGPUBuffer.cpp
        Source/Backends/WebGPU/WebGPUTexture.cpp
        Source/Backends/WebGPU/WebGPUSwapChain.cpp
        Source/Backends/WebGPU/WebGPUFence.cpp
        Source/Backends/WebGPU/WebGPUSemaphore.cpp
        Source/Backends/WebGPU/WebGPUShadowState.cpp
        Source/Backends/WebGPU/WebGPUCommandQueue.cpp
        Source/Backends/WebGPU/WebGPUCommandListPool.cpp
        Source/Backends/WebGPU/WebGPUInputLayout.cpp
        Source/Backends/WebGPU/WebGPUCommandList.cpp
        Source/Backends/WebGPU/WebGPUPipeline.cpp
        Source/Backends/WebGPU/WebGPUPipelineCache.cpp
        Source/Backends/WebGPU/WebGPURootSignature.cpp
        Source/Backends/WebGPU/WebGPUBindGroup.cpp
        Source/Backends/WebGPU/WebGPUBindGroupLayout.cpp
        Source/Backends/WebGPU/WebGPUQueryPool.cpp
)

set(DENOFIZ_GRAPHICS_BACKEND_SOURCES ${DENOFIZ_BACKENDS_INTERFACE_SOURCES})

if (TARGET_PLATFORM_MACOS)
    list(APPEND DENOFIZ_GRAPHICS_BACKEND_SOURCES Source/Utilities/OS_Apple.mm)
elseif (TARGET_PLATFORM_WINDOWS)
    list(APPEND DENOFIZ_GRAPHICS_BACKEND_SOURCES Source/Utilities/OS_Windows.cpp)
elseif (TARGET_PLATFORM_LINUX)
    list(APPEND DENOFIZ_GRAPHICS_BACKEND_SOURCES Source/Utilities/OS_Linux.cpp)
elseif (TARGET_PLATFORM_WEB)
    list(APPEND DENOFIZ_GRAPHICS_BACKEND_SOURCES Source/Utilities/OS_Web.cpp)
endif ()

if (NOT TARGET_PLAFORM_WEB AND BUILD_WEBGPU_NATIVE)
    list(APPEND DENOFIZ_GRAPHICS_BACKEND_SOURCES ${DENOFIZ_GRAPHICS_WEBGPU_SOURCES})
endif ()

if (TARGET_PLATFORM_WEB)
    list(APPEND DENOFIZ_GRAPHICS_BACKEND_SOURCES ${DENOFIZ_GRAPHICS_WEBGPU_SOURCES})
elseif (TARGET_PLATFORM_WINDOWS)
    list(APPEND DENOFIZ_GRAPHICS_BACKEND_SOURCES
            ${DENOFIZ_GRAPHICS_VULKAN_SOURCES}
            ${DENOFIZ_GRAPHICS_DIRECTX12_SOURCES}
    )
elseif (TARGET_PLATFORM_MACOS)
    list(APPEND DENOFIZ_GRAPHICS_BACKEND_SOURCES ${DENOFIZ_GRAPHICS_METAL_SOURCES})
elseif (TARGET_PLATFORM_LINUX)
    list(APPEND DENOFIZ_GRAPHICS_BACKEND_SOURCES ${DENOFIZ_GRAPHICS_VULKAN_SOURCES})
endif ()

if (NOT TARGET_PLATFORM_MACOS)
    list(APPEND DENOFIZ_GRAPHICS_BACKEND_SOURCES Source/Backends/Common/MetalCaptureStub.cpp)
endif ()

set(DENOFIZ_GRAPHICS_SOURCES
        ${DENOFIZ_ASSETS_SOURCES}
        ${DENOFIZ_GRAPHICS_BACKEND_SOURCES}
        ${DENOFIZ_INPUT_SOURCES}
        ${DENOFIZ_ANIMATION_SOURCES}
        ${DENOFIZ_UI_SOURCES}
        ${DENOFIZ_SUPPORT_SOURCES}
        ${DENOFIZ_DATA_SOURCES}
        ${DENOFIZ_UTILITIES_SOURCES}
)
