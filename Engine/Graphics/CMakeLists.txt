find_package(Vulkan REQUIRED)
find_package(VulkanHeaders CONFIG)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)

if (WIN32)
find_package(directx-headers CONFIG REQUIRED)
endif ()

find_package(directxtk12 CONFIG REQUIRED)
find_package(directx-dxc CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_reflect CONFIG REQUIRED)
find_package(spirv_cross_util CONFIG REQUIRED)
find_package(spirv_cross_cpp CONFIG REQUIRED)
find_package(spirv_cross_hlsl CONFIG REQUIRED)
find_package(spirv_cross_msl CONFIG REQUIRED)
find_package(spirv_cross_c CONFIG REQUIRED)

include("${PROJECT_SOURCE_DIR}/cmake/common.cmake")

set(DEN_OF_IZ_SOURCES
        #Vulkan Backend
        Source/Backends/Vulkan/VulkanDevice.cpp
        Source/Backends/Vulkan/VulkanSurface.cpp
        Source/Backends/Vulkan/VulkanUtilities.cpp
        Source/Backends/Vulkan/VulkanPipeline.cpp
        Source/Backends/Vulkan/VulkanRenderPass.cpp
        Source/Backends/Vulkan/VmaImplementation.cpp
        Source/Backends/Vulkan/Resource/VulkanLock.cpp
        Source/Backends/Vulkan/Resource/VulkanBufferResource.cpp
        Source/Backends/Vulkan/Resource/VulkanSamplerResource.cpp
        Source/Backends/Vulkan/Resource/VulkanCubeMapResource.cpp
        Source/Backends/Vulkan/Sample/TestVulkanRenderer.cpp

        #DirectX12 Backend
        Source/Backends/DirectX12/DX12Device.cpp
        Source/Backends/DirectX12/DeviceResources.cpp

        Source/Backends/Common/ShaderCompiler.cpp
        Source/Backends/Common/SpvProgram.cpp
)

add_library(DenOfIzGraphics ${DEN_OF_IZ_SOURCES})
install_target(DenOfIzGraphics)
install(FILES ${SPVHeaders} DESTINATION Include/DenOfIzGraphics)
target_include_default_directories(DenOfIzGraphics)
target_link_libraries(DenOfIzGraphics
        PUBLIC
        DenOfIzCore
        ${Vulkan_LIBRARY}
        Vulkan::Headers
        Microsoft::DirectX-Headers
        Microsoft::DirectXShaderCompiler
        Microsoft::DirectXTK12
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
    )

    target_compile_definitions(DenOfIzGraphics PUBLIC BUILD_DX12)
endif ()

target_compile_definitions(DenOfIzGraphics PUBLIC BUILD_VK)
set_target_default_properties(DenOfIzGraphics)