include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

set(BISTRO_DIR "${CMAKE_CURRENT_SOURCE_DIR}/_Assets/niagara_bistro")

if(NOT EXISTS "${BISTRO_DIR}/bistro")
    find_package(Git REQUIRED)
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/_Assets")
    execute_process(COMMAND ${GIT_EXECUTABLE} clone --depth 1 --filter=blob:none --sparse https://github.com/zeux/niagara_bistro "${BISTRO_DIR}")
    execute_process(COMMAND ${GIT_EXECUTABLE} sparse-checkout set bistro WORKING_DIRECTORY "${BISTRO_DIR}")
endif()

SET(DEN_OF_IZ_EXAMPLES_RAY_TRACING_SOURCES
        main.cpp
        Source/Bistro.cpp
        Source/RayTracedSceneExample.cpp
)

add_executable(DenOfIzExamples-RayTracedScene MACOSX_BUNDLE ${DEN_OF_IZ_EXAMPLES_RAY_TRACING_SOURCES})
copy_to_binary_target("${CMAKE_CURRENT_SOURCE_DIR}/.." "Assets" * DenOfIzExamples-RayTracedScene)
target_include_default_directories(DenOfIzExamples-RayTracedScene)
target_link_libraries(DenOfIzExamples-RayTracedScene
        PUBLIC
        DenOfIzExamples-Base
)
denofiz_app(DenOfIzExamples-RayTracedScene)

target_compile_definitions(DenOfIzExamples-RayTracedScene PRIVATE
    DZ_EXAMPLES_BISTRO_DIR="${BISTRO_DIR}"
)