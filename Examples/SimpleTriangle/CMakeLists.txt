include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

SET(DEN_OF_IZ_EXAMPLES_SIMPLE_TRIANGLE_SOURCES
        main.cpp
        Source/SimpleTriangleExample.cpp
)

add_executable(DenOfIzExamples-SimpleTriangle MACOSX_BUNDLE ${DEN_OF_IZ_EXAMPLES_SIMPLE_TRIANGLE_SOURCES})
copy_to_binary_target("${CMAKE_CURRENT_SOURCE_DIR}/.." "Assets" * DenOfIzExamples-SimpleTriangle)
target_include_default_directories(DenOfIzExamples-SimpleTriangle)
target_link_libraries(DenOfIzExamples-SimpleTriangle
        PUBLIC
        DenOfIzExamples-Base
)
apple_app(DenOfIzExamples-SimpleTriangle)