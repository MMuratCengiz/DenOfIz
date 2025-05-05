#!/bin/bash
# Script to build and package the DenOfIzGraphics NuGet package for all platforms

set -e

VERSION="1.0.0"
PACKAGE_DIR="./nuget_package"
OUTPUT_DIR="./nuget_output"
CURRENT_DIR=$(pwd)

rm -rf "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR"
mkdir -p "$OUTPUT_DIR"

mkdir -p "$PACKAGE_DIR/lib/netstandard2.0"
mkdir -p "$PACKAGE_DIR/runtimes/win-x64/native"
mkdir -p "$PACKAGE_DIR/runtimes/win-x86/native"
mkdir -p "$PACKAGE_DIR/runtimes/linux-x64/native"
mkdir -p "$PACKAGE_DIR/runtimes/osx-x64/native"
mkdir -p "$PACKAGE_DIR/build"
mkdir -p "$PACKAGE_DIR/docs"

# Copy README.md to package
cp "$(dirname "$0")/README.md" "$PACKAGE_DIR/docs/"

# Process .nuspec.in and .targets.in templates
# Replace variables with values
sed \
    -e "s/@PROJECT_VERSION@/$VERSION/g" \
    -e "s/@CURRENT_YEAR@/$(date +%Y)/g" \
    "$(dirname "$0")/DenOfIzGraphics.nuspec.in" > "$PACKAGE_DIR/DenOfIzGraphics.nuspec"

cp "$(dirname "$0")/DenOfIzGraphics.targets.in" "$PACKAGE_DIR/build/DenOfIzGraphics.targets"

# Copy the managed assembly and documentation to the lib folder
# Assuming the build has already been performed
# cp path/to/managed/assembly.dll "$PACKAGE_DIR/lib/netstandard2.0/"
# cp path/to/managed/assembly.xml "$PACKAGE_DIR/lib/netstandard2.0/"

# Function to copy native libraries for a specific platform
copy_platform_libs() {
    local platform="$1"
    local lib_dir="$2"
    local dest_dir="$PACKAGE_DIR/runtimes/$platform/native"
    
    echo "Copying libraries for $platform..."
    
    if [ -d "$lib_dir" ]; then
        # Copy all the native libraries for this platform
        case "$platform" in
            win-*)
                find "$lib_dir" -name "*.dll" -exec cp {} "$dest_dir/" \;
                ;;
            linux-*)
                find "$lib_dir" -name "*.so" -exec cp {} "$dest_dir/" \;
                ;;
            osx-*)
                find "$lib_dir" -name "*.dylib" -exec cp {} "$dest_dir/" \;
                ;;
        esac
    else
        echo "Warning: Directory $lib_dir not found. Skipping $platform."
    fi
}

# Platform-specific builds
# Example paths - you'll need to adjust these based on your actual build output locations
copy_platform_libs "win-x64" "../../../build/CSharp/Project/Lib/Windows-x64"
copy_platform_libs "win-x86" "../../../build/CSharp/Project/Lib/Windows-x86"
copy_platform_libs "linux-x64" "../../../build/CSharp/Project/Lib/Linux-x64"
copy_platform_libs "osx-x64" "../../../build/CSharp/Project/Lib/OSX-x64"

# Build the NuGet package
echo "Building NuGet package..."
cd "$PACKAGE_DIR"
nuget pack "./DenOfIzGraphics.nuspec" -OutputDirectory "$CURRENT_DIR/$OUTPUT_DIR"

# Clean up
cd "$CURRENT_DIR"
echo "Package created in $OUTPUT_DIR"