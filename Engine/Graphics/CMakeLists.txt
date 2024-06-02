include("${PROJECT_SOURCE_DIR}/cmake/common.cmake")
include("${PROJECT_SOURCE_DIR}/Engine/The-Forge/cmake/theforge_helper.cmake")

find_package(Vulkan REQUIRED)
find_package(VulkanHeaders CONFIG)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

if (WIN32)
    find_package(directx-headers CONFIG REQUIRED)
    find_package(directxtk12 CONFIG REQUIRED)
    find_package(directx-dxc CONFIG REQUIRED)
endif ()

find_package(glslang CONFIG REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_reflect CONFIG REQUIRED)
find_package(spirv_cross_util CONFIG REQUIRED)
find_package(spirv_cross_cpp CONFIG REQUIRED)
find_package(spirv_cross_hlsl CONFIG REQUIRED)
find_package(spirv_cross_msl CONFIG REQUIRED)
find_package(spirv_cross_c CONFIG REQUIRED)

find_path(TINYGLTF_INCLUDE_DIRS "tiny_gltf.h")

SET(DEN_OF_IZ_GRAPHICS_COMMON_SOURCES

        #Shader Compiler
        Source/Backends/Common/ShaderCompiler.cpp
        Source/Backends/Common/SpvProgram.cpp

        #Renderers
        Source/Renderer/SimpleRenderer.cpp)

SET(DEN_OF_IZ_GRAPHICS_VULKAN_SOURCES
        Source/Backends/Vulkan/VulkanLogicalDevice.cpp
        Source/Backends/Vulkan/VulkanCommandList.cpp
        Source/Backends/Vulkan/VulkanDescriptorTable.cpp
        Source/Backends/Vulkan/VulkanSurface.cpp
        Source/Backends/Vulkan/VulkanUtilities.cpp
        Source/Backends/Vulkan/VulkanPipeline.cpp
        Source/Backends/Vulkan/VulkanRootSignature.cpp
        Source/Backends/Vulkan/VulkanSwapChain.cpp
        Source/Backends/Vulkan/VmaImplementation.cpp
        # Vulkan Resources
        Source/Backends/Vulkan/Resource/VulkanFence.cpp
        Source/Backends/Vulkan/Resource/VulkanSemaphore.cpp
        Source/Backends/Vulkan/Resource/VulkanBufferResource.cpp
        Source/Backends/Vulkan/Resource/VulkanImageResource.cpp
        Source/Backends/Vulkan/Resource/VulkanSamplerResource.cpp
        Source/Backends/Vulkan/Resource/VulkanCubeMapResource.cpp
        Source/Backends/Vulkan/Resource/VulkanPipelineBarrierHelper.cpp)


set(DEN_OF_IZ_GRAPHICS_DIRECTX12_SOURCES
        #DirectX12 Backend
        Source/Backends/DirectX12/DX12LogicalDevice.cpp
        Source/Backends/DirectX12/DeviceResources.cpp
        Source/Backends/DirectX12/DX12Pipeline.cpp
        Source/Backends/DirectX12/DX12SwapChain.cpp
        Source/Backends/DirectX12/DX12RootSignature.cpp

        #DirectX12 Resources
        Source/Backends/DirectX12/Resource/DX12Fence.cpp
)

set(DEN_OF_IZ_GRAPHICS_METAL_SOURCES
    #TODO Implement Metal Backend
)

if (WIN32)
    set(DEN_OF_IZ_GRAPHICS_SOURCES
            ${DEN_OF_IZ_GRAPHICS_COMMON_SOURCES}
            ${DEN_OF_IZ_GRAPHICS_VULKAN_SOURCES}
            ${DEN_OF_IZ_GRAPHICS_DIRECTX12_SOURCES}
    )
elseif (APPLE)
    set(DEN_OF_IZ_GRAPHICS_SOURCES
            ${DEN_OF_IZ_GRAPHICS_COMMON_SOURCES}
            ${DEN_OF_IZ_GRAPHICS_METAL_SOURCES}
    )
else ()
    set(DEN_OF_IZ_GRAPHICS_SOURCES
            ${DEN_OF_IZ_GRAPHICS_COMMON_SOURCES}
            ${DEN_OF_IZ_GRAPHICS_VULKAN_SOURCES}
    )
endif()

#copy_to_binary(${CMAKE_CURRENT_SOURCE_DIR} "Assets" *)
add_library(DenOfIzGraphics ${DEN_OF_IZ_GRAPHICS_SOURCES})
install_target(DenOfIzGraphics)
install(FILES ${SPVHeaders} DESTINATION Include/DenOfIzGraphics)
target_include_default_directories(DenOfIzGraphics)
target_include_directories(DenOfIzGraphics PRIVATE ${TINYGLTF_INCLUDE_DIRS})
target_link_libraries(DenOfIzGraphics
        PUBLIC
        DenOfIzCore
        ${Vulkan_LIBRARY}
        Vulkan::Headers
        GPUOpen::VulkanMemoryAllocator
        spirv-cross-glsl
        spirv-cross-hlsl
        spirv-cross-cpp
        spirv-cross-reflect
        spirv-cross-msl
        spirv-cross-util
        spirv-cross-core
        glslang::glslang
        glslang::glslang-default-resource-limits
        glslang::SPIRV
        glslang::SPVRemapper
)

if (WIN32)
    target_link_libraries(DenOfIzGraphics
            PUBLIC
            Microsoft::DirectX-Headers
            Microsoft::DirectXShaderCompiler
            Microsoft::DirectXTK12
    )

    target_compile_definitions(DenOfIzGraphics PUBLIC BUILD_DX12)
endif ()

if (WIN32 OR LINUX)
    target_compile_definitions(DenOfIzGraphics PUBLIC BUILD_VK)
endif ()

include_theforge(DenOfIzGraphics)
set_target_default_properties(DenOfIzGraphics)