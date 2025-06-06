option(DZ_INSTALL "Enable install" ON)

include("${PROJECT_SOURCE_DIR}/cmake/common.cmake")

include(Packages.cmake)
include(Sources.cmake)

copy_to_binary(${CMAKE_CURRENT_SOURCE_DIR} "Assets" *)

add_library(DenOfIzGraphics  ${DEN_OF_IZ_GRAPHICS_SOURCES})

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
        PATTERN "*.hpp"
        PATTERN "Internal" EXCLUDE
)

if (WIN32)
    install(FILES 
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/dxc/bin/${CPU_ARCHITECTURE}/dxcompiler.dll"
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/dxc/bin/${CPU_ARCHITECTURE}/dxil.dll"
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/MetalShaderConverter/lib/metalirconverter.dll"
            DESTINATION ${CMAKE_INSTALL_BINDIR}
    )
elseif (APPLE)
    install(FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/MetalShaderConverter/lib/libmetalirconverter.dylib"
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/osx_dxc/lib/libdxcompiler.dylib"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
elseif (LINUX)
    install(FILES
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/linux_dxc/lib/libdxcompiler.so"
            "${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/linux_dxc/lib/libdxil.so"
            DESTINATION ${CMAKE_INSTALL_LIBDIR}
    )
endif()

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
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/DenOfIzGraphics
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
        ${STB_INCLUDE_DIRS}
)

target_include_default_directories(DenOfIzGraphics)
target_include_directories(DenOfIzGraphics PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/Internal)

target_link_libraries(DenOfIzGraphics PRIVATE
        spdlog::spdlog
        Microsoft::DirectXMath
        SDL2::SDL2
        SDL2::SDL2main
        Taskflow::Taskflow
        assimp::assimp
        $<BUILD_INTERFACE:ozz_animation>
        $<BUILD_INTERFACE:ozz_geometry>
        $<BUILD_INTERFACE:ozz_animation_offline>
        Freetype::Freetype
        harfbuzz::harfbuzz
        msdfgen::msdfgen
        msdfgen::msdfgen-ext
        msdfgen::msdfgen-core
        msdfgen::msdfgen-full
        $<BUILD_INTERFACE:msdf-atlas-gen::msdf-atlas-gen>
        miniz::miniz
        PkgConfig::thorvg)

if (DZ_INSTALL)
    target_compile_definitions(DenOfIzGraphics PUBLIC TVG_STATIC)
endif()
if (BUILD_SHARED_LIBS)
    target_compile_definitions(DenOfIzGraphics PRIVATE DZ_GRAPHICS_EXPORTS)
    target_compile_definitions(DenOfIzGraphics PRIVATE GLOG_EXPORT)
    target_compile_definitions(DenOfIzGraphics INTERFACE DZ_GRAPHICS_IMPORTS)
    target_compile_definitions(DenOfIzGraphics PRIVATE BUILD_SHARED_LIBS)
    if (NOT DZ_INSTALL)
        target_compile_definitions(DenOfIzGraphics PRIVATE STB_IMPLEMENTATION)
    endif()
else ()
    target_compile_definitions(DenOfIzGraphics PUBLIC GLOG_NO_EXPORT)
    target_compile_definitions(DenOfIzGraphics PUBLIC TVG_STATIC)
    target_link_libraries(DenOfIzGraphics
            PRIVATE
            SDL2::SDL2-static
    )
endif ()

if (WIN32)
    set(CMAKE_SKIP_INSTALL_RULES ON)
    add_subdirectory(_ThirdParty/D3D12MemoryAllocator EXCLUDE_FROM_ALL)
    set(CMAKE_SKIP_INSTALL_RULES OFF)

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
            PRIVATE
            dxgi
            d3d12
            dxcompiler
            WIL::WIL
            Microsoft::DirectX-Headers
            Microsoft::DirectX-Guids
            D3D12MemoryAllocator
    )
    
    target_link_libraries(DenOfIzGraphics
            PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/_ThirdParty/MetalShaderConverter/lib/metalirconverter.lib
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
            PRIVATE
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
            PRIVATE
            _ThirdParty/linux_dxc/lib/
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
            _ThirdParty/linux_dxc/include/dxc
    )

    #/usr/bin/ld: relocation R_X86_64_PC32 against symbol `_ZNSt6ranges5views4keysE' can not be used when making a shared object; recompile with -fPIC
    set_target_properties(DenOfIzGraphics PROPERTIES POSITION_INDEPENDENT_CODE ON)
endif ()

if (WIN32 OR LINUX)
    target_compile_definitions(DenOfIzGraphics PUBLIC BUILD_VK)
    target_link_libraries(DenOfIzGraphics PRIVATE
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