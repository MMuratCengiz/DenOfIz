cmake_minimum_required(VERSION 3.17.2)

if (UNIX AND NOT APPLE)
    set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
    set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
    set(CMAKE_INSTALL_RPATH "$ORIGIN")
endif ()

set(LANGUAGES C CXX)
if (APPLE)
    set(LANGUAGES C CXX OBJCXX OBJC)
endif ()
set(X_VCPKG_APPLOCAL_DEPS_INSTALL ON)
project(DenOfIz LANGUAGES ${LANGUAGES})

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(CRT_LINKAGE_STATIC "Link MSVC runtime statically" 1)
option(BUILD_TESTS OFF)
option(BUILD_EXAMPLES OFF)
option(BUILD_SHARED_LIBS OFF)
option(SWIG_CSHARP "Build DenOfIzGraphics csharp bindings" OFF)

set(Boost_NO_WARN_NEW_VERSIONS 1)
set(VCPKG_MANIFEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

set(SUB_PROJECTS Graphics Swig)
foreach (SUB_PROJECT IN LISTS SUB_PROJECTS)
    add_subdirectory(${SUB_PROJECT})
endforeach ()

if (BUILD_EXAMPLES)
    add_subdirectory(Examples)
endif ()

set(SOURCES main.cpp)
if (APPLE)
    set(SOURCES main.cpp ${PROJECT_SOURCE_DIR}/OS/MacOS/MainMenu.xib)
endif ()

if (APPLE)
    add_executable(DenOfIz MACOSX_BUNDLE ${SOURCES})
else()
    add_executable(DenOfIz ${SOURCES})
endif()

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
    set_target_properties(DenOfIz PROPERTIES XCODE_ATTRIBUTE_OTHER_CODE_SIGN_FLAGS "--deep")
endif ()

target_link_libraries(DenOfIz
        PUBLIC
        DenOfIzGraphics
)

if (APPLE)
    # Required for the forge, not necessary otherwise.
    target_compile_options(DenOfIz PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:-x objective-c++>
            $<$<COMPILE_LANGUAGE:C>:-x objective-c>)
endif ()

include(CPack)