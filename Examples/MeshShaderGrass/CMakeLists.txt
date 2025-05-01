include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

SET(DEN_OF_IZ_EXAMPLES_MESH_SHADER_GRASS_SOURCES
        main.cpp
        Source/MeshShaderGrassExample.cpp
)

add_executable(DenOfIzExamples-MeshShaderGrassExample MACOSX_BUNDLE ${DEN_OF_IZ_EXAMPLES_MESH_SHADER_GRASS_SOURCES})
copy_to_binary_target("${CMAKE_CURRENT_SOURCE_DIR}/.." "Assets" * DenOfIzExamples-MeshShaderGrassExample)
target_include_default_directories(DenOfIzExamples-MeshShaderGrassExample)
target_link_libraries(DenOfIzExamples-MeshShaderGrassExample
        PUBLIC
        DenOfIzExamples-Base
)
apple_app(DenOfIzExamples-MeshShaderGrassExample)