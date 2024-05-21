cmake_minimum_required(VERSION 3.28)

set(LANGUAGES C CXX)
if (APPLE)
    set(LANGUAGES C CXX OBJCXX OBJC)
endif ()

project(DenOfIz LANGUAGES ${LANGUAGES})

option(CRT_LINKAGE_STATIC "Link MSVC runtime statically" 1)

set(Boost_NO_WARN_NEW_VERSIONS 1)
set(VCPKG_MANIFEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

set(SUB_PROJECTS Engine/Core Engine/Graphics Engine/The-Forge Game)
foreach(SUB_PROJECT IN LISTS SUB_PROJECTS)
    add_subdirectory(${SUB_PROJECT})
endforeach()

set(SOURCES main.cpp)
if (APPLE)
    set(SOURCES main.cpp
            ${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Darwin/macOSAppDelegate.h
            ${PROJECT_SOURCE_DIR}/_External/The-Forge/Common_3/OS/Darwin/macOSAppDelegate.m
            ${PROJECT_SOURCE_DIR}/OS/MacOS/MainMenu.xib
    )
endif ()

add_executable(DenOfIz MACOSX_BUNDLE ${SOURCES})


if (APPLE)
    FILE(GLOB FONTS Game/Assets/Fonts/*)
    FILE(GLOB SHADERS ${PROJECT_SOURCE_DIR}/Game/Shaders/*)

    set_target_properties(DenOfIz PROPERTIES
            BUNDLE True
            XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_WEAK YES
            MACOSX_BUNDLE_GUI_IDENTIFIER mmuratcengiz.DenOfIz
            MACOSX_BUNDLE_EXECUTABLE_NAME DenOfIz
            MACOSX_BUNDLE_BUNDLE_NAME DenOfIz
            MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
            MACOSX_BUNDLE_INFO_PLIST ${PROJECT_SOURCE_DIR}/OS/MacOS/MacOSBundleInfo.plist.in
            RESOURCE ${PROJECT_SOURCE_DIR}/OS/MacOS/MainMenu.xib
    )
endif ()

target_link_libraries(DenOfIz
        PRIVATE
        DenOfIzGraphics
        DenOfIzGame
)