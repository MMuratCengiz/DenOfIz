#!/bin/bash
# Bash script to test the NuGet package structure
# This script doesn't install the package but verifies its structure

set -e

PACKAGE_PATH="${1:-./nuget_output}"

if [ ! -d "$PACKAGE_PATH" ]; then
    echo "Error: Package directory not found: $PACKAGE_PATH"
    exit 1
fi

NUPKG_FILES=$(find "$PACKAGE_PATH" -name "*.nupkg" | sort -r)
if [ -z "$NUPKG_FILES" ]; then
    echo "Error: No .nupkg files found in $PACKAGE_PATH"
    exit 1
fi

LATEST_PACKAGE=$(echo "$NUPKG_FILES" | head -n 1)
echo "Testing package: $(basename "$LATEST_PACKAGE")"

# Create a temporary directory for extraction
TEMP_DIR=$(mktemp -d)
trap 'rm -rf "$TEMP_DIR"' EXIT

# Rename .nupkg to .zip for extraction
ZIP_PATH="$TEMP_DIR/package.zip"
cp "$LATEST_PACKAGE" "$ZIP_PATH"

# Extract the package
echo "Extracting package to $TEMP_DIR"
mkdir -p "$TEMP_DIR/extracted"
if command -v unzip >/dev/null 2>&1; then
    unzip -q "$ZIP_PATH" -d "$TEMP_DIR/extracted"
else
    echo "Error: unzip command not found. Please install unzip to test the package."
    exit 1
fi

# Check for required files and directories
REQUIRED_PATHS=(
    "build/DenOfIzGraphics.targets"
    "docs/README.md"
    "lib/netstandard2.0"
)

RUNTIME_PATHS=(
    "runtimes/win-x64/native"
    "runtimes/win-x86/native"
    "runtimes/linux-x64/native"
    "runtimes/osx-x64/native"
)

echo -e "\nChecking required paths..."
for path in "${REQUIRED_PATHS[@]}"; do
    if [ -e "$TEMP_DIR/extracted/$path" ]; then
        echo -e "\033[32m✓ Found: $path\033[0m"
    else
        echo -e "\033[31m✗ Missing: $path\033[0m"
    fi
done

echo -e "\nChecking runtime directories..."
for path in "${RUNTIME_PATHS[@]}"; do
    if [ -d "$TEMP_DIR/extracted/$path" ]; then
        echo -e "\033[32m✓ Found: $path\033[0m"
        
        # Get contents of runtime directory
        FILES=$(find "$TEMP_DIR/extracted/$path" -type f 2>/dev/null)
        if [ -n "$FILES" ]; then
            echo "  Files:"
            while IFS= read -r file; do
                echo "    - $(basename "$file")"
            done <<< "$FILES"
        else
            echo -e "\033[33m  No files found in directory (this is expected if native libraries weren't copied)\033[0m"
        fi
    else
        echo -e "\033[31m✗ Missing: $path\033[0m"
    fi
done

# Check nuspec file
echo -e "\nChecking .nuspec file..."
NUSPEC_PATH="$TEMP_DIR/extracted/DenOfIzGraphics.nuspec"
if [ -f "$NUSPEC_PATH" ]; then
    echo -e "\033[32m✓ Found: DenOfIzGraphics.nuspec\033[0m"
    
    # Read basic info from the nuspec file
    if command -v xmllint >/dev/null 2>&1; then
        PACKAGE_ID=$(xmllint --xpath "string(/package/metadata/id)" "$NUSPEC_PATH" 2>/dev/null)
        PACKAGE_VERSION=$(xmllint --xpath "string(/package/metadata/version)" "$NUSPEC_PATH" 2>/dev/null)
        PACKAGE_AUTHORS=$(xmllint --xpath "string(/package/metadata/authors)" "$NUSPEC_PATH" 2>/dev/null)
        
        echo "  Package ID: $PACKAGE_ID"
        echo "  Version: $PACKAGE_VERSION"
        echo "  Authors: $PACKAGE_AUTHORS"
    else
        echo "  Note: xmllint not found. Install libxml2-utils for more detailed nuspec info."
        grep -o "<id>[^<]*</id>" "$NUSPEC_PATH" | sed 's/<id>\(.*\)<\/id>/  Package ID: \1/'
        grep -o "<version>[^<]*</version>" "$NUSPEC_PATH" | sed 's/<version>\(.*\)<\/version>/  Version: \1/'
    fi
else
    echo -e "\033[31m✗ Missing: DenOfIzGraphics.nuspec\033[0m"
fi

echo -e "\nPackage structure validation complete!"