include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

SET(DEN_OF_IZ_EXAMPLES_RAY_TRACING_SOURCES
        main.cpp
        Source/AnimatedFoxExample.cpp
)

add_executable(DenOfIzExamples-AnimatedFox MACOSX_BUNDLE ${DEN_OF_IZ_EXAMPLES_RAY_TRACING_SOURCES})
copy_to_binary_target("${CMAKE_CURRENT_SOURCE_DIR}/.." "Assets" * DenOfIzExamples-AnimatedFox)
target_include_default_directories(DenOfIzExamples-AnimatedFox)
target_include_directories(DenOfIzExamples-AnimatedFox PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../Assets/Shaders/RTProceduralGeometry)
target_link_libraries(DenOfIzExamples-AnimatedFox
        PUBLIC
        DenOfIzExamples-Base
)
apple_app(DenOfIzExamples-AnimatedFox)