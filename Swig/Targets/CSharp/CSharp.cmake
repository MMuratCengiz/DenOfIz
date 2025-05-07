set(SWIG_CSHARP_DIR ${CMAKE_BINARY_DIR}/CSharp)
set(SWIG_CSHARP_CXX_DIR ${SWIG_CSHARP_DIR}/Swig)
set(SWIG_CSHARP_PROJECT_DIR ${SWIG_CSHARP_DIR}/Project)
set(SWIG_CSHARP_CODE_DIR ${SWIG_CSHARP_PROJECT_DIR}/Code)
set(SWIG_CSHARP_LIB_DIR ${SWIG_CSHARP_PROJECT_DIR}/Lib)

# Create CSharp directory structure before configuring any targets
file(MAKE_DIRECTORY ${SWIG_CSHARP_DIR})
file(MAKE_DIRECTORY ${SWIG_CSHARP_CXX_DIR})
file(MAKE_DIRECTORY ${SWIG_CSHARP_PROJECT_DIR})
file(MAKE_DIRECTORY ${SWIG_CSHARP_CODE_DIR})
file(MAKE_DIRECTORY ${SWIG_CSHARP_LIB_DIR})

# Create a custom target to ensure directories exist at build time too
add_custom_target(
    InitCSharpDirs ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SWIG_CSHARP_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SWIG_CSHARP_CXX_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SWIG_CSHARP_PROJECT_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SWIG_CSHARP_CODE_DIR}
    COMMAND ${CMAKE_COMMAND} -E make_directory ${SWIG_CSHARP_LIB_DIR}
    COMMENT "Ensuring SWIG CSharp directories exist"
)

# Create platform-specific output directory with proper naming that works on all platforms
if(WIN32)
    set(PLATFORM_NAME "win")
elseif(APPLE)
    set(PLATFORM_NAME "osx")
elseif(UNIX)
    set(PLATFORM_NAME "linux")
else()
    set(PLATFORM_NAME "unknown")
endif()

# Determine architecture
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(PLATFORM_ARCH "x64")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(PLATFORM_ARCH "x86")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    set(PLATFORM_ARCH "arm64")
else()
    set(PLATFORM_ARCH "unknown")
endif()

set(PLATFORM_OUTPUT_DIR "${SWIG_CSHARP_LIB_DIR}/${PLATFORM_NAME}-${PLATFORM_ARCH}")

# Create platform directory during cmake configuration
file(MAKE_DIRECTORY ${PLATFORM_OUTPUT_DIR})

# Also create it during build time
add_custom_target(
    InitPlatformDir ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${PLATFORM_OUTPUT_DIR}
    COMMENT "Creating platform-specific output directory: ${PLATFORM_OUTPUT_DIR}"
)

# Make sure our SWIG targets depend on the directory structure
add_dependencies(InitPlatformDir InitCSharpDirs)

swig_add_library(DenOfIzGraphicsCSharp
        TYPE SHARED
        LANGUAGE CSharp
        SOURCES DenOfIzGraphics.i
        OUTPUT_DIR ${SWIG_CSHARP_CODE_DIR}
        OUTFILE_DIR ${SWIG_CSHARP_CXX_DIR}
)
add_dependencies(DenOfIzGraphicsCSharp InitPlatformDir)

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

# Always copy the library files
add_custom_command(TARGET DenOfIzGraphicsCSharp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:DenOfIzGraphicsCSharp> ${SWIG_CSHARP_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DenOfIzGraphicsCSharp> ${SWIG_CSHARP_LIB_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DenOfIzGraphics> ${SWIG_CSHARP_LIB_DIR}
        COMMAND_EXPAND_LISTS
)

# Copy platform-specific libraries 
add_custom_command(TARGET DenOfIzGraphicsCSharp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DenOfIzGraphicsCSharp> ${PLATFORM_OUTPUT_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DenOfIzGraphics> ${PLATFORM_OUTPUT_DIR}
)

# Skip the C# project generation unless explicitly requested
option(BUILD_CSHARP_PROJECT "Build C# project" OFF)

if(BUILD_CSHARP_PROJECT)
    # Create a .csproj file for the C# wrapper that includes only NativeLibraryLoader.cs
    # All SWIG-generated C# files will be compiled by SWIG itself
    set(CSHARP_PROJECT_FILE "${SWIG_CSHARP_PROJECT_DIR}/DenOfIzGraphics.csproj")
    add_custom_command(TARGET DenOfIzGraphicsCSharp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "<Project Sdk=\"Microsoft.NET.Sdk\">" > ${CSHARP_PROJECT_FILE}
        COMMAND ${CMAKE_COMMAND} -E echo "  <PropertyGroup>" >> ${CSHARP_PROJECT_FILE}
        COMMAND ${CMAKE_COMMAND} -E echo "    <TargetFramework>netstandard2.0</TargetFramework>" >> ${CSHARP_PROJECT_FILE}
        COMMAND ${CMAKE_COMMAND} -E echo "    <AssemblyName>DenOfIzGraphics</AssemblyName>" >> ${CSHARP_PROJECT_FILE}
        COMMAND ${CMAKE_COMMAND} -E echo "    <RootNamespace>DenOfIzGraphics</RootNamespace>" >> ${CSHARP_PROJECT_FILE}
        COMMAND ${CMAKE_COMMAND} -E echo "    <AllowUnsafeBlocks>true</AllowUnsafeBlocks>" >> ${CSHARP_PROJECT_FILE}
        COMMAND ${CMAKE_COMMAND} -E echo "  </PropertyGroup>" >> ${CSHARP_PROJECT_FILE}
        COMMAND ${CMAKE_COMMAND} -E echo "  <ItemGroup>" >> ${CSHARP_PROJECT_FILE}
        COMMAND ${CMAKE_COMMAND} -E echo "    <!-- Custom C# files - exclude SWIG generated files -->" >> ${CSHARP_PROJECT_FILE}
        COMMAND ${CMAKE_COMMAND} -E echo "    <Compile Include=\"${SWIG_CSHARP_CODE_DIR}\\NativeLibraryLoader.cs\" />" >> ${CSHARP_PROJECT_FILE}
        COMMAND ${CMAKE_COMMAND} -E echo "    <!-- SWIG generated files will be compiled by the SWIG build process -->" >> ${CSHARP_PROJECT_FILE}
        COMMAND ${CMAKE_COMMAND} -E echo "  </ItemGroup>" >> ${CSHARP_PROJECT_FILE}
        COMMAND ${CMAKE_COMMAND} -E echo "</Project>" >> ${CSHARP_PROJECT_FILE}
    )

    # Find dotnet
    find_program(DOTNET_EXECUTABLE dotnet)
    if(DOTNET_EXECUTABLE)
        # Add custom command to build the C# project
        add_custom_command(TARGET DenOfIzGraphicsCSharp POST_BUILD
            COMMAND ${DOTNET_EXECUTABLE} build ${CSHARP_PROJECT_FILE} -c Release -o ${SWIG_CSHARP_PROJECT_DIR}/bin/Release
        )
    else()
        message(WARNING "dotnet not found. C# project will not be built.")
    endif()
endif()

# End of C# module generation

# Create a helper class for native library loading
add_custom_command(
    TARGET InitCSharpDirs
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${SWIG_CSHARP_CODE_DIR}"
    COMMAND ${CMAKE_COMMAND} -E echo "using System;" > "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "using System.IO;" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "using System.Runtime.InteropServices;" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "namespace DenOfIz" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "{" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "    internal static class NativeLibraryLoader" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "    {" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "        static NativeLibraryLoader()" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "        {" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            LoadNativeLibrary();" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "        }" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "        private static void LoadNativeLibrary()" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "        {" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            string libraryName = GetNativeLibraryName();" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            try" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            {" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                string runtimesPath = Path.Combine(" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                    AppDomain.CurrentDomain.BaseDirectory," >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                    \"runtimes\"," >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                    GetRuntimeIdentifier()," >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                    \"native\"," >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                    libraryName);" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                if (File.Exists(runtimesPath))" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                {" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                    // .NET Core 3.0+ can use NativeLibrary.Load" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                    #if NETCOREAPP3_0_OR_GREATER" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                    NativeLibrary.Load(runtimesPath);" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                    #endif" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                    return;" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                }" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            }" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            catch (Exception ex)" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            {" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                Console.WriteLine($\"Error loading library from runtimes folder: {ex.Message}\");" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            }" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            // On platform-specific assemblies, DllImport will load the library" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            Console.WriteLine($\"Falling back to OS library loading for {libraryName}\");" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "        }" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "        private static string GetNativeLibraryName()" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "        {" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            {" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                return \"DenOfIzGraphicsCSharp.dll\";" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            }" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            {" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                return \"libDenOfIzGraphicsCSharp.so\";" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            }" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            {" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                return \"libDenOfIzGraphicsCSharp.dylib\";" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            }" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            else" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            {" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                throw new PlatformNotSupportedException(\"Unsupported platform\");" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            }" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "        }" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "        private static string GetRuntimeIdentifier()" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "        {" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            string architecture;" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            switch (RuntimeInformation.ProcessArchitecture)" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            {" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                case Architecture.X64:" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                    architecture = \"x64\";" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                    break;" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                case Architecture.X86:" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                    architecture = \"x86\";" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                    break;" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                case Architecture.Arm64:" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                    architecture = \"arm64\";" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                    break;" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                default:" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                    throw new PlatformNotSupportedException(\"Unsupported architecture\");" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            }" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            {" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                return $\"win-{architecture}\";" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            }" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            else if (RuntimeInformation.IsOSPlatform(OSPlatform.Linux))" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            {" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                return $\"linux-{architecture}\";" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            }" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            else if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX))" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            {" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                return $\"osx-{architecture}\";" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            }" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            else" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            {" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "                throw new PlatformNotSupportedException(\"Unsupported platform\");" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "            }" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "        }" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "    }" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
    COMMAND ${CMAKE_COMMAND} -E echo "}" >> "${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs"
)