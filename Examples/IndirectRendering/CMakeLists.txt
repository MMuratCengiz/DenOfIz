include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

SET(DEN_OF_IZ_EXAMPLES_INDIRECT_RENDERING_SOURCES
        main.cpp
        Source/IndirectRenderingExample.cpp
)

add_executable(DenOfIzExamples-IndirectRendering MACOSX_BUNDLE ${DEN_OF_IZ_EXAMPLES_INDIRECT_RENDERING_SOURCES})
copy_to_binary_target("${CMAKE_CURRENT_SOURCE_DIR}/.." "Assets" * DenOfIzExamples-IndirectRendering)
target_include_default_directories(DenOfIzExamples-IndirectRendering)
target_link_libraries(DenOfIzExamples-IndirectRendering
        PUBLIC
        DenOfIzExamples-Base
)
denofiz_app(DenOfIzExamples-IndirectRendering)