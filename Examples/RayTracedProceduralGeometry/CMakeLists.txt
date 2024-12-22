include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

SET(DEN_OF_IZ_EXAMPLES_RAY_TRACING_SOURCES
        main.cpp
        Source/RayTracedProceduralGeometryExample.cpp
)

add_executable(DenOfIzExamples-RayTracedProceduralGeometry MACOSX_BUNDLE ${DEN_OF_IZ_EXAMPLES_RAY_TRACING_SOURCES})
copy_to_binary_target("${CMAKE_CURRENT_SOURCE_DIR}/.." "Assets" * DenOfIzExamples-RayTracedProceduralGeometry)
target_include_default_directories(DenOfIzExamples-RayTracedProceduralGeometry)
target_link_libraries(DenOfIzExamples-RayTracedProceduralGeometry
        PUBLIC
        DenOfIzExamples-Base
)
apple_app(DenOfIzExamples-RayTracedProceduralGeometry)