include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

SET(DEN_OF_IZ_EXAMPLES_ANIMATED_FOX_SOURCES
        main.cpp
        Source/AnimatedFoxExample.cpp
)

add_executable(DenOfIzExamples-AnimatedFox MACOSX_BUNDLE ${DEN_OF_IZ_EXAMPLES_ANIMATED_FOX_SOURCES})
copy_to_binary_target("${CMAKE_CURRENT_SOURCE_DIR}/.." "Assets" * DenOfIzExamples-AnimatedFox)
target_include_default_directories(DenOfIzExamples-AnimatedFox)

# Find and include ozz-animation libraries
set(OZZ_ROOT "${PROJECT_SOURCE_DIR}/Graphics/_ThirdParty/ozz-animation")
target_include_directories(DenOfIzExamples-AnimatedFox PUBLIC
        ${OZZ_ROOT}/include
        ${OZZ_ROOT}/src
)

target_link_libraries(DenOfIzExamples-AnimatedFox
        PUBLIC
        DenOfIzExamples-Base
        ozz_animation
        ozz_animation_offline
        ozz_base
)

apple_app(DenOfIzExamples-AnimatedFox)