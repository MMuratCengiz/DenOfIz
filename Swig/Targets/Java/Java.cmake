set(CURRENT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Targets/Java)

set(SWIG_JAVA_DIR ${CMAKE_BINARY_DIR}/Java)
set(SWIG_JAVA_PROJECT_DIR ${SWIG_JAVA_DIR}/Project)
set(SWIG_JAVA_CXX_DIR ${SWIG_JAVA_DIR}/Swig)
set(SWIG_JAVA_LIB_DIR ${SWIG_JAVA_PROJECT_DIR}/Native)
set(SWIG_JAVA_SOURCE_DIR ${SWIG_JAVA_PROJECT_DIR}/Java)
set(SWIG_JAVA_PACKAGE_DIR ${SWIG_JAVA_SOURCE_DIR}/com/denofiz/graphics)

if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64")
    set(ARCH_NAME "arm64")
else()
    set(ARCH_NAME "x64")
endif()

set(MAVEN_BASE_DIR ${CURRENT_DIRECTORY}/Maven)
set(MAVEN_OUT_DIR ${CURRENT_DIRECTORY}/Maven_Out)
set(MAVEN_SRC_DIR ${MAVEN_BASE_DIR}/src/main/java)
set(MAVEN_SRC_GEN_DIR ${MAVEN_BASE_DIR}/src_gen/main/java)
set(MAVEN_RESOURCES_DIR ${MAVEN_BASE_DIR}/src/main/resources)
set(MAVEN_NATIVE_LIBS_DIR ${MAVEN_RESOURCES_DIR}/native)
set(MAVEN_NATIVE_WIN_DIR ${MAVEN_NATIVE_LIBS_DIR}/windows/${ARCH_NAME})
set(MAVEN_NATIVE_OSX_DIR ${MAVEN_NATIVE_LIBS_DIR}/macos/${ARCH_NAME})
set(MAVEN_NATIVE_LINUX_DIR ${MAVEN_NATIVE_LIBS_DIR}/linux/${ARCH_NAME})

find_package(Java REQUIRED)
find_package(JNI REQUIRED)

set_property(SOURCE DenOfIzGraphics.i PROPERTY COMPILE_OPTIONS "-package" "com.denofiz.graphics")
swig_add_library(DenOfIzGraphicsJava
        TYPE SHARED
        LANGUAGE Java
        SOURCES DenOfIzGraphics.i
        OUTPUT_DIR ${SWIG_JAVA_PACKAGE_DIR}
        OUTFILE_DIR ${SWIG_JAVA_CXX_DIR}
)
set_target_properties(DenOfIzGraphicsJava PROPERTIES
        SWIG_FLAGS "-E;-DSWIGJAVA"
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)

set_target_properties(DenOfIzGraphicsJava PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${SWIG_JAVA_LIB_DIR}"
)

if (APPLE)
    target_compile_options(DenOfIzGraphicsJava PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:-x objective-c++>
            $<$<COMPILE_LANGUAGE:C>:-x objective-c>)
    set_property(TARGET DenOfIzGraphicsJava APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")

    # Add special flags to bypass code signing issues on macOS
    if (CMAKE_OSX_DEPLOYMENT_TARGET)
        set_property(TARGET DenOfIzGraphicsJava APPEND_STRING PROPERTY
            LINK_FLAGS " -Wl,-platform_version,macos,${CMAKE_OSX_DEPLOYMENT_TARGET},${CMAKE_OSX_DEPLOYMENT_TARGET}")
    endif()
endif ()

file(MAKE_DIRECTORY ${SWIG_JAVA_LIB_DIR})
file(MAKE_DIRECTORY ${SWIG_JAVA_PACKAGE_DIR})

set_target_properties(DenOfIzGraphicsJava PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${SWIG_JAVA_LIB_DIR}
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
        ${JNI_INCLUDE_DIRS}
)

add_custom_target(DenOfIzGraphicsJavaCleanup
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${SWIG_JAVA_CXX_DIR}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${SWIG_JAVA_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${SWIG_JAVA_PACKAGE_DIR}
)

add_custom_command(
        TARGET DenOfIzGraphicsJava POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SWIG_JAVA_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SWIG_JAVA_PACKAGE_DIR}
        COMMAND_EXPAND_LISTS
)

# Copy generated library to lib directory
add_custom_command(TARGET DenOfIzGraphicsJava POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DenOfIzGraphicsJava> ${SWIG_JAVA_LIB_DIR}
)

if (BUILD_SHARED_LIBS)
    add_custom_command(TARGET DenOfIzGraphicsJava POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DenOfIzGraphics> ${SWIG_JAVA_LIB_DIR}
    )
else ()
    add_custom_command(TARGET DenOfIzGraphicsJava POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_LINKER_FILE:DenOfIzGraphics> ${SWIG_JAVA_LIB_DIR}
    )
endif ()

add_custom_command(TARGET DenOfIzGraphicsJava POST_BUILD
        COMMENT "Preparing Maven package files..."

        # Create directory structure
        COMMAND ${CMAKE_COMMAND} -E make_directory ${MAVEN_SRC_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${MAVEN_SRC_GEN_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${MAVEN_RESOURCES_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${MAVEN_NATIVE_LIBS_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${MAVEN_NATIVE_WIN_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${MAVEN_NATIVE_OSX_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${MAVEN_NATIVE_LINUX_DIR}

        # Clear previous generated code files only (not src)
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${MAVEN_SRC_GEN_DIR}/com/denofiz/graphics
        COMMAND ${CMAKE_COMMAND} -E make_directory ${MAVEN_SRC_GEN_DIR}/com/denofiz/graphics

        # Copy SWIG-generated files to src_gen
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${SWIG_JAVA_PACKAGE_DIR} ${MAVEN_SRC_GEN_DIR}/com/denofiz/graphics
)

if (WIN32)
    add_custom_command(TARGET DenOfIzGraphicsJava POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:DenOfIzGraphicsJava> ${MAVEN_NATIVE_WIN_DIR}/$<TARGET_FILE_NAME:DenOfIzGraphicsJava>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/dxcompiler.dll ${MAVEN_NATIVE_WIN_DIR}/dxcompiler.dll
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/dxil.dll ${MAVEN_NATIVE_WIN_DIR}/dxil.dll
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/metalirconverter.dll ${MAVEN_NATIVE_WIN_DIR}/metalirconverter.dll
    )

    if (BUILD_SHARED_LIBS)
        add_custom_command(TARGET DenOfIzGraphicsJava POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:DenOfIzGraphics> ${MAVEN_NATIVE_WIN_DIR}/$<TARGET_FILE_NAME:DenOfIzGraphics>
        )
    else ()
        add_custom_command(TARGET DenOfIzGraphicsJava POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_LINKER_FILE:DenOfIzGraphics> ${MAVEN_NATIVE_WIN_DIR}/$<TARGET_LINKER_FILE_NAME:DenOfIzGraphics>
        )
    endif ()

    add_custom_command(TARGET DenOfIzGraphicsJava POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Windows: Maven package files prepared successfully!"
    )
elseif (APPLE)
    add_custom_command(TARGET DenOfIzGraphicsJava POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:DenOfIzGraphicsJava> ${MAVEN_NATIVE_OSX_DIR}/$<TARGET_FILE_NAME:DenOfIzGraphicsJava>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/libdxcompiler.dylib ${MAVEN_NATIVE_OSX_DIR}/libdxcompiler.dylib
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/libmetalirconverter.dylib ${MAVEN_NATIVE_OSX_DIR}/libmetalirconverter.dylib
    )

    if (BUILD_SHARED_LIBS)
        add_custom_command(TARGET DenOfIzGraphicsJava POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:DenOfIzGraphics> ${MAVEN_NATIVE_OSX_DIR}/$<TARGET_FILE_NAME:DenOfIzGraphics>
        )
    else ()
        add_custom_command(TARGET DenOfIzGraphicsJava POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_LINKER_FILE:DenOfIzGraphics> ${MAVEN_NATIVE_OSX_DIR}/$<TARGET_LINKER_FILE_NAME:DenOfIzGraphics>
        )
    endif ()

    add_custom_command(TARGET DenOfIzGraphicsJava POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "macOS: Maven package files prepared successfully!"
    )
else () # Linux
    add_custom_command(TARGET DenOfIzGraphicsJava POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:DenOfIzGraphicsJava> ${MAVEN_NATIVE_LINUX_DIR}/$<TARGET_FILE_NAME:DenOfIzGraphicsJava>
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/libdxcompiler.so ${MAVEN_NATIVE_LINUX_DIR}/libdxcompiler.so
    )

    if (BUILD_SHARED_LIBS)
        add_custom_command(TARGET DenOfIzGraphicsJava POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:DenOfIzGraphics> ${MAVEN_NATIVE_LINUX_DIR}/$<TARGET_FILE_NAME:DenOfIzGraphics>
        )
    else ()
        add_custom_command(TARGET DenOfIzGraphicsJava POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_LINKER_FILE:DenOfIzGraphics> ${MAVEN_NATIVE_LINUX_DIR}/$<TARGET_LINKER_FILE_NAME:DenOfIzGraphics>
        )
    endif ()

    add_custom_command(TARGET DenOfIzGraphicsJava POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Linux: Maven package files prepared successfully!"
    )
endif ()

include(${CURRENT_DIRECTORY}/Maven.cmake)
add_custom_target(DenOfIzJava
    DEPENDS DenOfIzGraphicsJava DenOfIzGraphicsJavaManaged
    COMMENT "Building Java bindings and Maven package"
)