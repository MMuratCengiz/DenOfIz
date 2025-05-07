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
                # For Windows, don't copy the managed assembly as a native library
                find "$lib_dir" -name "*.dll" -and -not -name "DenOfIzGraphicsCSharp.dll" -exec cp -v {} "$dest_dir/" \;
                ;;
            linux-*)
                find "$lib_dir" -name "*.so" -exec cp -v {} "$dest_dir/" \;
                ;;
            osx-*)
                find "$lib_dir" -name "*.dylib" -exec cp -v {} "$dest_dir/" \;
                ;;
        esac
        
        # Copy managed assembly for netstandard2.0
        if [ "$platform" = "win-x64" ]; then
            local managed_dll="$lib_dir/DenOfIzGraphicsCSharp.dll"
            if [ -f "$managed_dll" ]; then
                cp -v "$managed_dll" "$PACKAGE_DIR/lib/netstandard2.0/DenOfIzGraphics.dll"
                echo "Copied managed assembly to lib/netstandard2.0"
                
                # Check for XML documentation file
                local xml_doc="${managed_dll%.dll}.xml"
                if [ -f "$xml_doc" ]; then
                    cp -v "$xml_doc" "$PACKAGE_DIR/lib/netstandard2.0/DenOfIzGraphics.xml"
                    echo "Copied XML documentation to lib/netstandard2.0"
                    
                    # Add XML doc file to nuspec
                    if ! grep -q "lib\\\\netstandard2.0\\\\DenOfIzGraphics.xml" "$PACKAGE_DIR/DenOfIzGraphics.nuspec"; then
                        # Find the files section and append the XML doc entry
                        sed -i '/<files>/a \    <file src="lib\\netstandard2.0\\DenOfIzGraphics.xml" target="lib\\netstandard2.0" />' "$PACKAGE_DIR/DenOfIzGraphics.nuspec"
                        echo "Updated nuspec to include XML documentation file"
                    fi
                } else {
                    echo "XML documentation not found, skipping"
                }
                fi
            else
                echo "Warning: Managed assembly not found at $managed_dll"
            fi
        fi
    else
        echo "Warning: Directory $lib_dir not found. Skipping $platform."
    fi
}

# Determine repository root and build directories
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="$REPO_ROOT/build"

# Try to find Windows build locations (for cross-platform builds)
WIN_X64_PATH=""
for config in "Debug_MSVC" "Release_MSVC" "Debug" "Release"; do
    test_path="$BUILD_DIR/$config/CSharp/Project/Lib/win-x64"
    if [ -d "$test_path" ]; then
        WIN_X64_PATH="$test_path"
        break
    fi
done

if [ -z "$WIN_X64_PATH" ]; then
    echo "Warning: Could not find Windows x64 build path. Will try generic build folder."
    WIN_X64_PATH="$BUILD_DIR/DenOfIz/Debug_MSVC/CSharp/Project/Lib/win-x64"
    if [ ! -d "$WIN_X64_PATH" ]; then
        WIN_X64_PATH="$BUILD_DIR/CSharp/Project/Lib/win-x64"
    fi
fi

# Windows x86 path
WIN_X86_PATH="${WIN_X64_PATH/win-x64/win-x86}"

# Linux and macOS paths
LINUX_PATH="$BUILD_DIR/CSharp/Project/Lib/linux-x64"
OSX_PATH="$BUILD_DIR/CSharp/Project/Lib/osx-x64"

# Copy libraries for each platform
copy_platform_libs "win-x64" "$WIN_X64_PATH"
copy_platform_libs "win-x86" "$WIN_X86_PATH"
copy_platform_libs "linux-x64" "$LINUX_PATH"
copy_platform_libs "osx-x64" "$OSX_PATH"

# Build the NuGet package
echo "Building NuGet package..."
cd "$PACKAGE_DIR"
nuget pack "./DenOfIzGraphics.nuspec" -OutputDirectory "$CURRENT_DIR/$OUTPUT_DIR"

# Clean up
cd "$CURRENT_DIR"
echo "Package created in $OUTPUT_DIR"