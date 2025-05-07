#!/usr/bin/env pwsh
# PowerShell script to build and package the DenOfIzGraphics NuGet package for all platforms

# Stop on first error
$ErrorActionPreference = "Stop"

$VERSION = "1.0.0"
$PACKAGE_DIR = "./nuget_package"
$OUTPUT_DIR = "./nuget_output"
$CURRENT_DIR = (Get-Location).Path

# Clean and create directories
if (Test-Path $PACKAGE_DIR) {
    Remove-Item -Path $PACKAGE_DIR -Recurse -Force
}
New-Item -Path $PACKAGE_DIR -ItemType Directory -Force | Out-Null
New-Item -Path $OUTPUT_DIR -ItemType Directory -Force | Out-Null

# Create required directories
$directories = @(
    "$PACKAGE_DIR/lib/netstandard2.0",
    "$PACKAGE_DIR/runtimes/win-x64/native",
    "$PACKAGE_DIR/runtimes/win-x86/native",
    "$PACKAGE_DIR/runtimes/linux-x64/native",
    "$PACKAGE_DIR/runtimes/osx-x64/native",
    "$PACKAGE_DIR/build",
    "$PACKAGE_DIR/docs"
)

foreach ($dir in $directories) {
    New-Item -Path $dir -ItemType Directory -Force | Out-Null
}

# Copy README.md to package
$scriptDir = Split-Path -Parent $MyInvocation.MyCommand.Path
Copy-Item -Path "$scriptDir/README.md" -Destination "$PACKAGE_DIR/docs/" -Force

# Process .nuspec.in and .targets.in templates
# Replace variables with values
$nuspecContent = Get-Content -Path "$scriptDir/DenOfIzGraphics.nuspec.in" -Raw
$nuspecContent = $nuspecContent -replace '@PROJECT_VERSION@', $VERSION
$nuspecContent = $nuspecContent -replace '@CURRENT_YEAR@', (Get-Date).Year
$nuspecContent | Set-Content -Path "$PACKAGE_DIR/DenOfIzGraphics.nuspec" -Force

Copy-Item -Path "$scriptDir/DenOfIzGraphics.targets.in" -Destination "$PACKAGE_DIR/build/DenOfIzGraphics.targets" -Force

# Function to copy native libraries for a specific platform
function Copy-PlatformLibs {
    param (
        [string] $platform,
        [string] $libDir
    )
    
    $destDir = "$PACKAGE_DIR/runtimes/$platform/native"
    
    Write-Host "Copying libraries for $platform..."
    
    if (Test-Path $libDir) {
        # Copy all the native libraries for this platform
        switch -Wildcard ($platform) {
            "win-*" {
                # Copy DLLs
                Get-ChildItem -Path $libDir -Filter "*.dll" -Recurse | ForEach-Object {
                    # Skip the managed assembly as we will handle it separately
                    if ($_.Name -ne "DenOfIzGraphicsCSharp.dll") {
                        Copy-Item -Path $_.FullName -Destination $destDir -Force
                        Write-Host "  Copied: $($_.Name)"
                    }
                }
            }
            "linux-*" {
                Get-ChildItem -Path $libDir -Filter "*.so" -Recurse | ForEach-Object {
                    Copy-Item -Path $_.FullName -Destination $destDir -Force
                    Write-Host "  Copied: $($_.Name)"
                }
            }
            "osx-*" {
                Get-ChildItem -Path $libDir -Filter "*.dylib" -Recurse | ForEach-Object {
                    Copy-Item -Path $_.FullName -Destination $destDir -Force
                    Write-Host "  Copied: $($_.Name)"
                }
            }
        }
        
        # Copy managed assembly for netstandard2.0
        if ($platform -eq "win-x64") {
            $managedDll = Join-Path -Path $libDir -ChildPath "DenOfIzGraphicsCSharp.dll"
            if (Test-Path $managedDll) {
                Copy-Item -Path $managedDll -Destination "$PACKAGE_DIR/lib/netstandard2.0/DenOfIzGraphics.dll" -Force
                Write-Host "Copied managed assembly to lib/netstandard2.0"
                
                # Check for XML documentation file
                $xmlDoc = $managedDll -replace "\.dll$", ".xml"
                if (Test-Path $xmlDoc) {
                    Copy-Item -Path $xmlDoc -Destination "$PACKAGE_DIR/lib/netstandard2.0/DenOfIzGraphics.xml" -Force
                    Write-Host "Copied XML documentation to lib/netstandard2.0"
                    
                    # Update the nuspec to include the XML documentation file
                    $nuspecPath = "$PACKAGE_DIR/DenOfIzGraphics.nuspec"
                    $nuspecXml = [xml](Get-Content $nuspecPath)
                    $filesNode = $nuspecXml.package.files
                    
                    # Check if there's already an XML file entry
                    $xmlEntry = $filesNode.file | Where-Object { $_.src -eq "lib\netstandard2.0\DenOfIzGraphics.xml" }
                    if (-not $xmlEntry) {
                        $xmlFileElement = $nuspecXml.CreateElement("file")
                        $xmlFileElement.SetAttribute("src", "lib\netstandard2.0\DenOfIzGraphics.xml")
                        $xmlFileElement.SetAttribute("target", "lib\netstandard2.0")
                        $filesNode.AppendChild($xmlFileElement) | Out-Null
                        $nuspecXml.Save($nuspecPath)
                        Write-Host "Updated nuspec to include XML documentation file"
                    }
                } else {
                    Write-Host "XML documentation not found, skipping" -ForegroundColor Yellow
                }
            } else {
                Write-Host "Warning: Managed assembly not found at $managedDll" -ForegroundColor Yellow
            }
        }
    } else {
        Write-Host "Warning: Directory $libDir not found. Skipping $platform." -ForegroundColor Yellow
    }
}

# Platform-specific builds
# Updated paths to match the actual build structure
$repoRoot = Resolve-Path (Join-Path $PSScriptRoot "../..")
$buildDir = Join-Path $repoRoot "build" 

# For Windows, look for Debug or Release configuration
$winConfigDirs = @("Debug_MSVC", "Release_MSVC", "Debug", "Release")
$winX64Path = $null
$winX86Path = $null

foreach ($config in $winConfigDirs) {
    $testPath = Join-Path $buildDir "$config/CSharp/Project/Lib/win-x64"
    if (Test-Path $testPath) {
        $winX64Path = $testPath
        break
    }
}

if (-not $winX64Path) {
    Write-Host "Warning: Could not find Windows x64 build path. Will try generic build folder." -ForegroundColor Yellow
    $winX64Path = Join-Path $buildDir "DenOfIz/Debug_MSVC/CSharp/Project/Lib/win-x64"
    if (-not (Test-Path $winX64Path)) {
        $winX64Path = Join-Path $buildDir "CSharp/Project/Lib/win-x64"
    }
}

# Similarly for x86 if needed
$winX86Path = $winX64Path -replace "win-x64", "win-x86"

# For Linux and macOS
$linuxPath = Join-Path $buildDir "CSharp/Project/Lib/linux-x64"
$osxPath = Join-Path $buildDir "CSharp/Project/Lib/osx-x64"

# Copy libraries
Copy-PlatformLibs -platform "win-x64" -libDir $winX64Path
Copy-PlatformLibs -platform "win-x86" -libDir $winX86Path
Copy-PlatformLibs -platform "linux-x64" -libDir $linuxPath
Copy-PlatformLibs -platform "osx-x64" -libDir $osxPath

# Build the NuGet package
Write-Host "Building NuGet package..."
Push-Location $PACKAGE_DIR
nuget pack "./DenOfIzGraphics.nuspec" -OutputDirectory "$CURRENT_DIR/$OUTPUT_DIR"
Pop-Location

# Clean up
Write-Host "Package created in $OUTPUT_DIR"