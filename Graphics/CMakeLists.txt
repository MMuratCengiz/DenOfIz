include("${PROJECT_SOURCE_DIR}/cmake/common.cmake")

include(Packages.cmake)
include(Sources.cmake)

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

install(EXPORT DenOfIzGraphicsTargets
        FILE DenOfIzGraphicsTargets.cmake
        NAMESPACE DenOfIz::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DenOfIzGraphics
)

target_include_directories(DenOfIzGraphics
        PRIVATE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/Tiny>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/MetalShaderConverter/include>
        $<BUILD_INTERFACE:${OZZ_ANIMATION_INCLUDE_DIR}>
)

target_include_default_directories(DenOfIzGraphics)
target_include_directories(DenOfIzGraphics PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Internal)
target_include_directories(DenOfIzGraphics PRIVATE ${Stb_INCLUDE_DIR})
target_include_directories(DenOfIzGraphics PRIVATE ${CGLTF_INCLUDE_DIRS})

target_link_libraries(DenOfIzGraphics PRIVATE
        spdlog::spdlog
        Microsoft::DirectXMath
        SDL3::SDL3
        harfbuzz::harfbuzz
        SheenBidi::SheenBidi
        libunibreak::libunibreak
        yoga::yogacore
        miniz::miniz
        $<BUILD_INTERFACE:ozz_animation>
        $<BUILD_INTERFACE:ozz_geometry>
        $<BUILD_INTERFACE:ozz_animation_offline>)

if (BUILD_SHARED_LIBS)
    target_compile_definitions(DenOfIzGraphics PRIVATE DZ_GRAPHICS_EXPORTS)
    target_compile_definitions(DenOfIzGraphics INTERFACE DZ_GRAPHICS_IMPORTS)
else ()
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
            dwrite
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
            PRIVATE "-framework CoreFoundation"
            PRIVATE "-framework CoreText"
            PRIVATE "-framework Foundation"
            PRIVATE "-framework Metal"
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
            ${CMAKE_DL_LIBS}
    )

    target_include_directories(DenOfIzGraphics
            PRIVATE
            _ThirdParty/DirectXShaderCompiler/external/DirectX-Headers/include
            _ThirdParty/DirectXShaderCompiler/external/DirectX-Headers/include/wsl/stubs
            _ThirdParty/dxc/linux/include/dxc
    )

    set_target_properties(DenOfIzGraphics PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif ()

if (TARGET_PLATFORM_WINDOWS OR TARGET_PLATFORM_LINUX)
    target_compile_definitions(DenOfIzGraphics PUBLIC BUILD_VK)
    target_link_libraries(DenOfIzGraphics PRIVATE
            volk::volk
            volk::volk_headers
            GPUOpen::VulkanMemoryAllocator)
endif ()

include(FetchContent)
set(BUILD_SHARED_LIBS_TEMP_MIXER ${BUILD_SHARED_LIBS})
set(BUILD_SHARED_LIBS OFF)
FetchContent_Declare(
        SDL3_mixer
        GIT_REPOSITORY https://github.com/libsdl-org/SDL_mixer.git
        GIT_TAG prerelease-3.1.2
)
FetchContent_MakeAvailable(SDL3_mixer)
set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_TEMP_MIXER} CACHE BOOL "Build shared libraries" FORCE)
set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_TEMP_MIXER})
target_link_libraries(DenOfIzGraphics PRIVATE $<BUILD_INTERFACE:SDL3_mixer::SDL3_mixer>)

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
