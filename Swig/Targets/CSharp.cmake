set(SWIG_CSHARP_DIR ${CMAKE_BINARY_DIR}/CSharp)
set(SWIG_CSHARP_PROJECT_DIR ${SWIG_CSHARP_DIR}/Project)
set(SWIG_CSHARP_CXX_DIR ${SWIG_CSHARP_DIR}/Swig)
set(SWIG_CSHARP_LIB_DIR ${SWIG_CSHARP_PROJECT_DIR}/Native)

swig_add_library(DenOfIzGraphicsCSharp
        TYPE SHARED
        LANGUAGE CSharp
        SOURCES DenOfIzGraphics.i
        OUTPUT_DIR ${SWIG_CSHARP_PROJECT_DIR}
        OUTFILE_DIR ${SWIG_CSHARP_CXX_DIR}
)

set_target_properties(DenOfIzGraphicsCSharp PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${SWIG_CSHARP_LIB_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${SWIG_CSHARP_LIB_DIR}
        SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE
        CXX_STANDARD_REQUIRED TRUE
)

target_link_libraries(DenOfIzGraphicsCSharp
        PUBLIC
        DenOfIzGraphics
)

add_custom_command(
        TARGET DenOfIzGraphicsCSharp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${SWIG_CSHARP_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SWIG_CSHARP_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:DenOfIzGraphicsCSharp> ${SWIG_CSHARP_LIB_DIR}
        COMMAND_EXPAND_LISTS
)