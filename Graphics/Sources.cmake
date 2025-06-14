set(DEN_OF_IZ_ASSETS_SOURCES
    Source/Assets/Bundle/Bundle.cpp
    Source/Assets/Bundle/BundleManager.cpp
    Source/Assets/FileSystem/PathResolver.cpp
    Source/Assets/FileSystem/FileIO.cpp
    Source/Assets/FileSystem/FSConfig.cpp
    Source/Assets/Font/Font.cpp
    Source/Assets/Font/FontLibrary.cpp
    Source/Assets/Font/TextBatch.cpp
    Source/Assets/Font/TextRenderer.cpp
    Source/Assets/Font/TextLayout.cpp
    Source/Assets/Font/TextLayoutCache.cpp
    Source/Assets/Font/EmbeddedFonts.cpp
    Source/Assets/GpuResource/GpuResourceLoader.cpp
    Source/Assets/Import/AssetPathUtilities.cpp
    Source/Assets/Import/AssimpImporter.cpp
    Source/Assets/Import/AssimpSceneLoader.cpp
    Source/Assets/Import/AssimpMeshProcessor.cpp
    Source/Assets/Import/AssimpMaterialProcessor.cpp
    Source/Assets/Import/AssimpSkeletonProcessor.cpp
    Source/Assets/Import/AssimpAnimationProcessor.cpp
    Source/Assets/Import/FontImporter.cpp
    Source/Assets/Import/ShaderImporter.cpp
    Source/Assets/Import/TextureImporter.cpp
    Source/Assets/Import/VGImporter.cpp
    Source/Assets/Serde/Asset.cpp
    Source/Assets/Serde/Animation/AnimationAssetReader.cpp
    Source/Assets/Serde/Animation/AnimationAssetWriter.cpp
    Source/Assets/Serde/Common/AssetReaderHelpers.cpp
    Source/Assets/Serde/Common/AssetWriterHelpers.cpp
    Source/Assets/Serde/Font/FontAssetReader.cpp
    Source/Assets/Serde/Font/FontAssetWriter.cpp
    Source/Assets/Serde/Material/MaterialAssetReader.cpp
    Source/Assets/Serde/Material/MaterialAssetWriter.cpp
    Source/Assets/Serde/Mesh/MeshAssetReader.cpp
    Source/Assets/Serde/Mesh/MeshAssetWriter.cpp
    Source/Assets/Serde/Physics/PhysicsAssetReader.cpp
    Source/Assets/Serde/Physics/PhysicsAssetWriter.cpp
    Source/Assets/Serde/Shader/ShaderAssetReader.cpp
    Source/Assets/Serde/Shader/ShaderAssetWriter.cpp
    Source/Assets/Serde/Skeleton/SkeletonAssetReader.cpp
    Source/Assets/Serde/Skeleton/SkeletonAssetWriter.cpp
    Source/Assets/Serde/Texture/TextureAssetReader.cpp
    Source/Assets/Serde/Texture/TextureAssetWriter.cpp
    Source/Assets/Shaders/DxcEnumConverter.cpp
    Source/Assets/Shaders/ReflectionDebugOutput.cpp
    Source/Assets/Shaders/ShaderCompiler.cpp
    Source/Assets/Shaders/ShaderReflectionHelper.cpp
    Source/Assets/Shaders/ShaderProgram.cpp
    Source/Assets/Stream/BinaryContainer.cpp
    Source/Assets/Stream/BinaryReader.cpp
    Source/Assets/Stream/BinaryWriter.cpp
)

if(APPLE OR WIN32)
    list(APPEND DEN_OF_IZ_ASSETS_SOURCES Source/Assets/Shaders/DxilToMsl.cpp)
endif()

set(DEN_OF_IZ_ANIMATION_SOURCES
    Source/Animation/AnimationStateManager.cpp
    Source/Animation/OzzAnimation.cpp
)

set(DEN_OF_IZ_INPUT_SOURCES
    Source/Input/Controller.cpp
    Source/Input/InputSystem.cpp
    Source/Input/Window.cpp
)

set(DEN_OF_IZ_VG_SOURCES
    Source/Assets/Vector2d/QuadRenderer.cpp
    Source/Assets/Vector2d/ThorVGWrapper.cpp
)

set(DEN_OF_IZ_UI_SOURCES
    Source/UI/ClayContext.cpp
    Source/UI/Clay.cpp
    Source/UI/ClayClipboard.cpp
    Source/UI/ClayRenderer.cpp
    Source/UI/ClayTextCache.cpp
    Source/UI/UIShapes.cpp
    Source/UI/UIShapeCache.cpp
    Source/UI/UITextVertexCache.cpp
    Source/UI/Widgets/Widget.cpp
    Source/UI/Widgets/CheckboxWidget.cpp
    Source/UI/Widgets/SliderWidget.cpp
    Source/UI/Widgets/DropdownWidget.cpp
    Source/UI/Widgets/ColorPickerWidget.cpp
    Source/UI/Widgets/TextFieldWidget.cpp
    Source/UI/Widgets/ResizableContainerWidget.cpp
    Source/UI/Widgets/DockableContainerWidget.cpp
    Source/UI/FullscreenQuadPipeline.cpp
)

set(DEN_OF_IZ_GRAPHICS_COMMON_SOURCES
    Source/Backends/GraphicsApi.cpp
    Source/Backends/Common/GraphicsWindowHandle.cpp
    Source/Data/AlignedDataWriter.cpp
    Source/Data/BatchResourceCopy.cpp
    Source/Data/Texture.cpp
    Source/Data/Geometry.cpp
    Source/Renderer/Sync/FrameSync.cpp
    Source/Renderer/Sync/ResourceTracking.cpp
    Source/Utilities/DZArena.cpp
    Source/Utilities/Engine.cpp
    Source/Utilities/StepTimer.cpp
    Source/Utilities/Utilities.cpp
    Source/Utilities/FrameDebugRenderer.cpp
    Source/Utilities/InteropMathConverter.cpp
    Source/Utilities/InteropUtilities.cpp
    Source/Backends/Interface/CommonData.cpp
    Source/Backends/Interface/ITextureResource.cpp
    Source/Backends/Interface/ShaderData.cpp
    Source/Backends/Interface/RayTracing/IShaderBindingTable.cpp
)

set(DEN_OF_IZ_GRAPHICS_VULKAN_SOURCES
    Source/Backends/Vulkan/VulkanLogicalDevice.cpp
    Source/Backends/Vulkan/VulkanCommandList.cpp
    Source/Backends/Vulkan/VulkanCommandQueue.cpp
    Source/Backends/Vulkan/VulkanCommandPool.cpp
    Source/Backends/Vulkan/VulkanEnumConverter.cpp
    Source/Backends/Vulkan/VulkanResourceBindGroup.cpp
    Source/Backends/Vulkan/VulkanInputLayout.cpp
    Source/Backends/Vulkan/VulkanPipeline.cpp
    Source/Backends/Vulkan/VulkanRootSignature.cpp
    Source/Backends/Vulkan/VulkanSwapChain.cpp
    Source/Backends/Vulkan/RayTracing/VulkanBottomLevelAS.cpp
    Source/Backends/Vulkan/RayTracing/VulkanShaderBindingTable.cpp
    Source/Backends/Vulkan/RayTracing/VulkanLocalRootSignature.cpp
    Source/Backends/Vulkan/RayTracing/VulkanShaderLocalData.cpp
    Source/Backends/Vulkan/RayTracing/VulkanTopLevelAS.cpp
    Source/Backends/Vulkan/VulkanFence.cpp
    Source/Backends/Vulkan/VulkanSemaphore.cpp
    Source/Backends/Vulkan/VulkanBufferResource.cpp
    Source/Backends/Vulkan/VulkanTextureResource.cpp
    Source/Backends/Vulkan/VulkanPipelineBarrierHelper.cpp
    Source/Backends/Vulkan/VulkanDescriptorPoolManager.cpp
)

set(DEN_OF_IZ_GRAPHICS_DIRECTX12_SOURCES
    Source/Backends/DirectX12/DX12LogicalDevice.cpp
    Source/Backends/DirectX12/DX12EnumConverter.cpp
    Source/Backends/DirectX12/DX12Pipeline.cpp
    Source/Backends/DirectX12/DX12SwapChain.cpp
    Source/Backends/DirectX12/DX12RootSignature.cpp
    Source/Backends/DirectX12/DX12InputLayout.cpp
    Source/Backends/DirectX12/DX12ResourceBindGroup.cpp
    Source/Backends/DirectX12/DX12CommandList.cpp
    Source/Backends/DirectX12/DX12CommandQueue.cpp
    Source/Backends/DirectX12/DX12DescriptorHeap.cpp
    Source/Backends/DirectX12/DX12CommandListPool.cpp
    Source/Backends/DirectX12/DX12BarrierHelper.cpp
    Source/Backends/DirectX12/RayTracing/DX12BottomLevelAS.cpp
    Source/Backends/DirectX12/RayTracing/DX12ShaderBindingTable.cpp
    Source/Backends/DirectX12/RayTracing/DX12LocalRootSignature.cpp
    Source/Backends/DirectX12/RayTracing/DX12ShaderLocalData.cpp
    Source/Backends/DirectX12/RayTracing/DX12TopLevelAS.cpp
    Source/Backends/DirectX12/DX12BufferResource.cpp
    Source/Backends/DirectX12/DX12Fence.cpp
    Source/Backends/DirectX12/DX12TextureResource.cpp
    Source/Backends/DirectX12/DX12Semaphore.cpp
)

set(DEN_OF_IZ_GRAPHICS_METAL_SOURCES
    Source/Backends/Metal/MetalArgumentBuffer.mm
    Source/Backends/Metal/MetalBufferResource.mm
    Source/Backends/Metal/MetalCommandList.mm
    Source/Backends/Metal/MetalCommandQueue.mm
    Source/Backends/Metal/MetalCommandListPool.mm
    Source/Backends/Metal/MetalEnumConverter.mm
    Source/Backends/Metal/MetalFence.mm
    Source/Backends/Metal/MetalInputLayout.mm
    Source/Backends/Metal/MetalLogicalDevice.mm
    Source/Backends/Metal/MetalPipeline.mm
    Source/Backends/Metal/MetalResourceBindGroup.mm
    Source/Backends/Metal/MetalRootSignature.mm
    Source/Backends/Metal/MetalSemaphore.mm
    Source/Backends/Metal/MetalSwapChain.mm
    Source/Backends/Metal/MetalTextureResource.mm
    Source/Backends/Metal/RayTracing/MetalBottomLevelAS.mm
    Source/Backends/Metal/RayTracing/MetalShaderBindingTable.mm
    Source/Backends/Metal/RayTracing/MetalLocalRootSignature.mm
    Source/Backends/Metal/RayTracing/MetalShaderLocalData.mm
    Source/Backends/Metal/RayTracing/MetalTopLevelAS.mm
)

set(DEN_OF_IZ_GRAPHICS_BACKEND_SOURCES ${DEN_OF_IZ_GRAPHICS_COMMON_SOURCES})

if(WIN32)
    list(APPEND DEN_OF_IZ_GRAPHICS_BACKEND_SOURCES 
        ${DEN_OF_IZ_GRAPHICS_VULKAN_SOURCES}
        ${DEN_OF_IZ_GRAPHICS_DIRECTX12_SOURCES}
    )
elseif(APPLE)
    list(APPEND DEN_OF_IZ_GRAPHICS_BACKEND_SOURCES ${DEN_OF_IZ_GRAPHICS_METAL_SOURCES})
else()
    list(APPEND DEN_OF_IZ_GRAPHICS_BACKEND_SOURCES ${DEN_OF_IZ_GRAPHICS_VULKAN_SOURCES})
endif()

set(DEN_OF_IZ_GRAPHICS_SOURCES
    ${DEN_OF_IZ_ASSETS_SOURCES}
    ${DEN_OF_IZ_GRAPHICS_BACKEND_SOURCES}
    ${DEN_OF_IZ_INPUT_SOURCES}
    ${DEN_OF_IZ_ANIMATION_SOURCES}
    ${DEN_OF_IZ_UI_SOURCES}
    ${DEN_OF_IZ_VG_SOURCES}
)