SET(DEN_OF_IZ_EXAMPLES_RENDER_TARGET_SOURCES
        main.cpp
        Source/RenderTargetExample.cpp
)

add_executable(DenOfIzExamples-RenderTarget ${DEN_OF_IZ_EXAMPLES_RENDER_TARGET_SOURCES})
copy_to_binary(${CMAKE_CURRENT_SOURCE_DIR} "Assets" *)
target_include_default_directories(DenOfIzExamples-RenderTarget)
target_link_libraries(DenOfIzExamples-RenderTarget
        PUBLIC
        DenOfIzExamples-Base
)