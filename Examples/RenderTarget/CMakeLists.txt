include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

SET(DEN_OF_IZ_EXAMPLES_RENDER_TARGET_SOURCES
        main.cpp
        Source/RenderTargetExample.cpp
)

add_executable(DenOfIzExamples-RenderTarget MACOSX_BUNDLE ${DEN_OF_IZ_EXAMPLES_RENDER_TARGET_SOURCES})
copy_to_binary_target("${CMAKE_CURRENT_SOURCE_DIR}/.." "Assets" * DenOfIzExamples-RenderTarget)
target_include_default_directories(DenOfIzExamples-RenderTarget)
target_link_libraries(DenOfIzExamples-RenderTarget
        PUBLIC
        DenOfIzExamples-Base
)
apple_app(DenOfIzExamples-RenderTarget)