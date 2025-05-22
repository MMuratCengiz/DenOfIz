include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

SET(DEN_OF_IZ_EXAMPLES_VG_EXAMPLE_SOURCES
        main.cpp
        Source/VGExample.cpp
)

add_executable(DenOfIzExamples-VGExample MACOSX_BUNDLE ${DEN_OF_IZ_EXAMPLES_VG_EXAMPLE_SOURCES})
copy_to_binary_target("${CMAKE_CURRENT_SOURCE_DIR}/.." "Assets" * DenOfIzExamples-VGExample)
target_include_default_directories(DenOfIzExamples-VGExample)
target_link_libraries(DenOfIzExamples-VGExample
        PUBLIC
        DenOfIzExamples-Base
)
apple_app(DenOfIzExamples-VGExample)