cmake_policy(SET CMP0122 NEW)
find_package(SWIG REQUIRED)
include(${SWIG_USE_FILE})

set_source_files_properties(DenOfIzGraphics.i PROPERTIES CPLUSPLUS ON)

swig_add_library(DenOfIzGraphicsCSharp
        TYPE SHARED
        LANGUAGE CSharp
        SOURCES DenOfIzGraphics.i
        OUTPUT_DIR ${CMAKE_BINARY_DIR}/CSharp
        OUTFILE_DIR ${CMAKE_BINARY_DIR}/SwigCSharp
)

set_target_properties(DenOfIzGraphicsCSharp PROPERTIES
        SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE
        CXX_STANDARD_REQUIRED TRUE
)

target_link_libraries(DenOfIzGraphicsCSharp
        PUBLIC
        DenOfIzGraphics
)

#swig_add_library(DenOfIzGraphicsJava
#        TYPE SHARED
#        LANGUAGE Java
#        SOURCES DenOfIzGraphics.i
#        OUTPUT_DIR ${CMAKE_BINARY_DIR}/Java
#        OUTFILE_DIR ${CMAKE_BINARY_DIR}/SwigJava
#)
#set_target_properties(DenOfIzGraphicsJava PROPERTIES
#        SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE
#        CXX_STANDARD_REQUIRED TRUE
#)

#target_link_libraries(DenOfIzGraphicsJava
#        PUBLIC
#        DenOfIzGraphics
#)
#
#target_include_directories(DenOfIzGraphicsJava
#        PUBLIC
#        "C:/Lib/jdk-23/include")
#
#target_include_directories(DenOfIzGraphicsJava
#        PUBLIC
#        "C:/Lib/jdk-23/include/win32")