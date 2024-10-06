include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

SET(DEN_OF_IZ_EXAMPLES_ROOT_CONSTANT_SOURCES
        main.cpp
        Source/RootConstantExample.cpp
)

add_executable(DenOfIzExamples-RootConstants MACOSX_BUNDLE ${DEN_OF_IZ_EXAMPLES_ROOT_CONSTANT_SOURCES})
copy_to_binary_target("${CMAKE_CURRENT_SOURCE_DIR}/.." "Assets" * DenOfIzExamples-RootConstants)
target_include_default_directories(DenOfIzExamples-RootConstants)
target_link_libraries(DenOfIzExamples-RootConstants
        PUBLIC
        DenOfIzExamples-Base
)

apple_app(DenOfIzExamples-RootConstants)