option(DZ_INSTALL "Enable install" ON)

include("${PROJECT_SOURCE_DIR}/cmake/common.cmake")

include(Packages.cmake)
include(Sources.cmake)

copy_to_binary(${CMAKE_CURRENT_SOURCE_DIR} "Assets" *)

add_library(DenOfIzGraphics ${DENOFIZ_GRAPHICS_SOURCES})

if (APPLE)
	set_target_properties(DenOfIzGraphics PROPERTIES
		BUILD_WITH_INSTALL_RPATH TRUE
		INSTALL_NAME_DIR "@rpath"
		BUILD_RPATH "@loader_path"
		INSTALL_RPATH "@loader_path"
	)
elseif (UNIX)
	set_target_properties(DenOfIzGraphics PROPERTIES
		BUILD_WITH_INSTALL_RPATH TRUE
		BUILD_RPATH "$ORIGIN"
		INSTALL_RPATH "$ORIGIN"
	)
endif ()

if (TARGET_PLATFORM_WINDOWS AND PIX_AVAILABLE)
    target_compile_definitions(DenOfIzGraphics PRIVATE USE_PIX)
    target_link_libraries(DenOfIzGraphics PRIVATE Microsoft::WinPixEventRuntime)
endif ()

include(GNUInstallDirs)
install(TARGETS DenOfIzGraphics
        EXPORT DenOfIzGraphicsTargets
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Include/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING
        PATTERN "*.h"
)
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Api/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        FILES_MATCHING
        PATTERN "*.h"
)

if (TARGET_PLATFORM_WINDOWS)
    install(FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/dxc/win32/bin/${CPU_ARCHITECTURE}/dxcompiler.dll"
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/dxc/win32/bin/${CPU_ARCHITECTURE}/dxil.dll"
            DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
    if (NOT CPU_ARCHITECTURE STREQUAL "arm64")
        install(FILES
                "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/MetalShaderConverter/lib/win/${CPU_ARCHITECTURE}/metalirconverter.dll"
                DESTINATION ${CMAKE_INSTALL_BINDIR}
        )
    endif ()
elseif (TARGET_PLATFORM_MACOS)
    install(FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/MetalShaderConverter/lib/osx/libmetalirconverter.dylib"
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/dxc/osx/lib/libdxcompiler.dylib"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
elseif (TARGET_PLATFORM_LINUX)
    install(FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/dxc/linux/lib/libdxcompiler.so"
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/dxc/linux/lib/libdxil.so"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif ()

include(CMakePackageConfigHelpers)
configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DenOfIzGraphicsConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/DenOfIzGraphicsConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DenOfIzGraphics
        PATH_VARS CMAKE_INSTALL_INCLUDEDIR CMAKE_INSTALL_LIBDIR
)

write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/DenOfIzGraphicsConfigVersion.cmake"
        VERSION 1.0.0
        COMPATIBILITY AnyNewerVersion
)

install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/DenOfIzGraphicsConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/DenOfIzGraphicsConfigVersion.cmake"
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/DenOfIzGraphicsUtils.cmake"
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/dpi_aware.manifest"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DenOfIzGraphics
)

install(DIRECTORY ${CMAKE_SOURCE_DIR}/Fonts
        DESTINATION share/DenOfIz
        FILES_MATCHING PATTERN "*.dzfont"
        PATTERN "README.md"
)

install(EXPORT DenOfIzGraphicsTargets
        FILE DenOfIzGraphicsTargets.cmake
        NAMESPACE DenOfIz::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DenOfIzGraphics
)

target_include_directories(DenOfIzGraphics
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/clay>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/Tiny>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/MetalShaderConverter/include>
        $<BUILD_INTERFACE:${OZZ_ANIMATION_INCLUDE_DIR}>
        $<BUILD_INTERFACE:${MSDF_ATLAS_GEN_INCLUDE_DIR}>
)

target_include_default_directories(DenOfIzGraphics)
target_include_directories(DenOfIzGraphics PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/Api/Include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_include_directories(DenOfIzGraphics PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Internal)
target_include_directories(DenOfIzGraphics PRIVATE ${Stb_INCLUDE_DIR})
target_include_directories(DenOfIzGraphics PRIVATE ${CGLTF_INCLUDE_DIRS})

target_link_libraries(DenOfIzGraphics PRIVATE
        spdlog::spdlog
        Microsoft::DirectXMath
        SDL3::SDL3
        Freetype::Freetype
        harfbuzz::harfbuzz
        msdfgen::msdfgen-core
        msdfgen::msdfgen-ext
        miniz::miniz
        $<BUILD_INTERFACE:ozz_animation>
        $<BUILD_INTERFACE:ozz_geometry>
        $<BUILD_INTERFACE:ozz_animation_offline>
        $<BUILD_INTERFACE:msdf-atlas-gen::msdf-atlas-gen>
        $<$<NOT:$<BOOL:${TARGET_PLATFORM_WEB}>>:msdfgen::msdfgen>
        $<$<NOT:$<BOOL:${TARGET_PLATFORM_WEB}>>:msdfgen::msdfgen-full>)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(DenOfIzGraphics PRIVATE DZ_GRAPHICS_EXPORTS)
    target_compile_definitions(DenOfIzGraphics PRIVATE GLOG_EXPORT)
    target_compile_definitions(DenOfIzGraphics INTERFACE DZ_GRAPHICS_IMPORTS)
    target_compile_definitions(DenOfIzGraphics PRIVATE BUILD_SHARED_LIBS)
    if (NOT DZ_INSTALL)
        target_compile_definitions(DenOfIzGraphics PRIVATE STB_IMPLEMENTATION)
    endif ()
else ()
    target_compile_definitions(DenOfIzGraphics PUBLIC GLOG_NO_EXPORT)
    target_link_libraries(DenOfIzGraphics
            PRIVATE
            SDL3::SDL3-static
    )
endif ()

if (TARGET_PLATFORM_WINDOWS)
    target_link_directories(
            DenOfIzGraphics
            PRIVATE
            _ThirdParty/dxc/win32/lib/${CPU_ARCHITECTURE}
    )

    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/dxc/win32/bin/${CPU_ARCHITECTURE}/dxcompiler.dll"
            "${CMAKE_BINARY_DIR}/dxcompiler.dll"
            COPYONLY
    )
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/dxc/win32/bin/${CPU_ARCHITECTURE}/dxil.dll"
            "${CMAKE_BINARY_DIR}/dxil.dll"
            COPYONLY
    )
    if (NOT CPU_ARCHITECTURE STREQUAL "arm64")
        configure_file(
                "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/MetalShaderConverter/lib/win/${CPU_ARCHITECTURE}/metalirconverter.dll"
                "${CMAKE_BINARY_DIR}/metalirconverter.dll"
                COPYONLY
        )

        add_library(metalirconverter STATIC IMPORTED)
        set_target_properties(metalirconverter PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/MetalShaderConverter/lib/win/${CPU_ARCHITECTURE}/metalirconverter.lib")
    else ()
        target_compile_definitions(DenOfIzGraphics PRIVATE DZ_NO_METAL_SHADER_CONVERTER)
    endif ()

    target_include_directories(DenOfIzGraphics
            PRIVATE
            _ThirdParty/dxc/win32/include/dxc
    )

    target_link_libraries(DenOfIzGraphics
            PRIVATE
            dxgi
            d3d12
            dxcompiler
            $<$<TARGET_EXISTS:WIL::WIL>:WIL::WIL>
            Microsoft::DirectX-Headers
            Microsoft::DirectX-Guids
            D3D12MemoryAllocator
    )

    if (NOT CPU_ARCHITECTURE STREQUAL "arm64")
        target_link_libraries(DenOfIzGraphics
                PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/MetalShaderConverter/lib/win/${CPU_ARCHITECTURE}/metalirconverter.lib
        )
    endif ()

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
if (TARGET_PLATFORM_MACOS)
    find_library(
            metal_irconverter_LIBRARIES
            NAMES metalirconverter
            PATHS "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/MetalShaderConverter/lib/osx/"
    )
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/MetalShaderConverter/lib/osx/libmetalirconverter.dylib"
            "${CMAKE_BINARY_DIR}/libmetalirconverter.dylib"
            COPYONLY
    )
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/dxc/osx/lib/libdxcompiler.dylib"
            "${CMAKE_BINARY_DIR}/libdxcompiler.dylib"
            COPYONLY
    )

    target_link_directories(
            DenOfIzGraphics
            PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/dxc/osx/lib>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
    )

    target_include_directories(DenOfIzGraphics
            PRIVATE
            _ThirdParty/DirectXShaderCompiler/external/DirectX-Headers/include
            _ThirdParty/DirectXShaderCompiler/external/DirectX-Headers/include/wsl/stubs
            _ThirdParty/dxc/osx/include/dxc
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
if (TARGET_PLATFORM_LINUX)
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/dxc/linux/lib/libdxcompiler.so"
            "${CMAKE_BINARY_DIR}/libdxcompiler.so"
            COPYONLY
    )
    configure_file(
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/dxc/linux/lib/libdxil.so"
            "${CMAKE_BINARY_DIR}/libdxil.so"
            COPYONLY
    )

    set(CMAKE_INSTALL_RPATH "$ORIGIN")
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)

    target_link_directories(
            DenOfIzGraphics
            PUBLIC
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/dxc/linux/lib>
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_LIBDIR}>
    )

    set_target_properties(DenOfIzGraphics PROPERTIES
            BUILD_RPATH "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/dxc/linux/lib:${CMAKE_BINARY_DIR}"
            INSTALL_RPATH "$ORIGIN"
            BUILD_WITH_INSTALL_RPATH OFF
            INSTALL_RPATH_USE_LINK_PATH OFF
            SKIP_BUILD_RPATH OFF
    )

    target_link_libraries(DenOfIzGraphics
            PRIVATE
            dxcompiler
            # Microsoft::DirectX-Headers # Can't use this because it conflicts with Dxc
    )

    target_include_directories(DenOfIzGraphics
            PRIVATE
            # Use DirectX-Headers from DirecXShaderCompiler even though we use prebuilt dxc, see note above on Dxc
            _ThirdParty/DirectXShaderCompiler/external/DirectX-Headers/include
            _ThirdParty/DirectXShaderCompiler/external/DirectX-Headers/include/wsl/stubs
            _ThirdParty/dxc/linux/include/dxc
    )

    #/usr/bin/ld: relocation R_X86_64_PC32 against symbol `_ZNSt6ranges5views4keysE' can not be used when making a shared object; recompile with -fPIC
    set_target_properties(DenOfIzGraphics PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif ()

if (TARGET_PLATFORM_WINDOWS OR TARGET_PLATFORM_LINUX)
    target_compile_definitions(DenOfIzGraphics PUBLIC BUILD_VK)
    target_link_libraries(DenOfIzGraphics PRIVATE
            volk::volk
            volk::volk_headers
            GPUOpen::VulkanMemoryAllocator)
endif ()

if (BUILD_WEBGPU_NATIVE)
    include(FetchContent)
    FetchContent_Declare(
            webgpu
            GIT_REPOSITORY https://github.com/eliemichel/WebGPU-distribution
            GIT_TAG wgpu-v24.0.0.2
    )
    FetchContent_MakeAvailable(webgpu)
    FetchContent_Declare(
            sdl3webgpu
            GIT_REPOSITORY https://github.com/eliemichel/sdl3webgpu
            GIT_TAG main
    )
    FetchContent_MakeAvailable(sdl3webgpu)
    target_link_libraries(DenOfIzGraphics PRIVATE
            $<BUILD_INTERFACE:webgpu>
            $<BUILD_INTERFACE:sdl3webgpu>
    )
    target_copy_webgpu_binaries(DenOfIzGraphics)
    if (DEFINED WEBGPU_RUNTIME_LIB)
        if (TARGET_PLATFORM_WINDOWS)
            install(FILES ${WEBGPU_RUNTIME_LIB}
                    DESTINATION ${CMAKE_INSTALL_BINDIR}
            )
        else ()
            install(FILES ${WEBGPU_RUNTIME_LIB}
                    DESTINATION ${CMAKE_INSTALL_LIBDIR}
            )
        endif ()
        set_target_properties(DenOfIzGraphics PROPERTIES WEBGPU_RUNTIME_LIB "${WEBGPU_RUNTIME_LIB}")
    endif ()
    target_compile_definitions(DenOfIzGraphics PUBLIC
            BUILD_WEBGPU
            BUILD_WEBGPU_NATIVE
    )
endif ()
if (TARGET_PLATFORM_WEB)
    if (EMSCRIPTEN_VERSION VERSION_GREATER_EQUAL "4.0.10")
        target_compile_options(DenOfIzGraphics PUBLIC
                "SHELL:--use-port=emdawnwebgpu"
        )
        target_link_options(DenOfIzGraphics PUBLIC
                "SHELL:--use-port=emdawnwebgpu"
                "SHELL:-sASYNCIFY=1"
                "SHELL:-sALLOW_MEMORY_GROWTH=1"
        )
    elseif (EMSCRIPTEN_VERSION VERSION_GREATER_EQUAL "3.1.56")
        target_compile_options(DenOfIzGraphics PUBLIC
                "SHELL:-sUSE_WEBGPU=1"
        )
        target_link_options(DenOfIzGraphics PUBLIC
                "SHELL:-sUSE_WEBGPU=1"
                "SHELL:-sASYNCIFY=1"
                "SHELL:-sALLOW_MEMORY_GROWTH=1"
        )
    else ()
        message(FATAL_ERROR "Unsupported emscripten version, use Emscripten 3.1.56+")
    endif ()
    target_compile_definitions(DenOfIzGraphics PUBLIC
            _XM_NO_INTRINSICS_ # DirectxMath
            DZ_DXC_NOT_SUPPORTED
            BUILD_WEBGPU
    )

    set(WASM_COMBINED_OBJ "${CMAKE_BINARY_DIR}/combined.o")
    set(VCPKG_LIB_DIR "${CMAKE_BINARY_DIR}/vcpkg_installed/wasm32-emscripten/lib")

    add_custom_command(TARGET DenOfIzGraphics POST_BUILD
        COMMAND ${CMAKE_C_COMPILER} -r -o "${WASM_COMBINED_OBJ}"
            -Wl,--whole-archive
            $<TARGET_FILE:DenOfIzGraphics>
            $<TARGET_FILE:spdlog::spdlog>
            $<TARGET_FILE:fmt::fmt>
            $<TARGET_FILE:SDL3::SDL3>
            "${VCPKG_LIB_DIR}/libfreetype.a"
            "${VCPKG_LIB_DIR}/libharfbuzz.a"
            "${VCPKG_LIB_DIR}/libpng.a"
            "${VCPKG_LIB_DIR}/libbz2.a"
            "${VCPKG_LIB_DIR}/libbrotlicommon.a"
            "${VCPKG_LIB_DIR}/libbrotlidec.a"
            "${VCPKG_LIB_DIR}/libbrotlienc.a"
            "${VCPKG_LIB_DIR}/libmsdfgen-core.a"
            "${VCPKG_LIB_DIR}/libmsdfgen-ext.a"
            "${VCPKG_LIB_DIR}/libminiz.a"
            $<TARGET_FILE:ozz_animation>
            $<TARGET_FILE:ozz_animation_offline>
            $<TARGET_FILE:ozz_geometry>
            $<TARGET_FILE:ozz_base>
            $<TARGET_FILE:msdf-atlas-gen::msdf-atlas-gen>
            -Wl,--no-whole-archive
        COMMAND ${CMAKE_COMMAND} -E remove $<TARGET_FILE:DenOfIzGraphics>
        COMMAND ${CMAKE_AR} rcs $<TARGET_FILE:DenOfIzGraphics> "${WASM_COMBINED_OBJ}"
        COMMAND ${CMAKE_COMMAND} -E remove "${WASM_COMBINED_OBJ}"
        COMMENT "Bundling all dependencies into libDenOfIzGraphics.a"
    )
endif ()

set_target_default_properties(DenOfIzGraphics)

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/DenOfIzGraphicsUtils.cmake)
if (TARGET_PLATFORM_LINUX)
    LINUX_LIB(DenOfIzGraphics)
endif ()

if (BUILD_TESTS)
    add_subdirectory(Tests)
endif ()

if (BUILD_TOOLS)
    add_subdirectory(Tools)
endif ()
