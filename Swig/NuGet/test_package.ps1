#!/usr/bin/env pwsh
# PowerShell script to test the NuGet package structure
# This script doesn't install the package but verifies its structure

param(
    [string]$PackagePath = "./nuget_output"
)

$ErrorActionPreference = "Stop"

if (-not (Test-Path $PackagePath)) {
    Write-Error "Package directory not found: $PackagePath"
    exit 1
}

$nupkgFiles = Get-ChildItem -Path $PackagePath -Filter "*.nupkg"
if ($nupkgFiles.Count -eq 0) {
    Write-Error "No .nupkg files found in $PackagePath"
    exit 1
}

$latestPackage = $nupkgFiles | Sort-Object LastWriteTime -Descending | Select-Object -First 1
Write-Host "Testing package: $($latestPackage.Name)"

# Create a temporary directory for extraction
$tempDir = [System.IO.Path]::Combine([System.IO.Path]::GetTempPath(), [System.Guid]::NewGuid().ToString())
New-Item -Path $tempDir -ItemType Directory -Force | Out-Null

try {
    # Rename .nupkg to .zip for extraction
    $zipPath = [System.IO.Path]::Combine($tempDir, "package.zip")
    Copy-Item -Path $latestPackage.FullName -Destination $zipPath -Force
    
    # Extract the package
    Write-Host "Extracting package to $tempDir"
    Expand-Archive -Path $zipPath -DestinationPath "$tempDir/extracted" -Force
    
    # Check for required files and directories
    $requiredPaths = @(
        "build/DenOfIzGraphics.targets",
        "docs/README.md",
        "lib/netstandard2.0"
    )
    
    $runtimePaths = @(
        "runtimes/win-x64/native",
        "runtimes/win-x86/native",
        "runtimes/linux-x64/native",
        "runtimes/osx-x64/native"
    )
    
    Write-Host "`nChecking required paths..."
    foreach ($path in $requiredPaths) {
        $fullPath = [System.IO.Path]::Combine("$tempDir/extracted", $path)
        if (Test-Path $fullPath) {
            Write-Host "✓ Found: $path" -ForegroundColor Green
        } else {
            Write-Host "✗ Missing: $path" -ForegroundColor Red
        }
    }
    
    Write-Host "`nChecking runtime directories..."
    foreach ($path in $runtimePaths) {
        $fullPath = [System.IO.Path]::Combine("$tempDir/extracted", $path)
        if (Test-Path $fullPath) {
            Write-Host "✓ Found: $path" -ForegroundColor Green
            
            # Get contents of runtime directory
            $files = Get-ChildItem -Path $fullPath -File -ErrorAction SilentlyContinue
            if ($files.Count -gt 0) {
                Write-Host "  Files:" -ForegroundColor DarkGray
                foreach ($file in $files) {
                    Write-Host "    - $($file.Name)" -ForegroundColor DarkGray
                }
            } else {
                Write-Host "  No files found in directory (this is expected if native libraries weren't copied)" -ForegroundColor Yellow
            }
        } else {
            Write-Host "✗ Missing: $path" -ForegroundColor Red
        }
    }
    
    # Check nuspec file
    Write-Host "`nChecking .nuspec file..."
    $nuspecPath = [System.IO.Path]::Combine("$tempDir/extracted", "DenOfIzGraphics.nuspec")
    if (Test-Path $nuspecPath) {
        Write-Host "✓ Found: DenOfIzGraphics.nuspec" -ForegroundColor Green
        
        # Read the nuspec file
        [xml]$nuspec = Get-Content $nuspecPath
        Write-Host "  Package ID: $($nuspec.package.metadata.id)" -ForegroundColor DarkGray
        Write-Host "  Version: $($nuspec.package.metadata.version)" -ForegroundColor DarkGray
        Write-Host "  Authors: $($nuspec.package.metadata.authors)" -ForegroundColor DarkGray
    } else {
        Write-Host "✗ Missing: DenOfIzGraphics.nuspec" -ForegroundColor Red
    }
    
    Write-Host "`nPackage structure validation complete!"
} finally {
    # Clean up
    if (Test-Path $tempDir) {
        Remove-Item -Path $tempDir -Recurse -Force
    }
}