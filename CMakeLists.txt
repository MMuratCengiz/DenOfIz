cmake_minimum_required(VERSION 3.28)

option(CRT_LINKAGE_STATIC "Link MSVC runtime statically" 0)

set(Boost_NO_WARN_NEW_VERSIONS 1)
set(VCPKG_MANIFEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})
include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common.cmake")
include("${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(DenOfIz)

set(CMAKE_CXX_STANDARD 23)

set(SUB_PROJECTS Engine/Core Engine/Graphics)
foreach(SUB_PROJECT IN LISTS SUB_PROJECTS)
    add_subdirectory(${SUB_PROJECT})
endforeach()

add_executable(DenOfIz main.cpp)
target_link_libraries(DenOfIz
        PRIVATE
        DenOfIzGraphics
)
set_target_default_properties(DenOfIz)