cmake_policy(SET CMP0122 NEW)

set(DENOFIZ_VERSION "1.0.0")
string(TIMESTAMP CURRENT_YEAR "%Y")

if (SWIG_CSHARP OR SWIG_JAVA)
    find_package(SWIG REQUIRED)
    include(${SWIG_USE_FILE})
    set_source_files_properties(DenOfIzGraphics.i PROPERTIES CPLUSPLUS ON)
    set_source_files_properties(DenOfIzGraphics_Dxc.i PROPERTIES CPLUSPLUS ON)
    set_source_files_properties(DenOfIzGraphics_Input.i PROPERTIES CPLUSPLUS ON)
endif ()

if (SWIG_CSHARP)
    include(Targets/CSharp/CSharp.cmake)
endif ()

if (SWIG_JAVA)
    include(Targets/Java/Java.cmake)
endif ()