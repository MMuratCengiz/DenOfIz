include("${PROJECT_SOURCE_DIR}/cmake/common.cmake")
include("${PROJECT_SOURCE_DIR}/Engine/The-Forge/cmake/theforge_helper.cmake")

# On windows use dxc from vcpkg, on other platforms build it from source
if (WIN32)
    find_package(directx-headers CONFIG REQUIRED)
    find_package(directxtk12 CONFIG REQUIRED)
    find_package(directx-dxc CONFIG REQUIRED)
else ()
    include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/include_dxcompiler.cmake")
    build_dxcompiler(_ThirdParty/DirectXShaderCompiler)
endif ()

find_package(Vulkan CONFIG)
find_package(VulkanHeaders CONFIG)
find_package(VulkanMemoryAllocator CONFIG REQUIRED)
find_package(glslang CONFIG REQUIRED)
find_package(spirv_cross_core CONFIG REQUIRED)
find_package(spirv_cross_glsl CONFIG REQUIRED)
find_package(spirv_cross_reflect CONFIG REQUIRED)
find_package(spirv_cross_util CONFIG REQUIRED)
find_package(spirv_cross_cpp CONFIG REQUIRED)
find_package(spirv_cross_hlsl CONFIG REQUIRED)
find_package(spirv_cross_msl CONFIG REQUIRED)
find_package(spirv_cross_c CONFIG REQUIRED)
find_package(Python3 COMPONENTS Interpreter REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_path(TINGLING_INCLUDE_DIRS "tiny_gltf.h")

SET(DEN_OF_IZ_GRAPHICS_COMMON_SOURCES
        #Shader Compiler
        Source/Backends/Common/GfxGlobal.cpp
        Source/Backends/Common/ShaderCompiler.cpp
        Source/Backends/Common/ShaderReflection.cpp
        Source/Backends/Common/ShaderProgram.cpp

        #Renderers
        Source/Renderer/SimpleRenderer.cpp)

SET(DEN_OF_IZ_GRAPHICS_VULKAN_SOURCES
        Source/Backends/Vulkan/VulkanLogicalDevice.cpp
        Source/Backends/Vulkan/VulkanCommandList.cpp
        Source/Backends/Vulkan/VulkanDescriptorTable.cpp
        Source/Backends/Vulkan/VulkanInputLayout.cpp
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
        Source/Backends/Vulkan/Resource/VulkanCubeMapResource.cpp
        Source/Backends/Vulkan/Resource/VulkanPipelineBarrierHelper.cpp)


set(DEN_OF_IZ_GRAPHICS_DIRECTX12_SOURCES
        #DirectX12 Backend
        Source/Backends/DirectX12/DX12LogicalDevice.cpp
        Source/Backends/DirectX12/DeviceResources.cpp
        Source/Backends/DirectX12/DX12Pipeline.cpp
        Source/Backends/DirectX12/DX12SwapChain.cpp
        Source/Backends/DirectX12/DX12RootSignature.cpp
        Source/Backends/DirectX12/DX12InputLayout.cpp
        Source/Backends/DirectX12/DX12DescriptorTable.cpp
        Source/Backends/DirectX12/DX12CommandList.cpp

        #DirectX12 Resources
        Source/Backends/DirectX12/Resource/DX12BufferResource.cpp
        Source/Backends/DirectX12/Resource/DX12Fence.cpp
        Source/Backends/DirectX12/Resource/DX12ImageResource.cpp
        Source/Backends/DirectX12/Resource/DX12Semaphore.cpp
)

set(DEN_OF_IZ_GRAPHICS_METAL_SOURCES
        Source/Backends/Metal/MetalImpl.cpp
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
endif ()

copy_to_binary(${CMAKE_CURRENT_SOURCE_DIR} "Assets" *)
add_library(DenOfIzGraphics ${DEN_OF_IZ_GRAPHICS_SOURCES})
install_target(DenOfIzGraphics)
install(FILES ${SPVHeaders} DESTINATION Include/DenOfIzGraphics)
target_include_default_directories(DenOfIzGraphics)

target_include_directories(DenOfIzGraphics
        PRIVATE
        ${TINYGLTF_INCLUDE_DIRS}
)

if (NOT WIN32)
    target_include_directories(DenOfIzGraphics
            PRIVATE
            _ThirdParty/DirectXShaderCompiler/include
    )

    target_link_libraries(DenOfIzGraphics
            PRIVATE
            dxcompiler
    )
else ()
    target_link_libraries(DenOfIzGraphics
            PUBLIC
            Microsoft::DirectXShaderCompiler
    )
endif ()

target_link_libraries(DenOfIzGraphics
        PUBLIC
        DenOfIzCore
)

target_link_libraries(DenOfIzGraphics
        PRIVATE
        SDL2::SDL2
        SDL2::SDL2main
        SDL2::SDL2-static
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
#    target_link_libraries(DenOfIzGraphics PUBLIC
#            "${CMAKE_CURRENT_SOURCE_DIR}/_/_ThirdParty/dxc/lib/dxcompiler.lib"
#            XInput
#    )
#    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/dxc/bin/x64/dxcompiler.dll" "${PROJECT_BINARY_DIR}/dxcompiler.dll" COPYONLY)
#    configure_file("${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/dxc/bin/x64/dxil.dll" "${PROJECT_BINARY_DIR}/dxil.dll" COPYONLY)

    target_link_libraries(DenOfIzGraphics
            PRIVATE
            dxgi
            d3d12
            Microsoft::DXIL
            Microsoft::DirectXShaderCompiler
            Microsoft::DirectX-Headers
            Microsoft::DirectXTK12
    )

    target_compile_definitions(DenOfIzGraphics PUBLIC BUILD_DX12)
endif ()
if (APPLE)
    include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/include_metalirconverter.cmake)

    target_include_directories(DenOfIzGraphics
            PRIVATE
            _ThirdParty/metal-cpp
            _ThirdParty/metal-cpp-extensions
            ${metal_irconverter_INCLUDE_DIRS}
            ${metal_irconverter_runtime_INCLUDE_DIRS}
    )

    target_link_libraries(DenOfIzGraphics
            PRIVATE
            ${metal_irconverter_LIBRARIES}
    )

    target_link_libraries(DenOfIzGraphics
            PRIVATE "-framework Cocoa"
            PRIVATE "-framework Foundation"
            PRIVATE "-framework Metal"
            PRIVATE "-framework MetalKit"
    )

    target_compile_definitions(DenOfIzGraphics PUBLIC BUILD_METAL)
endif ()


if (WIN32 OR LINUX)
    target_compile_definitions(DenOfIzGraphics PUBLIC BUILD_VK)
endif ()

set_target_default_properties(DenOfIzGraphics)