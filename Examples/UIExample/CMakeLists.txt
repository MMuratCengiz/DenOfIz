include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

SET(DEN_OF_IZ_EXAMPLES_UI_EXAMPLE_SOURCES
        main.cpp
        Source/UIExample.cpp
        Source/Spinning3DCubeWidget.cpp
)

add_executable(DenOfIzExamples-UIExample MACOSX_BUNDLE ${DEN_OF_IZ_EXAMPLES_UI_EXAMPLE_SOURCES})
copy_to_binary_target("${CMAKE_CURRENT_SOURCE_DIR}/.." "Assets" * DenOfIzExamples-UIExample)
target_include_default_directories(DenOfIzExamples-UIExample)
target_link_libraries(DenOfIzExamples-UIExample
        PUBLIC
        DenOfIzExamples-Base
)
apple_app(DenOfIzExamples-UIExample)