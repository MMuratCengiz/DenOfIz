include(${PROJECT_SOURCE_DIR}/cmake/common.cmake)

SET(DEN_OF_IZ_EXAMPLES_BINDLESS_SOURCES
        main.cpp
        Source/BindlessExample.cpp
)

add_executable(DenOfIzExamples-Bindless MACOSX_BUNDLE ${DEN_OF_IZ_EXAMPLES_BINDLESS_SOURCES})
copy_to_binary_target("${CMAKE_CURRENT_SOURCE_DIR}/.." "Assets" * DenOfIzExamples-Bindless)
target_include_default_directories(DenOfIzExamples-Bindless)
target_link_libraries(DenOfIzExamples-Bindless
        PUBLIC
        DenOfIzExamples-Base
)
apple_app(DenOfIzExamples-Bindless)