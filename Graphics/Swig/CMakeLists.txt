option(SWIG_CSHARP "Build DenOfIzGraphics csharp bindings" ON)

if (SWIG_CSHARP)
    cmake_policy(SET CMP0122 NEW)
    find_package(SWIG REQUIRED)
    include(${SWIG_USE_FILE})
endif ()

set_source_files_properties(DenOfIzGraphics.i PROPERTIES CPLUSPLUS ON)
#set_source_files_properties(DenOfIzGraphics.i PROPERTIES SWIG_FLAGS "-includeall")
swig_add_library(DenOfIzGraphicsCSharp
        LANGUAGE csharp
        SOURCES DenOfIzGraphics.i
)

set_target_properties(DenOfIzGraphicsCSharp PROPERTIES
        SWIG_OUTFILE_DIR ${CMAKE_BINARY_DIR}/swig_output
        CSharp_BINDIR ${CMAKE_BINARY_DIR}/csharp_output
        SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE
        CXX_STANDARD_REQUIRED TRUE
)

target_link_libraries(DenOfIzGraphicsCSharp
        PUBLIC
        DenOfIzGraphics
)