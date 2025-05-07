# DenOfIzGraphics NuGet Package Generator (PowerShell Version)
Write-Host "DenOfIzGraphics NuGet Package Generator" -ForegroundColor Cyan
Write-Host "======================================" -ForegroundColor Cyan

# Config variables
$VERSION = "1.0.0"
$PACKAGE_ID = "DenOfIzGraphics"
$AUTHORS = "DenOfIz Team"
$DESCRIPTION = "Cross-platform graphics library with DirectX, Vulkan, and Metal backends"
$COPYRIGHT = "Copyright © $(Get-Date -Format yyyy) DenOfIz Team"
$TAGS = "graphics directx vulkan metal"
$REPOSITORY_URL = "https://github.com/YourOrganization/DenOfIz"
$LICENSE_URL = "https://github.com/YourOrganization/DenOfIz/blob/master/LICENSE"
$PROJECT_URL = "https://github.com/YourOrganization/DenOfIz"

# Paths
$SCRIPT_DIR = Split-Path -Parent $MyInvocation.MyCommand.Path
$OUTPUT_DIR = Join-Path $SCRIPT_DIR "NuGet"
$TEMP_DIR = Join-Path $OUTPUT_DIR "temp"
$NUPKG_DIR = Join-Path $OUTPUT_DIR "nupkg"

# Check if dotnet is installed
try {
    $dotnetVersion = dotnet --version
    Write-Host "Using .NET SDK version: $dotnetVersion" -ForegroundColor Green
}
catch {
    Write-Host "Error: dotnet command not found. Please install .NET SDK." -ForegroundColor Red
    exit 1
}

# Build configuration
$BUILD_CONFIG = if ($args[0]) { $args[0] } else { "Debug_MSVC" }
Write-Host "Using build configuration: $BUILD_CONFIG" -ForegroundColor Yellow

# Check if build directory exists
$BUILD_DIR = Join-Path $SCRIPT_DIR "build\DenOfIz\$BUILD_CONFIG"
if (-not (Test-Path $BUILD_DIR)) {
    Write-Host "Error: Build directory not found: $BUILD_DIR" -ForegroundColor Red
    Write-Host "Please build the project first with: cmake --build build --config $BUILD_CONFIG" -ForegroundColor Yellow
    exit 1
}

# Create directories
if (-not (Test-Path $OUTPUT_DIR)) { New-Item -ItemType Directory -Path $OUTPUT_DIR | Out-Null }
if (Test-Path $TEMP_DIR) { Remove-Item -Recurse -Force $TEMP_DIR }
New-Item -ItemType Directory -Path $TEMP_DIR | Out-Null
if (-not (Test-Path $NUPKG_DIR)) { New-Item -ItemType Directory -Path $NUPKG_DIR | Out-Null }

# Create necessary directories
$dirs = @(
    "lib\netstandard2.0",
    "runtimes\win-x64\native",
    "runtimes\linux-x64\native",
    "runtimes\osx-x64\native",
    "runtimes\osx-arm64\native",
    "build",
    "contentFiles\any\any"
)

foreach ($dir in $dirs) {
    New-Item -ItemType Directory -Path (Join-Path $TEMP_DIR $dir) -Force | Out-Null
}

# Source directories
$CSHARP_DIR = Join-Path $BUILD_DIR "CSharp"
$LIB_DIR = Join-Path $CSHARP_DIR "Project\Lib"
$CODE_DIR = Join-Path $CSHARP_DIR "Project\Code"

# Check if required directories exist
if (-not (Test-Path $LIB_DIR)) {
    Write-Host "Error: Library directory not found: $LIB_DIR" -ForegroundColor Red
    exit 1
}

if (-not (Test-Path $CODE_DIR)) {
    Write-Host "Error: Code directory not found: $CODE_DIR" -ForegroundColor Red
    exit 1
}

# Copy C# source files to temp directory
Write-Host "Copying C# source files..." -ForegroundColor Green
Copy-Item -Path "$CODE_DIR\*.cs" -Destination (Join-Path $TEMP_DIR "lib\netstandard2.0") -Force

# Copy NativeLibrary.cs from the Src directory
Write-Host "Copying NativeLibrary.cs and PlatformHelper.cs..." -ForegroundColor Green
$SRC_DIR = Join-Path $SCRIPT_DIR "Swig\Targets\CSharp\Project\Src"
$nativeLibraryPath = Join-Path $SRC_DIR "NativeLibrary.cs"
$tempFixedPlatformHelperPath = Join-Path $SCRIPT_DIR "NuGet\temp\FixedPlatformHelper.cs"

# Copy NativeLibrary.cs
if (Test-Path $nativeLibraryPath) {
    Copy-Item -Path $nativeLibraryPath -Destination (Join-Path $TEMP_DIR "lib\netstandard2.0") -Force
}

# Use the fixed PlatformHelper.cs if it exists
if (Test-Path $tempFixedPlatformHelperPath) {
    Copy-Item -Path $tempFixedPlatformHelperPath -Destination (Join-Path $TEMP_DIR "lib\netstandard2.0\PlatformHelper.cs") -Force
    Write-Host "Using fixed PlatformHelper.cs" -ForegroundColor Green
}
else {
    # Create a fixed version by removing DenOfIz namespace references
    $platformHelperPath = Join-Path $SRC_DIR "PlatformHelper.cs"
    if (Test-Path $platformHelperPath) {
        $content = Get-Content -Path $platformHelperPath -Raw
        $fixedContent = $content -replace "DenOfIz\.", ""
        $fixedContent | Out-File -FilePath (Join-Path $TEMP_DIR "lib\netstandard2.0\PlatformHelper.cs") -Encoding utf8
        Write-Host "Created fixed PlatformHelper.cs" -ForegroundColor Green
    }
}

# Remove the DenOfIzGraphics.cs file that conflicts with the namespace
$conflictingGraphicsPath = Join-Path $TEMP_DIR "lib\netstandard2.0\DenOfIzGraphics.cs"
if (Test-Path $conflictingGraphicsPath) {
    Remove-Item -Path $conflictingGraphicsPath -Force
    Write-Host "Removed conflicting DenOfIzGraphics.cs file" -ForegroundColor Green
}

# Remove the problematic NativeLibraryLoader.cs if it exists (we don't need it anymore)
$nativeLoaderPath = Join-Path $TEMP_DIR "lib\netstandard2.0\NativeLibraryLoader.cs"
if (Test-Path $nativeLoaderPath) {
    Remove-Item -Path $nativeLoaderPath -Force
}

# No need for NativeLibraryLoader.cs since we use NativeLibrary.cs instead

# Check platform-specific directories
$PLATFORM_LIBS_WIN = Join-Path $LIB_DIR "win-x64"
$PLATFORM_LIBS_LINUX = Join-Path $LIB_DIR "linux-x64"
$PLATFORM_LIBS_OSX_X64 = Join-Path $LIB_DIR "osx-x64"
$PLATFORM_LIBS_OSX_ARM64 = Join-Path $LIB_DIR "osx-arm64"

# Copy Windows native libraries
Write-Host "Copying Windows native libraries..." -ForegroundColor Green
$winTargetDir = Join-Path $TEMP_DIR "runtimes\win-x64\native"

if (Test-Path $PLATFORM_LIBS_WIN) {
    # Copy from platform-specific directory if it exists
    $winFiles = Get-ChildItem -Path $PLATFORM_LIBS_WIN -File -ErrorAction SilentlyContinue
    if ($winFiles) {
        Copy-Item -Path "$PLATFORM_LIBS_WIN\*" -Destination $winTargetDir -Force
        Write-Host "Added Windows x64 runtime from platform directory" -ForegroundColor Green
    }
}
elseif (Test-Path (Join-Path $LIB_DIR "DenOfIzGraphicsCSharp.dll")) {
    # Fallback to root lib directory
    Copy-Item -Path (Join-Path $LIB_DIR "DenOfIzGraphicsCSharp.dll") -Destination $winTargetDir -Force
    
    # Copy main library if it exists
    $mainLib = Join-Path $LIB_DIR "DenOfIzGraphics.dll"
    if (Test-Path $mainLib) {
        Copy-Item -Path $mainLib -Destination $winTargetDir -Force
    }
    
    # Copy dependencies if they exist
    $dependencies = @(
        "D3D12Core.dll",
        "d3d12SDKLayers.dll",
        "dxil.dll",
        "dxcompiler.dll"
    )
    
    foreach ($dep in $dependencies) {
        $depPath = Join-Path $LIB_DIR $dep
        if (Test-Path $depPath) {
            Copy-Item -Path $depPath -Destination $winTargetDir -Force
        }
    }
    
    Write-Host "Added Windows x64 runtime from root directory" -ForegroundColor Green
}
else {
    Write-Host "Warning: Windows binaries not found in $LIB_DIR" -ForegroundColor Yellow
}

# Check for Linux native libraries
Write-Host "Checking for Linux native libraries..." -ForegroundColor Green
$linuxTargetDir = Join-Path $TEMP_DIR "runtimes\linux-x64\native"

if (Test-Path $PLATFORM_LIBS_LINUX) {
    # Copy from platform-specific directory if it exists
    $linuxFiles = Get-ChildItem -Path $PLATFORM_LIBS_LINUX -File -ErrorAction SilentlyContinue
    if ($linuxFiles) {
        Copy-Item -Path "$PLATFORM_LIBS_LINUX\*" -Destination $linuxTargetDir -Force
        Write-Host "Added Linux x64 runtime from platform directory" -ForegroundColor Green
    }
}
elseif (Test-Path (Join-Path $LIB_DIR "libDenOfIzGraphicsCSharp.so")) {
    # Fallback to root lib directory
    Copy-Item -Path (Join-Path $LIB_DIR "libDenOfIzGraphicsCSharp.so") -Destination $linuxTargetDir -Force
    
    $mainLib = Join-Path $LIB_DIR "libDenOfIzGraphics.so"
    if (Test-Path $mainLib) {
        Copy-Item -Path $mainLib -Destination $linuxTargetDir -Force
    }
    
    Write-Host "Added Linux x64 runtime from root directory" -ForegroundColor Green
}
else {
    Write-Host "Warning: Linux binaries not found (this is normal when building on Windows)" -ForegroundColor Yellow
}

# Check for macOS native libraries
Write-Host "Checking for macOS native libraries..." -ForegroundColor Green
$macX64TargetDir = Join-Path $TEMP_DIR "runtimes\osx-x64\native"
$macArm64TargetDir = Join-Path $TEMP_DIR "runtimes\osx-arm64\native"

# Check for x64 macOS binaries
if (Test-Path $PLATFORM_LIBS_OSX_X64) {
    $osxX64Files = Get-ChildItem -Path $PLATFORM_LIBS_OSX_X64 -File -ErrorAction SilentlyContinue
    if ($osxX64Files) {
        Copy-Item -Path "$PLATFORM_LIBS_OSX_X64\*" -Destination $macX64TargetDir -Force
        Write-Host "Added macOS x64 runtime from platform directory" -ForegroundColor Green
    }
}

# Check for arm64 macOS binaries
if (Test-Path $PLATFORM_LIBS_OSX_ARM64) {
    $osxArm64Files = Get-ChildItem -Path $PLATFORM_LIBS_OSX_ARM64 -File -ErrorAction SilentlyContinue
    if ($osxArm64Files) {
        Copy-Item -Path "$PLATFORM_LIBS_OSX_ARM64\*" -Destination $macArm64TargetDir -Force
        Write-Host "Added macOS arm64 runtime from platform directory" -ForegroundColor Green
    }
}

# Fallback to root lib directory for macOS
$macLib = Join-Path $LIB_DIR "libDenOfIzGraphicsCSharp.dylib"
if (Test-Path $macLib) {
    # For simplicity in PowerShell, just copy to both architectures if found in root
    Copy-Item -Path $macLib -Destination $macX64TargetDir -Force
    Copy-Item -Path $macLib -Destination $macArm64TargetDir -Force
    
    $mainLib = Join-Path $LIB_DIR "libDenOfIzGraphics.dylib"
    if (Test-Path $mainLib) {
        Copy-Item -Path $mainLib -Destination $macX64TargetDir -Force
        Copy-Item -Path $mainLib -Destination $macArm64TargetDir -Force
    }
    
    Write-Host "Added macOS runtime files to both x64 and arm64 from root directory" -ForegroundColor Green
}
elseif (-not (Test-Path $PLATFORM_LIBS_OSX_X64) -and -not (Test-Path $PLATFORM_LIBS_OSX_ARM64)) {
    Write-Host "Warning: macOS binaries not found (this is normal when building on Windows)" -ForegroundColor Yellow
}

# Create .csproj file
Write-Host "Creating project file..." -ForegroundColor Green
@"
<Project Sdk="Microsoft.NET.Sdk">
  <PropertyGroup>
    <TargetFrameworks>netstandard2.0;net6.0;net8.0;net9.0</TargetFrameworks>
    <LangVersion>8.0</LangVersion>
    <AllowUnsafeBlocks>true</AllowUnsafeBlocks>
    <GenerateDocumentationFile>true</GenerateDocumentationFile>
    <Nullable>enable</Nullable>

    <!-- NuGet package properties -->
    <PackageId>$PACKAGE_ID</PackageId>
    <Version>$VERSION</Version>
    <Authors>$AUTHORS</Authors>
    <Company>DenOfIz</Company>
    <Description>$DESCRIPTION</Description>
    <Copyright>$COPYRIGHT</Copyright>
    <PackageTags>$TAGS</PackageTags>
    <PackageProjectUrl>$PROJECT_URL</PackageProjectUrl>
    <RepositoryUrl>$REPOSITORY_URL</RepositoryUrl>
    <RepositoryType>git</RepositoryType>
    <PackageLicenseUrl>$LICENSE_URL</PackageLicenseUrl>
    
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
  
  <!-- Create lib folders for all target frameworks -->
  <ItemGroup>
    <!-- Copy compiled code to all target framework folders -->
    <Content Include="lib/netstandard2.0/*.cs">
      <PackagePath>lib/net6.0</PackagePath>
      <Pack>true</Pack>
    </Content>
    <Content Include="lib/netstandard2.0/*.cs">
      <PackagePath>lib/net8.0</PackagePath>
      <Pack>true</Pack>
    </Content>
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
"@ | Out-File -FilePath (Join-Path $TEMP_DIR "$PACKAGE_ID.csproj") -Encoding utf8

# Create build props file
Write-Host "Creating build props file..." -ForegroundColor Green
@"
<?xml version="1.0" encoding="utf-8"?>
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup>
    <DenOfIzGraphicsNativeLibsPath>`$(MSBuildThisFileDirectory)/../runtimes</DenOfIzGraphicsNativeLibsPath>
  </PropertyGroup>
</Project>
"@ | Out-File -FilePath (Join-Path $TEMP_DIR "build\$PACKAGE_ID.props") -Encoding utf8

# Create build targets file
Write-Host "Creating build targets file..." -ForegroundColor Green
@"
<?xml version="1.0" encoding="utf-8"?>
<Project xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <!-- Windows-specific targets -->
  <ItemGroup Condition="'`$(OS)' == 'Windows_NT'">
    <None Include="`$(DenOfIzGraphicsNativeLibsPath)/win-x64/native/*.dll">
      <Link>%(Filename)%(Extension)</Link>
      <CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory>
    </None>
  </ItemGroup>
  
  <!-- Linux-specific targets -->
  <ItemGroup Condition="'`$(OS)' != 'Windows_NT' AND `$([MSBuild]::IsOSPlatform('Linux'))">
    <None Include="`$(DenOfIzGraphicsNativeLibsPath)/linux-x64/native/*.so">
      <Link>%(Filename)%(Extension)</Link>
      <CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory>
    </None>
  </ItemGroup>
  
  <!-- macOS-specific targets -->
  <ItemGroup Condition="'`$(OS)' != 'Windows_NT' AND `$([MSBuild]::IsOSPlatform('OSX'))">
    <!-- Check for ARM64 macOS -->
    <None Include="`$(DenOfIzGraphicsNativeLibsPath)/osx-arm64/native/*.dylib"
          Condition="`$([System.Runtime.InteropServices.RuntimeInformation]::ProcessArchitecture) == 'Arm64'">
      <Link>%(Filename)%(Extension)</Link>
      <CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory>
    </None>
    
    <!-- Check for x64 macOS -->
    <None Include="`$(DenOfIzGraphicsNativeLibsPath)/osx-x64/native/*.dylib"
          Condition="`$([System.Runtime.InteropServices.RuntimeInformation]::ProcessArchitecture) == 'X64'">
      <Link>%(Filename)%(Extension)</Link>
      <CopyToOutputDirectory>PreserveNewest</CopyToOutputDirectory>
    </None>
  </ItemGroup>
</Project>
"@ | Out-File -FilePath (Join-Path $TEMP_DIR "build\$PACKAGE_ID.targets") -Encoding utf8

# Create README
Write-Host "Creating package README..." -ForegroundColor Green
@"
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
"@ | Out-File -FilePath (Join-Path $TEMP_DIR "README.md") -Encoding utf8

# Create the package
Write-Host "Building NuGet package..." -ForegroundColor Green
Push-Location $TEMP_DIR
dotnet pack -c Release -o $NUPKG_DIR
Pop-Location

# Check if package was created
$nupkgFiles = Get-ChildItem -Path $NUPKG_DIR -Filter "*.nupkg" -ErrorAction SilentlyContinue
if ($nupkgFiles.Count -gt 0) {
    Write-Host "NuGet package created successfully:" -ForegroundColor Green
    $nupkgFiles | ForEach-Object { Write-Host $_.FullName }
    Write-Host ""
    Write-Host "You can install the package locally with:" -ForegroundColor Cyan
    Write-Host "dotnet add package $PACKAGE_ID --source $NUPKG_DIR" -ForegroundColor Cyan
}
else {
    Write-Host "Error: NuGet package creation failed" -ForegroundColor Red
    exit 1
}

Write-Host "Done!" -ForegroundColor Green