#!/bin/bash
set -e

# This script builds a NuGet package from DenOfIzGraphics CSharp bindings
echo "DenOfIzGraphics NuGet Package Generator"
echo "======================================"

# Config variables
VERSION="1.0.0"
PACKAGE_ID="DenOfIzGraphics"
AUTHORS="DenOfIz Team"
DESCRIPTION="Cross-platform graphics library with DirectX, Vulkan, and Metal backends"
COPYRIGHT="Copyright © $(date +%Y) DenOfIz Team"
TAGS="graphics directx vulkan metal"
REPOSITORY_URL="https://github.com/YourOrganization/DenOfIz"
LICENSE_URL="https://github.com/YourOrganization/DenOfIz/blob/master/LICENSE"
PROJECT_URL="https://github.com/YourOrganization/DenOfIz"

# Paths
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" &> /dev/null && pwd)"
OUTPUT_DIR="${SCRIPT_DIR}/NuGet"
TEMP_DIR="${OUTPUT_DIR}/temp"
NUPKG_DIR="${OUTPUT_DIR}/nupkg"

# Check if dotnet is installed
if ! command -v dotnet &> /dev/null; then
    echo "Error: dotnet command not found. Please install .NET SDK."
    exit 1
fi

# Build configuration
BUILD_CONFIG=${1:-Debug_MSVC}
echo "Using build configuration: ${BUILD_CONFIG}"

# Check if build directory exists
BUILD_DIR="${SCRIPT_DIR}/build/DenOfIz/${BUILD_CONFIG}"
if [ ! -d "${BUILD_DIR}" ]; then
    echo "Error: Build directory not found: ${BUILD_DIR}"
    echo "Please build the project first with: cmake --build build --config ${BUILD_CONFIG}"
    exit 1
fi

# Create directories
mkdir -p "${OUTPUT_DIR}"
mkdir -p "${TEMP_DIR}"
mkdir -p "${NUPKG_DIR}"
mkdir -p "${TEMP_DIR}/lib/netstandard2.0"
mkdir -p "${TEMP_DIR}/runtimes/win-x64/native"
mkdir -p "${TEMP_DIR}/runtimes/linux-x64/native"
mkdir -p "${TEMP_DIR}/runtimes/osx-x64/native"
mkdir -p "${TEMP_DIR}/runtimes/osx-arm64/native"
mkdir -p "${TEMP_DIR}/build"
mkdir -p "${TEMP_DIR}/contentFiles/any/any"

# Source directories
CSHARP_DIR="${BUILD_DIR}/CSharp"
LIB_DIR="${CSHARP_DIR}/Project/Lib"
CODE_DIR="${CSHARP_DIR}/Project/Code"

# Check if required directories exist
if [ ! -d "${LIB_DIR}" ]; then
    echo "Error: Library directory not found: ${LIB_DIR}"
    exit 1
fi

if [ ! -d "${CODE_DIR}" ]; then
    echo "Error: Code directory not found: ${CODE_DIR}"
    exit 1
fi

# Copy C# source files to temp directory
echo "Copying C# source files..."
cp "${CODE_DIR}"/*.cs "${TEMP_DIR}/lib/netstandard2.0/"

# Copy NativeLibrary.cs from the Src directory
echo "Copying NativeLibrary.cs..."
SRC_DIR="${SCRIPT_DIR}/Swig/Targets/CSharp/Project/Src"
if [ -f "${SRC_DIR}/NativeLibrary.cs" ]; then
    cp "${SRC_DIR}/NativeLibrary.cs" "${TEMP_DIR}/lib/netstandard2.0/"
fi

# Use our fixed PlatformHelper.cs without DenOfIz namespace requirement
if [ -f "${SCRIPT_DIR}/NuGet/temp/FixedPlatformHelper.cs" ]; then
    cp "${SCRIPT_DIR}/NuGet/temp/FixedPlatformHelper.cs" "${TEMP_DIR}/lib/netstandard2.0/PlatformHelper.cs"
else
    # Fix the namespace issue
    if [ -f "${SRC_DIR}/PlatformHelper.cs" ]; then
        # Create a fixed version of PlatformHelper.cs without the dependency on DenOfIz namespace
        sed 's/DenOfIz\.//g' "${SRC_DIR}/PlatformHelper.cs" > "${TEMP_DIR}/lib/netstandard2.0/PlatformHelper.cs"
    fi
fi

# Remove the DenOfIzGraphics.cs file that conflicts with the namespace
if [ -f "${TEMP_DIR}/lib/netstandard2.0/DenOfIzGraphics.cs" ]; then
    rm "${TEMP_DIR}/lib/netstandard2.0/DenOfIzGraphics.cs"
fi

# Remove the problematic NativeLibraryLoader.cs if it exists (we don't need it anymore)
if [ -f "${TEMP_DIR}/lib/netstandard2.0/NativeLibraryLoader.cs" ]; then
    rm "${TEMP_DIR}/lib/netstandard2.0/NativeLibraryLoader.cs"
fi

# No need for NativeLibraryLoader.cs since we use NativeLibrary.cs instead

# Check platform-specific directories
PLATFORM_LIBS_WIN="${LIB_DIR}/win-x64"
PLATFORM_LIBS_LINUX="${LIB_DIR}/linux-x64"
PLATFORM_LIBS_OSX_X64="${LIB_DIR}/osx-x64"
PLATFORM_LIBS_OSX_ARM64="${LIB_DIR}/osx-arm64"

# Copy Windows native libraries
echo "Copying Windows native libraries..."
if [ -d "${PLATFORM_LIBS_WIN}" ] && [ -n "$(ls -A "${PLATFORM_LIBS_WIN}" 2>/dev/null)" ]; then
    # Copy from platform-specific directory if it exists and has files
    cp "${PLATFORM_LIBS_WIN}"/* "${TEMP_DIR}/runtimes/win-x64/native/" 2>/dev/null
    echo "Added Windows x64 runtime from platform directory"
elif [ -f "${LIB_DIR}/DenOfIzGraphicsCSharp.dll" ]; then
    # Fallback to root lib directory
    cp "${LIB_DIR}/DenOfIzGraphicsCSharp.dll" "${TEMP_DIR}/runtimes/win-x64/native/"
    cp "${LIB_DIR}/DenOfIzGraphics.dll" "${TEMP_DIR}/runtimes/win-x64/native/" 2>/dev/null || :
    
    # Copy dependencies if they exist
    for dep in D3D12Core.dll d3d12SDKLayers.dll dxil.dll dxcompiler.dll; do
        if [ -f "${LIB_DIR}/${dep}" ]; then
            cp "${LIB_DIR}/${dep}" "${TEMP_DIR}/runtimes/win-x64/native/"
        fi
    done
    
    echo "Added Windows x64 runtime from root directory"
else
    echo "Warning: Windows binaries not found in ${LIB_DIR}"
fi

# Copy Linux native libraries if they exist
echo "Checking for Linux native libraries..."
if [ -d "${PLATFORM_LIBS_LINUX}" ] && [ -n "$(ls -A "${PLATFORM_LIBS_LINUX}" 2>/dev/null)" ]; then
    # Copy from platform-specific directory if it exists and has files
    cp "${PLATFORM_LIBS_LINUX}"/* "${TEMP_DIR}/runtimes/linux-x64/native/" 2>/dev/null
    echo "Added Linux x64 runtime from platform directory"
elif [ -f "${LIB_DIR}/libDenOfIzGraphicsCSharp.so" ]; then
    # Fallback to root lib directory
    cp "${LIB_DIR}/libDenOfIzGraphicsCSharp.so" "${TEMP_DIR}/runtimes/linux-x64/native/"
    cp "${LIB_DIR}/libDenOfIzGraphics.so" "${TEMP_DIR}/runtimes/linux-x64/native/" 2>/dev/null || :
    echo "Added Linux x64 runtime from root directory"
else
    echo "Warning: Linux binaries not found (this is normal when building on Windows)"
fi

# Copy macOS native libraries if they exist
echo "Checking for macOS native libraries..."

# Check for x64 macOS binaries
if [ -d "${PLATFORM_LIBS_OSX_X64}" ] && [ -n "$(ls -A "${PLATFORM_LIBS_OSX_X64}" 2>/dev/null)" ]; then
    # Copy from platform-specific directory if it exists and has files
    cp "${PLATFORM_LIBS_OSX_X64}"/* "${TEMP_DIR}/runtimes/osx-x64/native/" 2>/dev/null
    echo "Added macOS x64 runtime from platform directory"
fi

# Check for arm64 macOS binaries
if [ -d "${PLATFORM_LIBS_OSX_ARM64}" ] && [ -n "$(ls -A "${PLATFORM_LIBS_OSX_ARM64}" 2>/dev/null)" ]; then
    # Copy from platform-specific directory if it exists and has files
    cp "${PLATFORM_LIBS_OSX_ARM64}"/* "${TEMP_DIR}/runtimes/osx-arm64/native/" 2>/dev/null
    echo "Added macOS arm64 runtime from platform directory"
fi

# Fallback to root lib directory for macOS
if [ -f "${LIB_DIR}/libDenOfIzGraphicsCSharp.dylib" ]; then
    # Try to detect if it's x64 or arm64
    if file "${LIB_DIR}/libDenOfIzGraphicsCSharp.dylib" 2>/dev/null | grep -q "x86_64"; then
        cp "${LIB_DIR}/libDenOfIzGraphicsCSharp.dylib" "${TEMP_DIR}/runtimes/osx-x64/native/"
        cp "${LIB_DIR}/libDenOfIzGraphics.dylib" "${TEMP_DIR}/runtimes/osx-x64/native/" 2>/dev/null || :
        echo "Added macOS x64 runtime from root directory"
    elif file "${LIB_DIR}/libDenOfIzGraphicsCSharp.dylib" 2>/dev/null | grep -q "arm64"; then
        cp "${LIB_DIR}/libDenOfIzGraphicsCSharp.dylib" "${TEMP_DIR}/runtimes/osx-arm64/native/"
        cp "${LIB_DIR}/libDenOfIzGraphics.dylib" "${TEMP_DIR}/runtimes/osx-arm64/native/" 2>/dev/null || :
        echo "Added macOS arm64 runtime from root directory"
    else
        # If we can't determine the architecture, copy to both directories
        cp "${LIB_DIR}/libDenOfIzGraphicsCSharp.dylib" "${TEMP_DIR}/runtimes/osx-x64/native/"
        cp "${LIB_DIR}/libDenOfIzGraphics.dylib" "${TEMP_DIR}/runtimes/osx-x64/native/" 2>/dev/null || :
        cp "${LIB_DIR}/libDenOfIzGraphicsCSharp.dylib" "${TEMP_DIR}/runtimes/osx-arm64/native/"
        cp "${LIB_DIR}/libDenOfIzGraphics.dylib" "${TEMP_DIR}/runtimes/osx-arm64/native/" 2>/dev/null || :
        echo "Added macOS runtime to both x64 and arm64 (architecture not determined)"
    fi
else
    echo "Warning: macOS binaries not found (this is normal when building on Windows)"
fi

# Create .csproj file
echo "Creating project file..."
cat > "${TEMP_DIR}/${PACKAGE_ID}.csproj" << EOF
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFramework>netstandard2.0</TargetFramework>
    <LangVersion>8.0</LangVersion>
    <AllowUnsafeBlocks>true</AllowUnsafeBlocks>
    <GenerateDocumentationFile>true</GenerateDocumentationFile>
    <Nullable>enable</Nullable>

    <!-- NuGet package properties -->
    <PackageId>${PACKAGE_ID}</PackageId>
    <Version>${VERSION}</Version>
    <Authors>${AUTHORS}</Authors>
    <Company>DenOfIz</Company>
    <Description>${DESCRIPTION}</Description>
    <Copyright>${COPYRIGHT}</Copyright>
    <PackageTags>${TAGS}</PackageTags>
    <PackageProjectUrl>${PROJECT_URL}</PackageProjectUrl>
    <RepositoryUrl>${REPOSITORY_URL}</RepositoryUrl>
    <RepositoryType>git</RepositoryType>
    <PackageLicenseUrl>${LICENSE_URL}</PackageLicenseUrl>
    
    <!-- Don't pack content to lib folder, we handle it manually -->
    <IncludeBuildOutput>false</IncludeBuildOutput>
  </PropertyGroup>

  <!-- Disable default compile items to avoid duplicates -->
  <PropertyGroup>
    <EnableDefaultCompileItems>false</EnableDefaultCompileItems>
  </PropertyGroup>
  
  <!-- Include source files -->
  <ItemGroup>
    <Compile Include="lib/netstandard2.0/*.cs" />
  </ItemGroup>

  <!-- Platform specific native libraries -->
  <ItemGroup>
    <!-- Windows x64 -->
    <Content Include="runtimes/win-x64/native/*.dll">
      <PackagePath>runtimes/win-x64/native</PackagePath>
      <Pack>true</Pack>
    </Content>
    
    <!-- Linux x64 -->
    <Content Include="runtimes/linux-x64/native/*.so">
      <PackagePath>runtimes/linux-x64/native</PackagePath>
      <Pack>true</Pack>
    </Content>
    
    <!-- macOS x64 -->
    <Content Include="runtimes/osx-x64/native/*.dylib">
      <PackagePath>runtimes/osx-x64/native</PackagePath>
      <Pack>true</Pack>
    </Content>
    
    <!-- macOS arm64 -->
    <Content Include="runtimes/osx-arm64/native/*.dylib">
      <PackagePath>runtimes/osx-arm64/native</PackagePath>
      <Pack>true</Pack>
    </Content>
    
    <!-- Pack managed assembly -->
    <Content Include="lib/netstandard2.0/*.cs">
      <PackagePath>lib/netstandard2.0</PackagePath>
      <Pack>true</Pack>
    </Content>
  </ItemGroup>
  
  <!-- Create props file for native references -->
  <ItemGroup>
    <Content Include="build/*.props;build/*.targets">
      <PackagePath>build</PackagePath>
      <Pack>true</Pack>
    </Content>
  </ItemGroup>
</Project>
EOF

# Create build props file
echo "Creating build props file..."
cat > "${TEMP_DIR}/build/${PACKAGE_ID}.props" << EOF
<?xml version="1.0" encoding="utf-8"?>
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup>
    <DenOfIzGraphicsNativeLibsPath>$(MSBuildThisFileDirectory)/../runtimes</DenOfIzGraphicsNativeLibsPath>
  </PropertyGroup>
</Project>
EOF

# Create build targets file
echo "Creating build targets file..."
cat > "${TEMP_DIR}/build/${PACKAGE_ID}.targets" << EOF
<?xml version="1.0" encoding="utf-8"?>
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <!-- Windows-specific targets -->
  <ItemGroup Condition="'$(OS)' == 'Windows_NT'">
    <None Include="$(DenOfIzGraphicsNativeLibsPath)/win-x64/native/*.dll">
      <Link>%(Filename)%(Extension)</Link>
      <CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory>
    </None>
  </ItemGroup>
  
  <!-- Linux-specific targets -->
  <ItemGroup Condition="'$(OS)' != 'Windows_NT' AND $([MSBuild]::IsOSPlatform('Linux'))">
    <None Include="$(DenOfIzGraphicsNativeLibsPath)/linux-x64/native/*.so">
      <Link>%(Filename)%(Extension)</Link>
      <CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory>
    </None>
  </ItemGroup>
  
  <!-- macOS-specific targets -->
  <ItemGroup Condition="'$(OS)' != 'Windows_NT' AND $([MSBuild]::IsOSPlatform('OSX'))">
    <!-- Check for ARM64 macOS -->
    <None Include="$(DenOfIzGraphicsNativeLibsPath)/osx-arm64/native/*.dylib"
          Condition="$([System.Runtime.InteropServices.RuntimeInformation]::ProcessArchitecture) == 'Arm64'">
      <Link>%(Filename)%(Extension)</Link>
      <CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory>
    </None>
    
    <!-- Check for x64 macOS -->
    <None Include="$(DenOfIzGraphicsNativeLibsPath)/osx-x64/native/*.dylib"
          Condition="$([System.Runtime.InteropServices.RuntimeInformation]::ProcessArchitecture) == 'X64'">
      <Link>%(Filename)%(Extension)</Link>
      <CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory>
    </None>
  </ItemGroup>
</Project>
EOF

# Create README
echo "Creating package README..."
cat > "${TEMP_DIR}/README.md" << EOF
# DenOfIzGraphics

Cross-platform graphics library with DirectX, Vulkan, and Metal backends.

## Getting Started

This package provides .NET bindings for the DenOfIzGraphics native library, a cross-platform graphics engine that supports:

- DirectX 12 on Windows
- Vulkan on Windows and Linux
- Metal on macOS

## Usage

```csharp
using DenOfIzGraphics;

// Create graphics API instance with automatic API selection
var api = PlatformHelper.CreateOptimalGraphicsApi();

// Initialize libraries
DenOfIz.NativeLibrary.LoadLibraries();

// Create a window
var windowProps = new WindowProperties() 
{
    Title = "DenOfIz Example",
    Width = 1280,
    Height = 720
};
var window = new Window(windowProps);

// Your rendering code here...
```

## Platform Support

- Windows x64
- Linux x64 
- macOS (x64 and ARM64)

## Requirements

- .NET Standard 2.0 compatible platform
- Windows 10+ for DirectX 12 support
- Linux with Vulkan support
- macOS 10.13+ for Metal support

## License

See the LICENSE file for details.
EOF

# Create the package
echo "Building NuGet package..."
pushd "${TEMP_DIR}" > /dev/null
dotnet pack -c Release -o "${NUPKG_DIR}"
popd > /dev/null

# Check if package was created
NUPKG_COUNT=$(ls -1 "${NUPKG_DIR}"/*.nupkg 2>/dev/null | wc -l)
if [ "$NUPKG_COUNT" -gt 0 ]; then
    echo "NuGet package created successfully:"
    ls -la "${NUPKG_DIR}"/*.nupkg
    echo ""
    echo "You can install the package locally with:"
    echo "dotnet add package ${PACKAGE_ID} --source ${NUPKG_DIR}"
else
    echo "Error: NuGet package creation failed"
    exit 1
fi

echo "Done!"