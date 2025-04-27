include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

SET(DEN_OF_IZ_EXAMPLES_FONT_RENDERING_SOURCES
        main.cpp
        Source/TextRenderingExample.cpp
)

add_executable(DenOfIzExamples-TextRendering MACOSX_BUNDLE ${DEN_OF_IZ_EXAMPLES_FONT_RENDERING_SOURCES})
copy_to_binary_target("${CMAKE_CURRENT_SOURCE_DIR}/.." "Assets" * DenOfIzExamples-TextRendering)
target_include_default_directories(DenOfIzExamples-TextRendering)
target_link_libraries(DenOfIzExamples-TextRendering
        PUBLIC
        DenOfIzExamples-Base
)

apple_app(DenOfIzExamples-TextRendering)