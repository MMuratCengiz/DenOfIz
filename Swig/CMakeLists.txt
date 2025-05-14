cmake_policy(SET CMP0122 NEW)

file(READ "${CMAKE_CURRENT_SOURCE_DIR}/../version.txt" VERSION_CONTENT)
string(STRIP "${VERSION_CONTENT}" DENOFIZ_VERSION)
string(TIMESTAMP CURRENT_YEAR "%Y")

find_package(Java)
find_package(JNI)

if (NOT Java_FOUND OR NOT JNI_FOUND)
    set(SWIG_JAVA OFF)
    message(WARNING "Java or JNI not found. SWIG_JAVA will be disabled.")
endif ()

if (SWIG_CSHARP OR SWIG_JAVA)
    find_package(SWIG REQUIRED)
    include(${SWIG_USE_FILE})
    set_source_files_properties(DenOfIzGraphics.i PROPERTIES CPLUSPLUS ON)
    set_source_files_properties(DenOfIzGraphics_Input.i PROPERTIES CPLUSPLUS ON)
endif ()

if (SWIG_CSHARP)
    include(Targets/CSharp/CSharp.cmake)

    if (APPLE)
        set_target_properties(DenOfIzGraphicsCSharp PROPERTIES
            INSTALL_RPATH "@loader_path"
            BUILD_WITH_INSTALL_RPATH TRUE)
    endif()
endif ()

if (SWIG_JAVA)
    include(Targets/Java/Java.cmake)
endif ()