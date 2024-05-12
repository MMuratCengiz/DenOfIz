cmake_minimum_required(VERSION 3.28)
project(DenOfIz LANGUAGES C CXX OBJCXX OBJC)

option(CRT_LINKAGE_STATIC "Link MSVC runtime statically" 1)

set(Boost_NO_WARN_NEW_VERSIONS 1)
set(VCPKG_MANIFEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

#Engine/Core Engine/Graphics
set(SUB_PROJECTS Engine/The-Forge Game)
foreach(SUB_PROJECT IN LISTS SUB_PROJECTS)
    add_subdirectory(${SUB_PROJECT})
endforeach()

add_executable(DenOfIz MACOSX_BUNDLE main.cpp)

set_target_properties(DenOfIz PROPERTIES
        BUNDLE True
        MACOSX_BUNDLE_GUI_IDENTIFIER mmuratcengiz.DenOfIz
        MACOSX_BUNDLE_EXECUTABLE_NAME DenOfIz
        MACOSX_BUNDLE_BUNDLE_NAME DenOfIz
        MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
        MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/cmake/MacOSBundleInfo.plist.in
)

target_link_libraries(DenOfIz
        PRIVATE
# DenOfIzGraphics
        DenOfIzGame
)