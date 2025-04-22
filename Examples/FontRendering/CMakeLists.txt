include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

SET(DEN_OF_IZ_EXAMPLES_FONT_RENDERING_SOURCES
        main.cpp
        Source/FontRenderingExample.cpp
)

add_executable(DenOfIzExamples-FontRendering MACOSX_BUNDLE ${DEN_OF_IZ_EXAMPLES_FONT_RENDERING_SOURCES})
copy_to_binary_target("${CMAKE_CURRENT_SOURCE_DIR}/.." "Assets" * DenOfIzExamples-FontRendering)
target_include_default_directories(DenOfIzExamples-FontRendering)
target_link_libraries(DenOfIzExamples-FontRendering
        PUBLIC
        DenOfIzExamples-Base
)

apple_app(DenOfIzExamples-FontRendering)