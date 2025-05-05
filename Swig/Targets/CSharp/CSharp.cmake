set(SWIG_CSHARP_DIR ${CMAKE_BINARY_DIR}/CSharp)
set(SWIG_CSHARP_CXX_DIR ${SWIG_CSHARP_DIR}/Swig)
set(SWIG_CSHARP_PROJECT_DIR ${SWIG_CSHARP_DIR}/Project)
set(SWIG_CSHARP_CODE_DIR ${SWIG_CSHARP_PROJECT_DIR}/Code)
set(SWIG_CSHARP_LIB_DIR ${SWIG_CSHARP_PROJECT_DIR}/Lib)

swig_add_library(DenOfIzGraphicsCSharp
        TYPE SHARED
        LANGUAGE CSharp
        SOURCES DenOfIzGraphics.i
        OUTPUT_DIR ${SWIG_CSHARP_CODE_DIR}
        OUTFILE_DIR ${SWIG_CSHARP_CXX_DIR}
)
set_target_properties(DenOfIzGraphicsCSharp PROPERTIES
        SWIG_FLAGS "-E"
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
)

add_custom_command(
        TARGET DenOfIzGraphicsCSharp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory ${SWIG_CSHARP_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:DenOfIzGraphicsCSharp> ${SWIG_CSHARP_LIB_DIR}
        COMMAND_EXPAND_LISTS
)

add_custom_command(TARGET DenOfIzGraphicsCSharp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DenOfIzGraphicsCSharp> ${SWIG_CSHARP_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DenOfIzGraphics> ${SWIG_CSHARP_LIB_DIR}
)

# Platform-specific output directory for NuGet packaging
set(PLATFORM_OUTPUT_DIR "${SWIG_CSHARP_LIB_DIR}/${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
file(MAKE_DIRECTORY ${PLATFORM_OUTPUT_DIR})

# Copy platform-specific libraries to their respective directories
add_custom_command(TARGET DenOfIzGraphicsCSharp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DenOfIzGraphicsCSharp> ${PLATFORM_OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DenOfIzGraphics> ${PLATFORM_OUTPUT_DIR}
)

# Create a .csproj file for the C# wrapper
set(CSHARP_PROJECT_FILE "${SWIG_CSHARP_PROJECT_DIR}/DenOfIzGraphics.csproj")
file(WRITE ${CSHARP_PROJECT_FILE} 
"<Project Sdk=\"Microsoft.NET.Sdk\">
  <PropertyGroup>
    <TargetFramework>netstandard2.0</TargetFramework>
    <AssemblyName>DenOfIzGraphics</AssemblyName>
    <RootNamespace>DenOfIzGraphics</RootNamespace>
    <AllowUnsafeBlocks>true</AllowUnsafeBlocks>
  </PropertyGroup>
  <ItemGroup>
    <Compile Include=\"${SWIG_CSHARP_CODE_DIR}\\*.cs\" />
  </ItemGroup>
</Project>")

# Add custom command to build the C# project
find_program(DOTNET_EXECUTABLE dotnet REQUIRED)
add_custom_command(TARGET DenOfIzGraphicsCSharp POST_BUILD
    COMMAND ${DOTNET_EXECUTABLE} build ${CSHARP_PROJECT_FILE} -c Release -o ${SWIG_CSHARP_PROJECT_DIR}/bin/Release
)

# Create a helper class for native library loading
file(WRITE "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs" "
using System;
using System.IO;
using System.Runtime.InteropServices;

namespace DenOfIz 
{
    internal static class NativeLibraryLoader 
    {
        static NativeLibraryLoader() 
        {
            LoadNativeLibrary();
        }

        private static void LoadNativeLibrary() 
        {
            string libraryName = GetNativeLibraryName();
            string runtimesPath = Path.Combine(
                AppDomain.CurrentDomain.BaseDirectory,
                \"runtimes\",
                GetRuntimeIdentifier(),
                \"native\",
                libraryName);

            if (!File.Exists(runtimesPath)) 
            {
                // Fall back to default loading mechanism
                runtimesPath = libraryName;
            }

            // TODO NativeLibrary.Load
        }

        private static string GetNativeLibraryName() 
        {
            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                return \"DenOfIzGraphicsCSharp.dll\";
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                return \"libDenOfIzGraphicsCSharp.so\";
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                return \"libDenOfIzGraphicsCSharp.dylib\";
            }
            else
            {
                throw new PlatformNotSupportedException(\"Unsupported platform\");
            }
        }

        private static string GetRuntimeIdentifier() 
        {
            string architecture = RuntimeInformation.ProcessArchitecture switch 
            {
                Architecture.X64 => \"x64\",
                Architecture.X86 => \"x86\",
                Architecture.Arm64 => \"arm64\",
                _ => throw new PlatformNotSupportedException(\"Unsupported architecture\")
            };

            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                return $\"win-{architecture}\";
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))
            {
                return $\"linux-{architecture}\";
            }
            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
            {
                return $\"osx-{architecture}\";
            }
            else
            {
                throw new PlatformNotSupportedException(\"Unsupported platform\");
            }
        }
    }
}")