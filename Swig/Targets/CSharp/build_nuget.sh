!/bin/bash

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

VERSION="1.0.0"
nuget pack NuGet/DenOfIzGraphics.nuspec -Version $VERSION -OutputDirectory ./NuGet_Out