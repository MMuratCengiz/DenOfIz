include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

SET(DEN_OF_IZ_EXAMPLES_TRANSPARENCY_SOURCES
        main.cpp
        Source/TransparencyExample.cpp
        Source/ColoredSphereAsset.cpp
        Source/ColoredSpherePipeline.cpp
)

add_executable(DenOfIzExamples-Transparency MACOSX_BUNDLE ${DEN_OF_IZ_EXAMPLES_TRANSPARENCY_SOURCES})
copy_to_binary_target("${CMAKE_CURRENT_SOURCE_DIR}/.." "Assets" * DenOfIzExamples-Transparency)
target_include_default_directories(DenOfIzExamples-Transparency)
target_link_libraries(DenOfIzExamples-Transparency
        PUBLIC
        DenOfIzExamples-Base
)
apple_app(DenOfIzExamples-Transparency)