include("${PROJECT_SOURCE_DIR}/cmake/common.cmake")

# On windows use dxc from vcpkg, on other platforms build it from source
if (WIN32 OR LINUX)
    find_package(directx-headers CONFIG REQUIRED)
endif ()
if (WIN32)
    find_package(directx12-agility CONFIG REQUIRED)
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
find_package(assimp CONFIG REQUIRED)
find_package(draco CONFIG REQUIRED)
find_package(Freetype REQUIRED)
find_package(harfbuzz CONFIG REQUIRED)
find_package(msdfgen CONFIG REQUIRED)
find_package(miniz CONFIG REQUIRED)
find_package(tinyxml2 CONFIG REQUIRED)
find_path(STB_INCLUDE_DIRS "stb_image.h")
find_path(TINGLING_INCLUDE_DIRS "tiny_gltf.h")
find_path(EARCUT_HPP_INCLUDE_DIRS "mapbox/earcut.hpp")

find_package(PkgConfig)
pkg_check_modules(thorvg REQUIRED IMPORTED_TARGET thorvg)

set(OZZ_ANIMATION_DIR "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/ozz-animation")
if (EXISTS "${OZZ_ANIMATION_DIR}")
    set(OZZ_ANIMATION_INCLUDE_DIR "${OZZ_ANIMATION_DIR}/include")
    set(OZZ_ANIMATION_FOUND TRUE)
else ()
    message(FATAL_ERROR "ozz-animation not found in _ThirdParty directory")
endif ()

set(DEN_OF_IZ_ASSETS_SOURCES
        Source/Assets/Bundle/Bundle.cpp
        Source/Assets/Bundle/BundleManager.cpp
        Source/Assets/FileSystem/PathResolver.cpp
        Source/Assets/FileSystem/FileIO.cpp
        Source/Assets/FileSystem/FSConfig.cpp
        Source/Assets/Font/Font.cpp
        Source/Assets/Font/FontLibrary.cpp
        Source/Assets/Font/TextRenderer.cpp
        Source/Assets/Font/TextLayout.cpp
        Source/Assets/GpuResource/GpuResourceLoader.cpp
        Source/Assets/Import/AssetScanner.cpp
        Source/Assets/Import/AssetPathUtilities.cpp
        Source/Assets/Import/AssimpImporter.cpp
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

if (APPLE OR WIN32)
    set(DEN_OF_IZ_ASSETS_SOURCES ${DEN_OF_IZ_ASSETS_SOURCES} Source/Assets/Shaders/DxilToMsl.cpp)
endif ()

set(DEN_OF_IZ_ANIMATION_SOURCES
        Source/Animation/AnimationStateManager.cpp
        Source/Animation/OzzAnimation.cpp
)

set(DEN_OF_IZ_INPUT_SOURCES
        Source/Input/Controller.cpp
        Source/Input/EventHandler.cpp
        Source/Input/InputSystem.cpp
        Source/Input/Window.cpp
)

set(DEN_OF_IZ_VG_SOURCES
        Source/Assets/Vector2d/QuadRenderer.cpp
        Source/Assets/Vector2d/ThorVGWrapper.cpp
)

set(DEN_OF_IZ_GRAPHICS_COMMON_SOURCES
        Source/Backends/GraphicsApi.cpp
        #Shader Compiler
        Source/Backends/Common/GraphicsWindowHandle.cpp

        #Data
        Source/Data/AlignedDataWriter.cpp
        Source/Data/BatchResourceCopy.cpp
        Source/Data/Texture.cpp
        Source/Data/Geometry.cpp

        #Renderers
        Source/Renderer/Assets/AssetData.cpp
        Source/Renderer/Assets/MaterialData.cpp
        Source/Renderer/Sync/FrameSync.cpp
        Source/Renderer/Sync/ResourceTracking.cpp
        Source/Renderer/Graph/RenderGraph.cpp

        Source/Utilities/StepTimer.cpp
        Source/Utilities/Time.cpp
        Source/Utilities/Utilities.cpp
        Source/Utilities/FrameDebugRenderer.cpp
        Source/Utilities/InteropMathConverter.cpp
        Source/Utilities/InteropUtilities.cpp

        # Interface utilities
        Source/Backends/Interface/CommonData.cpp
        Source/Backends/Interface/IRootSignature.cpp
        Source/Backends/Interface/ITextureResource.cpp
        Source/Backends/Interface/ShaderData.cpp
        Source/Backends/Interface/RayTracing/IShaderBindingTable.cpp
        Source/Backends/Interface/RayTracing/ILocalRootSignature.cpp
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
        # RayTracing
        Source/Backends/Vulkan/RayTracing/VulkanBottomLevelAS.cpp
        Source/Backends/Vulkan/RayTracing/VulkanShaderBindingTable.cpp
        Source/Backends/Vulkan/RayTracing/VulkanLocalRootSignature.cpp
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
        Source/Backends/DirectX12/DX12CommandQueue.cpp
        Source/Backends/DirectX12/DX12DescriptorHeap.cpp
        Source/Backends/DirectX12/DX12CommandListPool.cpp
        Source/Backends/DirectX12/DX12BarrierHelper.cpp
        # RayTracing
        Source/Backends/DirectX12/RayTracing/DX12BottomLevelAS.cpp
        Source/Backends/DirectX12/RayTracing/DX12ShaderBindingTable.cpp
        Source/Backends/DirectX12/RayTracing/DX12LocalRootSignature.cpp
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
        # RayTracing
        Source/Backends/Metal/RayTracing/MetalBottomLevelAS.mm
        Source/Backends/Metal/RayTracing/MetalShaderBindingTable.mm
        Source/Backends/Metal/RayTracing/MetalLocalRootSignature.mm
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
    )
endif ()

copy_to_binary(${CMAKE_CURRENT_SOURCE_DIR} "Assets" *)

add_library(DenOfIzGraphics
        ${DEN_OF_IZ_ASSETS_SOURCES}
        ${DEN_OF_IZ_GRAPHICS_SOURCES}
        ${DEN_OF_IZ_INPUT_SOURCES}
        ${DEN_OF_IZ_ANIMATION_SOURCES}
        ${DEN_OF_IZ_VG_SOURCES}
)
#install_target(DenOfIzGraphics)

if (CRT_LINKAGE_STATIC EQUAL 1)
    set(ozz_build_msvc_rt_dll OFF CACHE BOOL "Select msvc DLL runtime library" FORCE)
    set(MSDF_ATLAS_DYNAMIC_RUNTIME OFF CACHE BOOL "Link dynamic runtime library instead of static" FORCE)
else ()
    set(ozz_build_msvc_rt_dll ON CACHE BOOL "Select msvc DLL runtime library" FORCE)
    set(MSDF_ATLAS_DYNAMIC_RUNTIME ON CACHE BOOL "Link dynamic runtime library instead of static" FORCE)
endif ()

set(ozz_build_tests OFF CACHE BOOL "Don't build ozz samples" FORCE)
set(ozz_build_samples OFF CACHE BOOL "Don't build ozz samples" FORCE)
set(ozz_build_howtos OFF CACHE BOOL "Don't build ozz howtos" FORCE)

if (LINUX)
    set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif ()
add_subdirectory(_ThirdParty/ozz-animation)

set(MSDF_ATLAS_BUILD_STANDALONE OFF CACHE BOOL "Build the msdf-atlas-gen standalone executable")
set(MSDF_ATLAS_USE_SKIA OFF CACHE BOOL "Build with the Skia library")
set(MSDF_ATLAS_NO_ARTERY_FONT ON CACHE BOOL "Disable Artery Font export and do not require its submodule")
set(MSDFGEN_USE_SKIA OFF CACHE BOOL "Build msdfgen with the Skia library")
set(MSDFGEN_VCPKG_FEATURES_SET ON CACHE BOOL "Use vcpkg features")
set(VCPKG_MANIFEST_NO_DEFAULT_FEATURES ON CACHE BOOL "Disable default features")
add_subdirectory(_ThirdParty/msdf-atlas-gen)

target_include_default_directories(DenOfIzGraphics)
target_include_directories(DenOfIzGraphics
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/Tiny> $<INSTALL_INTERFACE:_ThirdParty/Tiny>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/MetalShaderConverter/include> $<INSTALL_INTERFACE:_ThirdParty/MetalShaderConverter/include>
        $<BUILD_INTERFACE:${OZZ_ANIMATION_INCLUDE_DIR}> $<INSTALL_INTERFACE:_ThirdParty/ozz-animation/include>
        $<BUILD_INTERFACE:${MSDF_ATLAS_GEN_INCLUDE_DIR}> $<INSTALL_INTERFACE:_ThirdParty/msdf-atlas-gen>
        ${EARCUT_HPP_INCLUDE_DIRS}
        ${STB_INCLUDE_DIRS}
)


target_link_libraries(DenOfIzGraphics
        PUBLIC
        glog::glog
        Microsoft::DirectXMath
        SDL2::SDL2
        SDL2::SDL2main
        Taskflow::Taskflow
        assimp::assimp
        ozz_animation
        ozz_geometry
        ozz_animation_offline
        Freetype::Freetype
        harfbuzz::harfbuzz
        msdfgen::msdfgen
        msdfgen::msdfgen-ext
        msdfgen::msdfgen-core
        msdfgen::msdfgen-full
        msdf-atlas-gen::msdf-atlas-gen
        miniz::miniz
        tinyxml2::tinyxml2
        PkgConfig::thorvg
)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(DenOfIzGraphics PRIVATE DZ_GRAPHICS_EXPORTS)
    target_compile_definitions(DenOfIzGraphics INTERFACE DZ_GRAPHICS_IMPORTS)
    target_compile_definitions(DenOfIzGraphics PRIVATE BUILD_SHARED_LIBS)
else ()
    target_compile_definitions(DenOfIzGraphics PUBLIC TVG_STATIC)
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

    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/dxc/bin/${CPU_ARCHITECTURE}/dxcompiler.dll"
            "${CMAKE_BINARY_DIR}/dxcompiler.dll"
            COPYONLY
    )
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/dxc/bin/${CPU_ARCHITECTURE}/dxil.dll"
            "${CMAKE_BINARY_DIR}/dxil.dll"
            COPYONLY
    )
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/MetalShaderConverter/lib/metalirconverter.dll"
            "${CMAKE_BINARY_DIR}/metalirconverter.dll"
            COPYONLY
    )

    add_library(metalirconverter STATIC IMPORTED)
    set_target_properties(metalirconverter PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/MetalShaderConverter/lib/metalirconverter.lib")

    target_link_libraries(DenOfIzGraphics
            PUBLIC
            WIL::WIL
            dxgi
            d3d12
            dxcompiler
            Microsoft::DirectX-Headers
            Microsoft::DirectX-Guids
            D3D12MemoryAllocator
            metalirconverter
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
    find_library(
            metal_irconverter_LIBRARIES
            NAMES metalirconverter
            PATHS "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/MetalShaderConverter/lib/"
    )
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/MetalShaderConverter/lib/libmetalirconverter.dylib"
            "${CMAKE_BINARY_DIR}/libmetalirconverter.dylib"
            COPYONLY
    )
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/osx_dxc/lib/libdxcompiler.dylib"
            "${CMAKE_BINARY_DIR}/libdxcompiler.dylib"
            COPYONLY
    )

    target_link_directories(
            DenOfIzGraphics
            PUBLIC
            _ThirdParty/osx_dxc/lib/
    )

    target_include_directories(DenOfIzGraphics
            PUBLIC
            _ThirdParty/DirectXShaderCompiler/external/DirectX-Headers/include
            _ThirdParty/DirectXShaderCompiler/external/DirectX-Headers/include/wsl/stubs
            _ThirdParty/osx_dxc/include/dxc
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
    set_property(TARGET DenOfIzGraphics APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")
endif ()
if (LINUX)
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/linux_dxc/lib/libdxcompiler.so"
            "${CMAKE_BINARY_DIR}/libdxcompiler.so"
            COPYONLY
    )
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/linux_dxc/lib/libdxil.so"
            "${CMAKE_BINARY_DIR}/libdxil.so"
            COPYONLY
    )

    target_link_directories(
            DenOfIzGraphics
            PUBLIC
            _ThirdParty/linux_dxc/lib/
    )

    target_link_libraries(DenOfIzGraphics
            PRIVATE
            dxcompiler
            # Microsoft::DirectX-Headers # Can't use this because it conflicts with Dxc
    )

    target_include_directories(DenOfIzGraphics
            PUBLIC
            # Use DirectX-Headers from DirecXShaderCompiler even though we use prebuilt dxc, see note above on Dxc
            _ThirdParty/DirectXShaderCompiler/external/DirectX-Headers/include
            _ThirdParty/DirectXShaderCompiler/external/DirectX-Headers/include/wsl/stubs
            _ThirdParty/linux_dxc/include/dxc
    )

    #/usr/bin/ld: relocation R_X86_64_PC32 against symbol `_ZNSt6ranges5views4keysE' can not be used when making a shared object; recompile with -fPIC
    set_target_properties(DenOfIzGraphics PROPERTIES POSITION_INDEPENDENT_CODE ON)
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

if (LINUX)
    LINUX_LIB(DenOfIzGraphics)
endif ()

if (BUILD_TESTS)
    add_subdirectory(Tests)
endif ()