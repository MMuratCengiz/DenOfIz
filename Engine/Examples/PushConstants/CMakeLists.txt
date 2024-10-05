include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

SET(DEN_OF_IZ_EXAMPLES_PUSH_CONSTANT_SOURCES
        main.cpp
        Source/PushConstantExample.cpp
)

add_executable(DenOfIzExamples-PushConstants MACOSX_BUNDLE ${DEN_OF_IZ_EXAMPLES_PUSH_CONSTANT_SOURCES})
copy_to_binary_target("${CMAKE_CURRENT_SOURCE_DIR}/.." "Assets" * DenOfIzExamples-PushConstants)
target_include_default_directories(DenOfIzExamples-PushConstants)
target_link_libraries(DenOfIzExamples-PushConstants
        PUBLIC
        DenOfIzExamples-Base
)

apple_app(DenOfIzExamples-PushConstants)