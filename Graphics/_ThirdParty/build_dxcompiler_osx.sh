#!/bin/bash
# Build script for DirectXShaderCompiler on macOS, "installs" dxc into osx_dxc in line with linux_dxc

set -e
set -x

SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd )"
DXC_SOURCE_DIR="${SCRIPT_DIR}/DirectXShaderCompiler"
BUILD_DIR="${DXC_SOURCE_DIR}/build"
OUTPUT_DIR="${SCRIPT_DIR}/osx_dxc"
INCLUDE_DIR="${OUTPUT_DIR}/include"
LIB_DIR="${OUTPUT_DIR}/lib"
BIN_DIR="${OUTPUT_DIR}/bin"

if [ -d "${OUTPUT_DIR}" ]; then
    echo "Cleaning previous output directory..."
    rm -rf "${OUTPUT_DIR}"
fi

# Output directories
mkdir -p "${OUTPUT_DIR}"
mkdir -p "${INCLUDE_DIR}/dxc/Support"
mkdir -p "${INCLUDE_DIR}/hlsl/vk/khr"
mkdir -p "${LIB_DIR}"
mkdir -p "${BIN_DIR}"

# Check if the source directory exists
if [ ! -d "${DXC_SOURCE_DIR}" ]; then
    echo "DirectXShaderCompiler source directory not found at ${DXC_SOURCE_DIR} run 'git submodule update --init --recursive'"
    exit 1
fi

cd "${DXC_SOURCE_DIR}"
git submodule update --init --recursive

rm -rf "${BUILD_DIR}"
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

cmake "${DXC_SOURCE_DIR}" \
    -C "${DXC_SOURCE_DIR}/cmake/caches/PredefinedParams.cmake" \
    -DCMAKE_BUILD_TYPE=Release \
    -DHLSL_OPTIONAL_PROJS_IN_DEFAULT=OFF \
    -DHLSL_ENABLE_ANALYZE=OFF \
    -DHLSL_OFFICIAL_BUILD=OFF \
    -DHLSL_ENABLE_FIXED_VER=OFF \
    -DHLSL_BUILD_DXILCONV=OFF \
    -DHLSL_INCLUDE_TESTS=OFF \
    -DENABLE_SPIRV_CODEGEN=OFF \
    -DSPIRV_BUILD_TESTS=OFF \
    -DLLVM_BUILD_RUNTIME=ON \
    -DLLVM_BUILD_EXAMPLES=OFF \
    -DLLVM_BUILD_TESTS=OFF \
    -DLLVM_INCLUDE_TESTS=OFF \
    -DLLVM_INCLUDE_DOCS=OFF \
    -DLLVM_INCLUDE_EXAMPLES=OFF \
    -DLLVM_OPTIMIZED_TABLEGEN=OFF \
    -DLLVM_APPEND_VC_REV=OFF \
    -DLLVM_ENABLE_RTTI=ON \
    -DLLVM_ENABLE_EH=ON \
    -DLLVM_BUILD_TOOLS=OFF \
    -DLLVM_INCLUDE_UTILS=OFF \
    -DCLANG_CL=OFF \
    -DCLANG_FORMAT_EXE="" \
    -DLLVM_TARGETS_TO_BUILD="None" \
    -DLLVM_DEFAULT_TARGET_TRIPLE="dxil-ms-dx" \
    -DCLANG_ENABLE_STATIC_ANALYZER=OFF \
    -DCLANG_ENABLE_ARCMT=OFF \
    -DCLANG_BUILD_EXAMPLES=OFF \
    -DCLANG_INCLUDE_TESTS=OFF \
    -DLLVM_ENABLE_TERMINFO=OFF \
    -DLIBCLANG_BUILD_STATIC=ON \
    -DLLVM_DISTRIBUTION_COMPONENTS="dxcompiler;dxc-headers" \
    -DBUILD_TESTING=OFF \
    -G Ninja

# Build
ninja dxcompiler

# Copy the minimal set of necessary include files to match the linux_dxc structure
echo "Copying include files..."

# dxc headers
cp "${DXC_SOURCE_DIR}/include/dxc/WinAdapter.h" "${INCLUDE_DIR}/dxc/"
cp "${DXC_SOURCE_DIR}/include/dxc/dxcapi.h" "${INCLUDE_DIR}/dxc/"
cp "${DXC_SOURCE_DIR}/include/dxc/dxcerrors.h" "${INCLUDE_DIR}/dxc/"
cp "${DXC_SOURCE_DIR}/include/dxc/dxcisense.h" "${INCLUDE_DIR}/dxc/"

cp "${DXC_SOURCE_DIR}/include/dxc/Support/ErrorCodes.h" "${INCLUDE_DIR}/dxc/Support/"

# Probably not necessary but keep it inline with linux_dxc
touch "${INCLUDE_DIR}/hlsl/LICENSE.txt"
touch "${INCLUDE_DIR}/hlsl/README.txt"

cp "${DXC_SOURCE_DIR}/include/llvm/SPIRV/spirv.h" "${INCLUDE_DIR}/hlsl/vk/spirv.h" 2>/dev/null || :
cp "${DXC_SOURCE_DIR}/include/llvm/SPIRV/opcode_selector.h" "${INCLUDE_DIR}/hlsl/vk/opcode_selector.h" 2>/dev/null || :
cp "${DXC_SOURCE_DIR}/external/SPIRV-Headers/include/spirv/unified1/spirv.h" "${INCLUDE_DIR}/hlsl/vk/spirv.h" 2>/dev/null || :
cp "${DXC_SOURCE_DIR}/external/SPIRV-Headers/include/spirv/unified1/spirv.h" "${INCLUDE_DIR}/hlsl/vk/khr/cooperative_matrix.h" 2>/dev/null || :
cp "${DXC_SOURCE_DIR}/external/SPIRV-Headers/include/spirv/unified1/spirv.h" "${INCLUDE_DIR}/hlsl/vk/khr/cooperative_matrix.impl" 2>/dev/null || :

echo "Copying library files..."
cp "${BUILD_DIR}/lib/libdxcompiler.dylib" "${LIB_DIR}/"

echo "Copying license files..."
cp "${DXC_SOURCE_DIR}/LICENSE.TXT" "${OUTPUT_DIR}/LICENSE-LLVM.txt"
cp "${DXC_SOURCE_DIR}/ThirdPartyNotices.txt" "${OUTPUT_DIR}/"

chmod +x "${LIB_DIR}/libdxcompiler.dylib"

install_name_tool -id "@rpath/libdxcompiler.dylib" "${LIB_DIR}/libdxcompiler.dylib"

echo "Build completed successfully!"
echo "Library files are available at: ${LIB_DIR}"
echo "Include files are available at: ${INCLUDE_DIR}"