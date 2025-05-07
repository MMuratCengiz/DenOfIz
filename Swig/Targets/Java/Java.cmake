set(SWIG_JAVA_DIR ${CMAKE_BINARY_DIR}/Java)
set(SWIG_JAVA_PROJECT_DIR ${SWIG_JAVA_DIR}/Project)
set(SWIG_JAVA_CXX_DIR ${SWIG_JAVA_DIR}/Swig)
set(SWIG_JAVA_LIB_DIR ${SWIG_JAVA_PROJECT_DIR}/Native)

swig_add_library(DenOfIzGraphicsJava
        TYPE SHARED
        LANGUAGE Java
        SOURCES DenOfIzGraphics.i
        OUTPUT_DIR ${SWIG_JAVA_DIR}
        OUTFILE_DIR ${SWIG_JAVA_CXX_DIR}
)

set_target_properties(DenOfIzGraphicsJava PROPERTIES
        SWIG_FLAGS "-E;-DSWIGJAVA"
)

set_target_properties(DenOfIzGraphicsJava PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY ${SWIG_JAVA_LIB_DIR}
        SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE
        CXX_STANDARD_REQUIRED TRUE
)

target_link_libraries(DenOfIzGraphicsJava
        PUBLIC
        DenOfIzGraphics
)

target_include_directories(DenOfIzGraphicsJava
        PUBLIC
        "C:/Lib/jdk-23/include")

target_include_directories(DenOfIzGraphicsJava
        PUBLIC
        "C:/Lib/jdk-23/include/win32")