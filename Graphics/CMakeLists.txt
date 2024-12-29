include("${PROJECT_SOURCE_DIR}/cmake/common.cmake")

# On windows use dxc from vcpkg, on other platforms build it from source
if (WIN32)
    find_package(directx-headers CONFIG REQUIRED)
    find_package(directxtk12 CONFIG REQUIRED)
    find_package(directx12-agility CONFIG REQUIRED)
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

find_package(SDL2 CONFIG REQUIRED)
find_package(wil CONFIG REQUIRED)
find_package(directxmath CONFIG REQUIRED)
find_package(Taskflow CONFIG REQUIRED)
find_package(glog CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb_image.h")

find_path(TINGLING_INCLUDE_DIRS "tiny_gltf.h")

SET(DEN_OF_IZ_GRAPHICS_COMMON_SOURCES
        Source/Backends/GraphicsApi.cpp
        #Shader Compiler
        Source/Backends/Common/GraphicsWindowHandle.cpp
        Source/Backends/Common/ShaderCompiler.cpp
        Source/Backends/Common/ShaderProgram.cpp

        #Data
        Source/Data/BatchResourceCopy.cpp
        Source/Data/Texture.cpp
        Source/Data/Geometry.cpp

        #Renderers
        Source/Renderer/Assets/AssetData.cpp
        Source/Renderer/Assets/MaterialData.cpp
        Source/Renderer/Common/CommandListRing.cpp
        Source/Renderer/Graph/RenderGraph.cpp

        Source/Utilities/Time.cpp
        Source/Utilities/Utilities.cpp

        # Interface utilities
        Source/Backends/Interface/CommonData.cpp
        Source/Backends/Interface/IInputLayout.cpp
        Source/Backends/Interface/ITextureResource.cpp
        Source/Backends/Interface/ShaderData.cpp
        Source/Backends/Interface/RayTracing/IShaderBindingTable.cpp
)

SET(DEN_OF_IZ_GRAPHICS_VULKAN_SOURCES
        Source/Backends/Vulkan/VulkanLogicalDevice.cpp
        Source/Backends/Vulkan/VulkanCommandList.cpp
        Source/Backends/Vulkan/VulkanCommandPool.cpp
        Source/Backends/Vulkan/VulkanEnumConverter.cpp
        Source/Backends/Vulkan/VulkanResourceBindGroup.cpp
        Source/Backends/Vulkan/VulkanInputLayout.cpp
        Source/Backends/Vulkan/VulkanPipeline.cpp
        Source/Backends/Vulkan/VulkanRootSignature.cpp
        Source/Backends/Vulkan/VulkanSwapChain.cpp
        # RayTracing
        Source/Backends/Vulkan/RayTracing/VulkanBottomLevelAS.cpp
        Source/Backends/Vulkan/RayTracing/VulkanShaderBindingTable.cpp
        Source/Backends/Vulkan/RayTracing/VulkanShaderLocalDataLayout.cpp
        Source/Backends/Vulkan/RayTracing/VulkanShaderLocalData.cpp
        Source/Backends/Vulkan/RayTracing/VulkanTopLevelAS.cpp
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
        Source/Backends/DirectX12/DX12EnumConverter.cpp
        Source/Backends/DirectX12/DX12Pipeline.cpp
        Source/Backends/DirectX12/DX12SwapChain.cpp
        Source/Backends/DirectX12/DX12RootSignature.cpp
        Source/Backends/DirectX12/DX12InputLayout.cpp
        Source/Backends/DirectX12/DX12ResourceBindGroup.cpp
        Source/Backends/DirectX12/DX12CommandList.cpp
        Source/Backends/DirectX12/DX12DescriptorHeap.cpp
        Source/Backends/DirectX12/DX12CommandListPool.cpp
        Source/Backends/DirectX12/DX12BarrierHelper.cpp
        # RayTracing
        Source/Backends/DirectX12/RayTracing/DX12BottomLevelAS.cpp
        Source/Backends/DirectX12/RayTracing/DX12ShaderBindingTable.cpp
        Source/Backends/DirectX12/RayTracing/DX12ShaderLocalDataLayout.cpp
        Source/Backends/DirectX12/RayTracing/DX12ShaderLocalData.cpp
        Source/Backends/DirectX12/RayTracing/DX12TopLevelAS.cpp

        #DirectX12 Resources
        Source/Backends/DirectX12/DX12BufferResource.cpp
        Source/Backends/DirectX12/DX12Fence.cpp
        Source/Backends/DirectX12/DX12TextureResource.cpp
        Source/Backends/DirectX12/DX12Semaphore.cpp
)

set(DEN_OF_IZ_GRAPHICS_METAL_SOURCES
        Source/Backends/Metal/MetalArgumentBuffer.mm
        Source/Backends/Metal/MetalBufferResource.mm
        Source/Backends/Metal/MetalCommandList.mm
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
        # RayTracing
        Source/Backends/Metal/RayTracing/MetalBottomLevelAS.mm
        Source/Backends/Metal/RayTracing/MetalShaderBindingTable.mm
        Source/Backends/Metal/RayTracing/MetalShaderLocalDataLayout.mm
        Source/Backends/Metal/RayTracing/MetalShaderLocalData.mm
        Source/Backends/Metal/RayTracing/MetalTopLevelAS.mm
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
            Source/Renderer/Common/CommandListRing.cpp
    )
endif ()

copy_to_binary(${CMAKE_CURRENT_SOURCE_DIR} "Assets" *)

add_library(DenOfIzGraphics ${DEN_OF_IZ_GRAPHICS_SOURCES})
#install_target(DenOfIzGraphics)
target_include_default_directories(DenOfIzGraphics)

target_include_directories(DenOfIzGraphics
        ${TINYGLTF_INCLUDE_DIRS}
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/Tiny> $<INSTALL_INTERFACE:_ThirdParty/Tiny>
        ${STB_INCLUDE_DIRS}
)

target_link_libraries(DenOfIzGraphics
        PUBLIC
        glog::glog
        Microsoft::DirectXMath
        SDL2::SDL2
        SDL2::SDL2main
        Taskflow::Taskflow
)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(DenOfIzGraphics PRIVATE DZ_GRAPHICS_EXPORTS)
    target_compile_definitions(DenOfIzGraphics INTERFACE DZ_GRAPHICS_IMPORTS)
else ()
    target_link_libraries(DenOfIzGraphics
            PUBLIC
            SDL2::SDL2-static
    )
endif ()

if (WIN32)
    # Non VCPKG because github repo is more up to date, and the one in vcpkg is unofficial
    add_subdirectory(_ThirdParty/D3D12MemoryAllocator)

    target_link_directories(
            DenOfIzGraphics
            PRIVATE
            _ThirdParty/dxc/lib/${CPU_ARCHITECTURE}
    )

    configure_file(_ThirdParty/dxc/bin/${CPU_ARCHITECTURE}/dxcompiler.dll COPYONLY)
    configure_file(_ThirdParty/dxc/bin/${CPU_ARCHITECTURE}/dxil.dll COPYONLY)

    target_link_libraries(DenOfIzGraphics
            PUBLIC
            WIL::WIL
            dxgi
            d3d12
            dxcompiler
            Microsoft::DirectX-Headers
            Microsoft::DirectXTK12
            Microsoft::DirectX-Guids
            D3D12MemoryAllocator
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
            _ThirdParty/DirectXShaderCompiler/external/DirectX-Headers/include
            _ThirdParty/DirectXShaderCompiler/external/DirectX-Headers/include/wsl/stubs
    )

    include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/include_metalirconverter.cmake)

    target_include_directories(DenOfIzGraphics
            PUBLIC
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
            PUBLIC DirectX-Headers
    )

    target_compile_definitions(DenOfIzGraphics PUBLIC BUILD_METAL)
    target_compile_options(DenOfIzGraphics PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:-x objective-c++>
            $<$<COMPILE_LANGUAGE:C>:-x objective-c>)
    set_property(TARGET DenOfIzGraphics APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")
endif ()

if (WIN32 OR LINUX)
    target_compile_definitions(DenOfIzGraphics PUBLIC BUILD_VK)
    target_link_libraries(DenOfIzGraphics PUBLIC
            volk::volk
            volk::volk_headers
            GPUOpen::VulkanMemoryAllocator)
endif ()

set_target_default_properties(DenOfIzGraphics)
#target_compile_definitions(DenOfIzGraphics PUBLIC NSIGHT_ENABLE)

if (BUILD_TESTS)
    add_subdirectory(Tests)
endif ()