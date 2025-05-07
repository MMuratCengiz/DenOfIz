cmake_policy(SET CMP0122 NEW)

set(DENOFIZ_VERSION "1.0.0")
string(TIMESTAMP CURRENT_YEAR "%Y")

if (SWIG_CSHARP OR SWIG_JAVA)
    find_package(SWIG REQUIRED)
    include(${SWIG_USE_FILE})
    set_source_files_properties(DenOfIzGraphics.i PROPERTIES CPLUSPLUS ON)
    set_source_files_properties(DenOfIzGraphics_Dxc.i PROPERTIES CPLUSPLUS ON)
endif ()

# Define platform names and architectures consistently across the build
if (WIN32)
    set(NATIVE_LIB_PREFIX "")
    set(NATIVE_LIB_SUFFIX ".dll")
    set(PLATFORM_RID "win")
elseif (APPLE)
    set(NATIVE_LIB_PREFIX "lib")
    set(NATIVE_LIB_SUFFIX ".dylib")
    set(PLATFORM_RID "osx")
elseif (UNIX)
    set(NATIVE_LIB_PREFIX "lib")
    set(NATIVE_LIB_SUFFIX ".so")
    set(PLATFORM_RID "linux")
else ()
    set(NATIVE_LIB_PREFIX "lib")
    set(NATIVE_LIB_SUFFIX ".so")
    set(PLATFORM_RID "unknown")
endif ()

# Detect architecture
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH "x64")
elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
    set(ARCH "x86")
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "arm")
    set(ARCH "arm64")
else ()
    set(ARCH "unknown")
endif ()

if (SWIG_CSHARP)
    include(Targets/CSharp/CSharp.cmake)
endif ()

if (SWIG_JAVA)
    include(Targets/Java/Java.cmake)
endif ()

if (SWIG_CSHARP)
    set(NUGET_OUTPUT_DIR ${CMAKE_BINARY_DIR}/nuget)
    set(NUGET_SPEC_FILE ${NUGET_OUTPUT_DIR}/DenOfIzGraphics.nuspec)
    set(NUGET_TARGETS_FILE ${NUGET_OUTPUT_DIR}/DenOfIzGraphics.targets)

    file(MAKE_DIRECTORY ${NUGET_OUTPUT_DIR})
    file(MAKE_DIRECTORY ${NUGET_OUTPUT_DIR}/build)
    file(MAKE_DIRECTORY ${NUGET_OUTPUT_DIR}/lib/netstandard2.0)
    file(MAKE_DIRECTORY ${NUGET_OUTPUT_DIR}/runtimes/${PLATFORM_RID}-${ARCH}/native)

    file(WRITE ${NUGET_SPEC_FILE} "<?xml version=\"1.0\" encoding=\"utf-8\"?>
<package xmlns=\"http://schemas.microsoft.com/packaging/2013/05/nuspec.xsd\">
  <metadata>
    <id>DenOfIzGraphics</id>
    <version>${DENOFIZ_VERSION}</version>
    <authors>DenOfIz Team</authors>
    <owners>DenOfIz Team</owners>
    <requireLicenseAcceptance>false</requireLicenseAcceptance>
    <license type=\"expression\">GPL-3.0-or-later</license>
    <projectUrl>https://github.com/yourusername/DenOfIz</projectUrl>
    <description>Cross-platform graphics library for Windows, macOS, and Linux</description>
    <copyright>Copyright ${CURRENT_YEAR}</copyright>
    <tags>graphics directx vulkan metal</tags>
    <dependencies>
      <group targetFramework=\"netstandard2.0\" />
    </dependencies>
  </metadata>
  <files>
    <!-- Managed assembly -->
    <file src=\"lib/netstandard2.0/*.*\" target=\"lib/netstandard2.0\" />
    
    <!-- Platform-specific native libraries -->
    <file src=\"runtimes/${PLATFORM_RID}-${ARCH}/native/*.*\" target=\"runtimes/${PLATFORM_RID}-${ARCH}/native\" />
    
    <!-- Build targets -->
    <file src=\"build/DenOfIzGraphics.targets\" target=\"build/DenOfIzGraphics.targets\" />
  </files>
</package>")

    file(WRITE ${NUGET_TARGETS_FILE} "<?xml version=\"1.0\" encoding=\"utf-8\"?>
<Project ToolsVersion=\"4.0\" xmlns=\"http://schemas.microsoft.com/developer/msbuild/2003\">
  <ItemGroup>
    <NativeLibs Include=\"$(MSBuildThisFileDirectory)../runtimes/**/*.dll;$(MSBuildThisFileDirectory)../runtimes/**/*.so;$(MSBuildThisFileDirectory)../runtimes/**/*.dylib\" />
    <None Include=\"@(NativeLibs)\">
      <Link>%(RecursiveDir)%(FileName)%(Extension)</Link>
      <CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory>
    </None>
  </ItemGroup>
</Project>")

    add_custom_target(nuget_package
            COMMAND ${CMAKE_COMMAND} -E copy ${NUGET_TARGETS_FILE} ${NUGET_OUTPUT_DIR}/build/DenOfIzGraphics.targets
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DenOfIzGraphics> ${NUGET_OUTPUT_DIR}/runtimes/${PLATFORM_RID}-${ARCH}/native/
            COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:DenOfIzGraphicsCSharp> ${NUGET_OUTPUT_DIR}/runtimes/${PLATFORM_RID}-${ARCH}/native/
            COMMAND nuget pack ${NUGET_SPEC_FILE} -OutputDirectory ${NUGET_OUTPUT_DIR}
            DEPENDS DenOfIzGraphicsCSharp DenOfIzGraphics
            COMMENT "Building NuGet package"
    )

    # Make sure NuGet output directories exist
    file(MAKE_DIRECTORY ${NUGET_OUTPUT_DIR}/lib/netstandard2.0)
    file(MAKE_DIRECTORY ${NUGET_OUTPUT_DIR}/runtimes/${PLATFORM_RID}-${ARCH}/native)

    add_custom_command(
            TARGET InitCSharpDirs POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_OUTPUT_DIR}/lib/netstandard2.0
            COMMAND ${CMAKE_COMMAND} -E make_directory ${NUGET_OUTPUT_DIR}/runtimes/${PLATFORM_RID}-${ARCH}/native
    )

    # Copy managed code to NuGet package
    # Only copy NativeLibraryLoader.cs, let SWIG manage its generated files
    add_custom_command(
            TARGET DenOfIzGraphicsCSharp POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy
            ${SWIG_CSHARP_CODE_DIR}/NativeLibraryLoader.cs
            ${NUGET_OUTPUT_DIR}/lib/netstandard2.0/
    )
endif ()