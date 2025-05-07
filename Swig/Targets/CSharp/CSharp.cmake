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
set(NUGET_LIB_DIR ${NUGET_BASE_DIR}/lib/netstandard2.0)
set(NUGET_RUNTIMES_WIN_DIR ${NUGET_BASE_DIR}/runtimes/win-x64/native)
set(NUGET_RUNTIMES_OSX_DIR ${NUGET_BASE_DIR}/runtimes/osx-x64/native)
set(NUGET_RUNTIMES_LINUX_DIR ${NUGET_BASE_DIR}/runtimes/linux-x64/native)
set(NUGET_BUILD_DIR ${NUGET_BASE_DIR}/build)

# Make sure the NuGet directories exist
file(MAKE_DIRECTORY ${NUGET_LIB_DIR})
file(MAKE_DIRECTORY ${NUGET_RUNTIMES_WIN_DIR})
file(MAKE_DIRECTORY ${NUGET_RUNTIMES_OSX_DIR})
file(MAKE_DIRECTORY ${NUGET_RUNTIMES_LINUX_DIR})
file(MAKE_DIRECTORY ${NUGET_BUILD_DIR})

swig_add_library(DenOfIzGraphicsCSharp
        TYPE SHARED
        LANGUAGE CSharp
        SOURCES DenOfIzGraphics.i
        OUTPUT_DIR ${SWIG_CSHARP_CODE_DIR}
        OUTFILE_DIR ${SWIG_CSHARP_CXX_DIR}
)
set_target_properties(DenOfIzGraphicsCSharp PROPERTIES
        SWIG_FLAGS "-E;-DSWIGCSHARP"
)

set_target_properties(DenOfIzGraphicsCSharp PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${SWIG_CSHARP_LIB_DIR}
        LIBRARY_OUTPUT_DIRECTORY ${SWIG_CSHARP_LIB_DIR}
        SWIG_USE_TARGET_INCLUDE_DIRECTORIES TRUE
        CXX_STANDARD_REQUIRED TRUE
)

swig_link_libraries(DenOfIzGraphicsCSharp DenOfIzGraphics)

add_custom_command(
        TARGET DenOfIzGraphicsCSharp PRE_BUILD
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${SWIG_CSHARP_CXX_DIR}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${SWIG_CSHARP_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E remove_directory ${SWIG_CSHARP_NATIVE_DIR}
)

add_custom_command(
        TARGET DenOfIzGraphicsCSharp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SWIG_CSHARP_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SWIG_CSHARP_NATIVE_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:DenOfIzGraphicsCSharp> ${SWIG_CSHARP_NATIVE_DIR}
        COMMAND_EXPAND_LISTS
)

add_custom_command(TARGET DenOfIzGraphicsCSharp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DenOfIzGraphicsCSharp> ${SWIG_CSHARP_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DenOfIzGraphics> ${SWIG_CSHARP_NATIVE_DIR}
)

add_custom_command(TARGET DenOfIzGraphicsCSharp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/dxcompiler.dll ${SWIG_CSHARP_NATIVE_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/dxil.dll ${SWIG_CSHARP_NATIVE_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CMAKE_BINARY_DIR}/metalirconverter.dll ${SWIG_CSHARP_NATIVE_DIR}
)

# Store the version info for NuGet
set(VERSION_FILE ${CURRENT_DIRECTORY}/../../version.txt)
file(WRITE ${VERSION_FILE} "1.0.0")

# Add NuGet package creation as a post-build step
add_custom_command(TARGET DenOfIzGraphicsCSharp POST_BUILD
        COMMENT "Preparing NuGet package files..."
        
        # Create NuGet package directories
        COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_RUNTIMES_WIN_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_RUNTIMES_OSX_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_RUNTIMES_LINUX_DIR}
        COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_BUILD_DIR}
        
        # Copy managed assembly to lib/netstandard2.0
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                $<TARGET_FILE:DenOfIzGraphicsCSharp> 
                ${NUGET_LIB_DIR}/DenOfIzGraphics.dll
        
        # Copy native libraries to appropriate runtime directories
        # Windows native libraries
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                $<TARGET_FILE:DenOfIzGraphics> 
                ${NUGET_RUNTIMES_WIN_DIR}/DenOfIzGraphics.dll
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                ${CMAKE_BINARY_DIR}/dxcompiler.dll 
                ${NUGET_RUNTIMES_WIN_DIR}/dxcompiler.dll
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                ${CMAKE_BINARY_DIR}/dxil.dll 
                ${NUGET_RUNTIMES_WIN_DIR}/dxil.dll
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                ${CMAKE_BINARY_DIR}/metalirconverter.dll 
                ${NUGET_RUNTIMES_WIN_DIR}/metalirconverter.dll
        
        # Copy targets file
        COMMAND ${CMAKE_COMMAND} -E copy_if_different 
                ${CURRENT_DIRECTORY}/NuGet/build/DenOfIzGraphics.targets
                ${NUGET_BUILD_DIR}/DenOfIzGraphics.targets
                
        # Print success message
        COMMAND ${CMAKE_COMMAND} -E echo "NuGet package files prepared successfully!"
)

# Add custom target for building the NuGet package
add_custom_target(DenOfIzNuget
        COMMAND ${CMAKE_COMMAND} -E echo "Building NuGet package..."
        COMMAND nuget pack ${NUGET_BASE_DIR}/DenOfIzGraphics.nuspec -Version 1.0.0 -OutputDirectory ${NUGET_BASE_DIR}
        DEPENDS DenOfIzGraphicsCSharp
        WORKING_DIRECTORY ${NUGET_BASE_DIR}
        COMMENT "Building NuGet package for DenOfIzGraphics"
)

# Add a message to inform the user that the package is ready
add_custom_command(TARGET DenOfIzNuget POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "NuGet package has been created successfully at ${NUGET_BASE_DIR}/DenOfIzGraphics.1.0.0.nupkg"
)