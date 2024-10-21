include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

SET(DEN_OF_IZ_EXAMPLES_RAY_TRACING_SOURCES
        main.cpp
        Source/RayTracedTriangleExample.cpp
)

add_executable(DenOfIzExamples-RayTracedTriangle MACOSX_BUNDLE ${DEN_OF_IZ_EXAMPLES_RAY_TRACING_SOURCES})
copy_to_binary_target("${CMAKE_CURRENT_SOURCE_DIR}/.." "Assets" * DenOfIzExamples-RayTracedTriangle)
target_include_default_directories(DenOfIzExamples-RayTracedTriangle)
target_link_libraries(DenOfIzExamples-RayTracedTriangle
        PUBLIC
        DenOfIzExamples-Base
)
apple_app(DenOfIzExamples-RayTracedTriangle)