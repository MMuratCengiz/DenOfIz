# This cmake file is used as an include from Swig
set(CURRENT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Targets/CSharp)

set(SWIG_CSHARP_DIR ${CMAKE_BINARY_DIR}/CSharp)
set(SWIG_CSHARP_CXX_DIR ${SWIG_CSHARP_DIR}/Swig)
set(SWIG_CSHARP_PROJECT_DIR ${SWIG_CSHARP_DIR}/Project)
set(SWIG_CSHARP_CODE_DIR ${SWIG_CSHARP_PROJECT_DIR}/Code)
set(SWIG_CSHARP_NATIVE_DIR ${SWIG_CSHARP_PROJECT_DIR}/Native)
set(SWIG_CSHARP_LIB_DIR ${SWIG_CSHARP_PROJECT_DIR}/Lib)

# NuGet package directories
set(NUGET_BASE_DIR ${CURRENT_DIRECTORY}/NuGet)
set(NUGET_OUT_DIR ${CURRENT_DIRECTORY}/NuGet_Out)
set(NUGET_LIB_DIR ${NUGET_BASE_DIR}/lib/netstandard2.0)
set(NUGET_SRC_GEN_DIR ${NUGET_BASE_DIR}/src_gen)
set(NUGET_RUNTIMES_WIN_DIR ${NUGET_BASE_DIR}/runtimes/win-x64/native)
set(NUGET_RUNTIMES_OSX_DIR ${NUGET_BASE_DIR}/runtimes/osx-x64/native)
set(NUGET_RUNTIMES_LINUX_DIR ${NUGET_BASE_DIR}/runtimes/linux-x64/native)
set(NUGET_BUILD_DIR ${NUGET_BASE_DIR}/build)

swig_add_library(DenOfIzGraphicsCSharp
        TYPE SHARED
        LANGUAGE CSharp
        SOURCES DenOfIzGraphics.i
        OUTPUT_DIR ${SWIG_CSHARP_CODE_DIR}
        OUTFILE_DIR ${SWIG_CSHARP_CXX_DIR}
)
set_target_properties(DenOfIzGraphicsCSharp PROPERTIES
        SWIG_FLAGS "-E;-DSWIGCSHARP"
        CXX_STANDARD 23
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
)
set_target_properties(DenOfIzGraphicsCSharp PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/CSharp/Project/Lib"
)
if (APPLE)
    target_compile_options(DenOfIzGraphicsCSharp PRIVATE
            $<$<COMPILE_LANGUAGE:CXX>:-x objective-c++>
            $<$<COMPILE_LANGUAGE:C>:-x objective-c>)
    set_property(TARGET DenOfIzGraphicsCSharp APPEND_STRING PROPERTY COMPILE_FLAGS "-fobjc-arc")
endif()

file(MAKE_DIRECTORY ${SWIG_CSHARP_LIB_DIR})

set_target_properties(DenOfIzGraphicsCSharp PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${SWIG_CSHARP_LIB_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${SWIG_CSHARP_LIB_DIR}
        SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE
        CXX_STANDARD_REQUIRED TRUE
)

swig_link_libraries(DenOfIzGraphicsCSharp DenOfIzGraphics)

add_custom_target(DenOfIzGraphicsCSharpCleanup
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${SWIG_CSHARP_CXX_DIR}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${SWIG_CSHARP_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${SWIG_CSHARP_NATIVE_DIR}
)

add_custom_command(
        TARGET DenOfIzGraphicsCSharp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SWIG_CSHARP_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SWIG_CSHARP_NATIVE_DIR}
        COMMAND_EXPAND_LISTS
)

add_custom_command(TARGET DenOfIzGraphicsCSharp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DenOfIzGraphicsCSharp> ${SWIG_CSHARP_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DenOfIzGraphics> ${SWIG_CSHARP_NATIVE_DIR}
)

if (WIN32)
    add_custom_command(TARGET DenOfIzGraphicsCSharp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:DenOfIzGraphicsCSharp> ${SWIG_CSHARP_NATIVE_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/dxcompiler.dll ${SWIG_CSHARP_NATIVE_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/dxil.dll ${SWIG_CSHARP_NATIVE_DIR}
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/metalirconverter.dll ${SWIG_CSHARP_NATIVE_DIR}
    )
endif ()
if (APPLE)
    # Todo find a way to implement: COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:DenOfIzGraphicsCSharp> ${SWIG_CSHARP_NATIVE_DIR}
    add_custom_command(TARGET DenOfIzGraphicsCSharp POST_BUILD
            # COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/libdxcompiler.dylib ${SWIG_CSHARP_NATIVE_DIR} # dxcompiler is statically built on Osx
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/libmetalirconverter.dylib ${SWIG_CSHARP_NATIVE_DIR}
    )
endif ()
if (LINUX)
    # Todo find a way to implement: COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:DenOfIzGraphicsCSharp> ${SWIG_CSHARP_NATIVE_DIR}
    add_custom_command(TARGET DenOfIzGraphicsCSharp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/libdxcompiler.so ${SWIG_CSHARP_NATIVE_DIR}
    )
endif ()

add_custom_command(TARGET DenOfIzGraphicsCSharp POST_BUILD
        COMMENT "Preparing NuGet package files..."

        # Create NuGet package directories
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${NUGET_SRC_GEN_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_SRC_GEN_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_RUNTIMES_WIN_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_RUNTIMES_OSX_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_RUNTIMES_LINUX_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_BUILD_DIR}

        # Copy generated code
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${SWIG_CSHARP_CODE_DIR} ${NUGET_SRC_GEN_DIR}
)

if (WIN32)
    add_custom_command(TARGET DenOfIzGraphicsCSharp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:DenOfIzGraphicsCSharp> ${NUGET_RUNTIMES_WIN_DIR}/DenOfIzGraphicsCSharp.dll
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:DenOfIzGraphics> ${NUGET_RUNTIMES_WIN_DIR}/DenOfIzGraphics.dll
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/dxcompiler.dll ${NUGET_RUNTIMES_WIN_DIR}/dxcompiler.dll
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/dxil.dll ${NUGET_RUNTIMES_WIN_DIR}/dxil.dll
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/metalirconverter.dll ${NUGET_RUNTIMES_WIN_DIR}/metalirconverter.dll
            COMMAND ${CMAKE_COMMAND} -E echo "Windows: NuGet package files prepared successfully!"
    )
elseif (APPLE)
    add_custom_command(TARGET DenOfIzGraphicsCSharp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:DenOfIzGraphicsCSharp> ${NUGET_RUNTIMES_OSX_DIR}/libDenOfIzGraphicsCSharp.dylib
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:DenOfIzGraphics> ${NUGET_RUNTIMES_OSX_DIR}/libDenOfIzGraphics.dylib
            # COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/libdxcompiler.dylib ${NUGET_RUNTIMES_OSX_DIR}/libdxcompiler.dylib # Statically built
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/libmetalirconverter.dylib ${NUGET_RUNTIMES_OSX_DIR}/libmetalirconverter.dylib
            COMMAND ${CMAKE_COMMAND} -E echo "macOS: NuGet package files prepared successfully!"
    )
else () # Linux
    add_custom_command(TARGET DenOfIzGraphicsCSharp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:DenOfIzGraphicsCSharp> ${NUGET_RUNTIMES_LINUX_DIR}/libDenOfIzGraphicsCSharp.so
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:DenOfIzGraphics> ${NUGET_RUNTIMES_LINUX_DIR}/libDenOfIzGraphics.so
            COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/libdxcompiler.so ${NUGET_RUNTIMES_LINUX_DIR}/libdxcompiler.so
            COMMAND ${CMAKE_COMMAND} -E echo "Linux: NuGet package files prepared successfully!"
    )
endif ()

add_custom_target(DenOfIzGraphicsCSharpManaged
        COMMAND dotnet build ./DenOfIzGraphics.csproj -c Release --output lib/netstandard2.0
        WORKING_DIRECTORY "${NUGET_BASE_DIR}"
        DEPENDS DenOfIzGraphicsCSharp
)

include(${CURRENT_DIRECTORY}/Nuget_Exe.cmake)
add_custom_target(DenOfIzNuget
        COMMAND ${CMAKE_COMMAND} -E echo "Building NuGet package..."
        COMMAND ${NUGET_COMMAND} pack ${NUGET_BASE_DIR}/DenOfIzGraphics.nuspec -Version ${DENOFIZ_VERSION} -OutputDirectory ${NUGET_OUT_DIR}
        DEPENDS DenOfIzGraphicsCSharp DenOfIzGraphicsCSharpManaged
        WORKING_DIRECTORY ${NUGET_BASE_DIR}
)

add_custom_command(TARGET DenOfIzNuget POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "NuGet package has been created successfully at ${NUGET_OUT_DIR}/DenOfIzGraphics.${DENOFIZ_VERSION}.nupkg"
)
