include("${PROJECT_SOURCE_DIR}/cmake/common.cmake")
include("${PROJECT_SOURCE_DIR}/Engine/The-Forge/cmake/theforge_helper.cmake")

# On windows use dxc from vcpkg, on other platforms build it from source
if (WIN32)
    find_package(directx-headers CONFIG REQUIRED)
    find_package(directxtk12 CONFIG REQUIRED)
    find_package(directx12-agility CONFIG)
else ()
    # MacOS still needs to build from source
    include("${CMAKE_CURRENT_SOURCE_DIR}/../cmake/include_dxcompiler.cmake")
    build_dxcompiler(_ThirdParty/DirectXShaderCompiler)
endif ()

if (WIN32 OR LINUX)
    find_package(volk CONFIG REQUIRED)
    find_package(Vulkan REQUIRED)
    find_package(VulkanMemoryAllocator CONFIG REQUIRED)
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
find_package(Python3 COMPONENTS Interpreter REQUIRED)
find_package(SDL2 CONFIG REQUIRED)
find_package(wil CONFIG REQUIRED)
find_package(directxmath CONFIG REQUIRED)

find_path(TINGLING_INCLUDE_DIRS "tiny_gltf.h")

SET(DEN_OF_IZ_GRAPHICS_COMMON_SOURCES
        #Shader Compiler
        Source/Backends/Common/GfxGlobal.cpp
        Source/Backends/Common/ShaderCompiler.cpp
        Source/Backends/Common/ShaderReflection.cpp
        Source/Backends/Common/ShaderProgram.cpp

        #Data
        Source/Data/BatchResourceCopy.cpp
        Source/Data/Texture.cpp
        Source/Data/VertexTypes.cpp
        Source/Data/Geometry.cpp

        #Renderers
        Source/Renderer/ComputeTest.cpp
        Source/Renderer/SimpleRenderer.cpp

        #Helpers
        Source/Helpers/BatchResourceCopyHelper.cpp
)

SET(DEN_OF_IZ_GRAPHICS_VULKAN_SOURCES
        Source/Backends/Vulkan/VulkanLogicalDevice.cpp
        Source/Backends/Vulkan/VulkanCommandList.cpp
        Source/Backends/Vulkan/VulkanResourceBindGroup.cpp
        Source/Backends/Vulkan/VulkanInputLayout.cpp
        Source/Backends/Vulkan/VulkanUtilities.cpp
        Source/Backends/Vulkan/VulkanPipeline.cpp
        Source/Backends/Vulkan/VulkanRootSignature.cpp
        Source/Backends/Vulkan/VulkanSwapChain.cpp
        # Vulkan Resources
        Source/Backends/Vulkan/VulkanFence.cpp
        Source/Backends/Vulkan/VulkanSemaphore.cpp
        Source/Backends/Vulkan/VulkanBufferResource.cpp
        Source/Backends/Vulkan/VulkanTextureResource.cpp
        Source/Backends/Vulkan/VulkanPipelineBarrierHelper.cpp
        Source/Backends/Vulkan/VulkanDescriptorPoolManager.cpp
)


set(DEN_OF_IZ_GRAPHICS_DIRECTX12_SOURCES
        #DirectX12 Backend
        Source/Backends/DirectX12/DX12LogicalDevice.cpp
        Source/Backends/DirectX12/DeviceResources.cpp
        Source/Backends/DirectX12/DX12Pipeline.cpp
        Source/Backends/DirectX12/DX12SwapChain.cpp
        Source/Backends/DirectX12/DX12RootSignature.cpp
        Source/Backends/DirectX12/DX12InputLayout.cpp
        Source/Backends/DirectX12/DX12ResourceBindGroup.cpp
        Source/Backends/DirectX12/DX12CommandList.cpp

        #DirectX12 Resources
        Source/Backends/DirectX12/DX12BufferResource.cpp
        Source/Backends/DirectX12/DX12Fence.cpp
        Source/Backends/DirectX12/DX12TextureResource.cpp
        Source/Backends/DirectX12/DX12Semaphore.cpp
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
        PUBLIC
        _ThirdParty/D3D12MemoryAllocator #TODO: Remove once dependency to the-forge is removed !TF!
        _ThirdParty/Tiny/
)

target_link_libraries(DenOfIzGraphics
        PUBLIC
        DenOfIzCore
        PRIVATE
        SDL2::SDL2
        SDL2::SDL2main
        SDL2::SDL2-static
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
        Microsoft::DirectXMath
)

if (WIN32)
    target_include_directories(DenOfIzGraphics
            PRIVATE
            _ThirdParty/dxc/inc
    )

    target_link_directories(
            DenOfIzGraphics
            PRIVATE
            _ThirdParty/dxc/lib/${CPU_ARCHITECTURE}
    )

    configure_file(_ThirdParty/dxc/bin/${CPU_ARCHITECTURE}/dxcompiler.dll COPYONLY)
    configure_file(_ThirdParty/dxc/bin/${CPU_ARCHITECTURE}/dxil.dll COPYONLY)

    target_link_libraries(DenOfIzGraphics
            PRIVATE
            WIL::WIL
            dxgi
            d3d12
            dxcompiler
            Microsoft::DirectX-Headers
            Microsoft::DirectXTK12
            Microsoft::DirectX-Guids
    )

    if (TARGET Microsoft::DirectX12-Agility)
        target_link_libraries(DenOfIzGraphics
                PRIVATE
                Microsoft::DirectX12-Agility
        )
        file(MAKE_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/D3D12")
        add_custom_command(TARGET DenOfIzGraphics POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:Microsoft::DirectX12-Core,IMPORTED_LOCATION_RELEASE> $<TARGET_FILE_DIR:DenOfIzGraphics>/D3D12
                COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_PROPERTY:Microsoft::DirectX12-Layers,IMPORTED_LOCATION_DEBUG> $<TARGET_FILE_DIR:DenOfIzGraphics>/D3D12
                COMMAND_EXPAND_LISTS
        )
    endif ()
    target_compile_definitions(DenOfIzGraphics PUBLIC BUILD_DX12)
endif ()
if (APPLE)
    target_include_directories(DenOfIzGraphics
            PUBLIC
            _ThirdParty/DirectXShaderCompiler/include/dxc
    )

    include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/include_metalirconverter.cmake)

    target_include_directories(DenOfIzGraphics
            PUBLIC
            _ThirdParty/metal-cpp
            _ThirdParty/metal-cpp-extensions
            ${metal_irconverter_INCLUDE_DIRS}
            ${metal_irconverter_runtime_INCLUDE_DIRS}
    )

    target_link_libraries(DenOfIzGraphics
            PRIVATE ${metal_irconverter_LIBRARIES}
            PRIVATE "-framework Cocoa"
            PRIVATE "-framework Foundation"
            PRIVATE "-framework Metal"
            PRIVATE "-framework MetalKit"
            PRIVATE dxcompiler
    )

    target_compile_definitions(DenOfIzGraphics PUBLIC BUILD_METAL)
    target_compile_options(DenOfIzGraphics PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:-x objective-c++>
            $<$<COMPILE_LANGUAGE:C>:-x objective-c>)
endif ()

if (WIN32 OR LINUX)
    target_compile_definitions(DenOfIzGraphics PUBLIC BUILD_VK)
    target_link_libraries(DenOfIzGraphics PUBLIC
            volk::volk
            volk::volk_headers
#            Vulkan::Vulkan
            GPUOpen::VulkanMemoryAllocator)
endif ()

set_target_default_properties(DenOfIzGraphics)