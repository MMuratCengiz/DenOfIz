# How to Build DenOfIz

This document provides detailed instructions for building DenOfIz on various platforms and with different configurations.

## Prerequisites

Before using DenOfIz, ensure you have the following prerequisites installed:

### All Platforms
- CMake (3.17.2 or higher)
- C++23 compatible compiler
- vcpkg (included as a submodule)
- SWIG (if building language bindings)

### Windows
- Windows SDK
- Visual Studio 2022 or higher with C++ workload
- DirectX12 compatible graphics hardware

### macOS
- Xcode 14 or higher
- Metal compatible graphics hardware
- Required packages:
  ```bash
  brew install ninja swig mono freetype harfbuzz sdl2 autoconf automake autoconf-archive
  ```

### Linux
- GCC 11 or Clang 15+
- Vulkan SDK
- Required packages:
  ```bash
  sudo apt-get update
  sudo apt-get install -y ninja-build build-essential swig mono-complete
  sudo apt-get install -y libglu1-mesa-dev freeglut3-dev libvulkan-dev libsdl2-dev
  sudo apt-get install -y libfreetype-dev libharfbuzz-dev
  sudo apt-get install -y libltdl-dev libtool autoconf automake autoconf-archive
  sudo apt-get install -y libwayland-dev libxkbcommon-dev libegl1-mesa-dev
  sudo apt-get install -y libibus-1.0-dev python3-jinja2
  ```

## Building

1. Clone the repository with submodules:
   ```bash
   git clone --recursive https://github.com/MMuratCengiz/DenOfIz.git
   cd DenOfIz
   ```

2. Build the library:
   ```bash
   cmake -S . -B build
   cmake --build build --config Release
   ```

3. To build with examples:
   ```bash
   cmake -DBUILD_EXAMPLES=ON -S . -B build
   cmake --build build --config Release --target DenOfIzExamples-SimpleTriangle
   ```

4. Run an example:
   ```bash
   # On Windows
   build/Examples/SimpleTriangle/SimpleTriangle.exe
   
   # On macOS/Linux
   ./build/Examples/SimpleTriangle/SimpleTriangle
   ```
   
## Build Options

DenOfIz provides several CMake options to customize your build:

| Option | Description | Default |
| ------ | ----------- | ------- |
| BUILD_EXAMPLES | Build example applications | OFF |
| BUILD_TESTS | Build test suite | OFF |
| BUILD_SHARED_LIBS | Build as shared libraries instead of static | OFF |
| SWIG_CSHARP | Build C# bindings | OFF |
| CRT_LINKAGE_STATIC | On Windows, link MSVC runtime statically | ON |

## Platform-Specific Instructions

### Windows

1. Clone the repository with submodules:
   ```bash
   git clone --recursive https://github.com/your-org/DenOfIz.git
   cd DenOfIz
   ```

2. Configure and build:
   ```bash
   cmake -S . -B build
   cmake --build build --config Release
   ```

3. To build with Visual Studio solution:
   ```bash
   cmake -S . -B build -G "Visual Studio 17 2022" -A x64
   ```
   Then open `build/DenOfIz.sln` in Visual Studio.

### macOS

1. Clone the repository with submodules:
   ```bash
   git clone --recursive https://github.com/your-org/DenOfIz.git
   cd DenOfIz
   ```

2. Configure and build:
   ```bash
   cmake -S . -B build
   cmake --build build --config Release
   ```

3. To create an Xcode project:
   ```bash
   cmake -S . -B build -G Xcode
   ```
   Then open `build/DenOfIz.xcodeproj` in Xcode.

### Linux

1. Install required dependencies:
   ```bash
   # Ubuntu/Debian
   sudo apt-get update
   sudo apt-get install build-essential libvulkan-dev libsdl2-dev \
                        libx11-dev libxrandr-dev libxinerama-dev \
                        libxcursor-dev libxi-dev
   
   # Fedora
   sudo dnf install gcc-c++ vulkan-devel SDL2-devel \
                    libX11-devel libXrandr-devel libXinerama-devel \
                    libXcursor-devel libXi-devel
   ```

2. Clone the repository with submodules:
   ```bash
   git clone --recursive https://github.com/your-org/DenOfIz.git
   cd DenOfIz
   ```

3. Configure and build:
   ```bash
   cmake -S . -B build
   cmake --build build --config Release
   ```

## Testing

To build and run tests:

```bash
cmake -DBUILD_TESTS=ON -S . -B build
cmake --build build --target DenOfIzGraphics-Tests
cd build && ctest
```

To run a specific test:

```bash
cd build && ./Graphics/Tests/DenOfIzGraphics-Tests --gtest_filter=TestName*
```

## Building C# Bindings

DenOfIz supports C# bindings through SWIG:

```bash
cmake -DSWIG_CSHARP=ON -S . -B build
cmake --build build --config Release --target DenOfIzGraphicsCSharp
```

The C# bindings will be generated in the build directory. Swig/CSharp

However realistically the best way to use .net is to let github actions generate the nuget package.

## FAQ

No questions have been asked yet :(
