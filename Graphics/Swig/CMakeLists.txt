option(SWIG_CSHARP "Build DenOfIzGraphics csharp bindings" ON)

cmake_policy(SET CMP0122 NEW)
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

set_source_files_properties(DenOfIzGraphics.i PROPERTIES CPLUSPLUS ON)

swig_add_library(DenOfIzGraphicsCSharp
        TYPE OBJECT
        OUTPUT_DIR ${CMAKE_BINARY_DIR}/CSharp/Source
        LANGUAGE csharp
        SOURCES DenOfIzGraphics.i
)

set_target_properties(DenOfIzGraphicsCSharp PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/CSharp
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/CSharp
        LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/CSharp
        SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE
        CXX_STANDARD_REQUIRED TRUE
)

target_link_libraries(DenOfIzGraphicsCSharp
        PUBLIC
        DenOfIzGraphics
)