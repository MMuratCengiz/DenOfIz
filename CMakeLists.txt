cmake_minimum_required(VERSION 3.17.2)

if (UNIX AND NOT APPLE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH FALSE)
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
    set(CMAKE_BUILD_RPATH "${CMAKE_BINARY_DIR}:${CMAKE_CURRENT_SOURCE_DIR}/Graphics/_ThirdParty/dxc/linux/lib")
endif ()

set(LANGUAGES C CXX)
if (APPLE)
    set(LANGUAGES C CXX OBJCXX OBJC)
endif ()
set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)
project(DenOfIz LANGUAGES ${LANGUAGES})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/utf-8)
endif ()

option(CRT_LINKAGE_STATIC "Link MSVC runtime statically" 1)
option(BUILD_TESTS OFF)
option(BUILD_EXAMPLES OFF)
option(BUILD_SHARED_LIBS OFF)
option(BUILD_TOOLS OFF)
option(BUILD_BINDINGS "Build DenOfIzGraphics Bindings" OFF)

set(VCPKG_MANIFEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

add_subdirectory(Graphics)

if (BUILD_BINDINGS)
    add_subdirectory(Bindings)
endif()
if (BUILD_EXAMPLES)
    add_subdirectory(Examples)
endif ()

include(CPack)
