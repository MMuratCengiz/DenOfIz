# How to Build DenOfIz

DenOfIz builds with CMake presets (`CMakePresets.json`) and vcpkg, which is a submodule. The first configure builds the `vcpkg.json` dependencies, SDL3_mixer is fetched at
configure time.

```bash
git clone --recursive https://github.com/MMuratCengiz/DenOfIz.git
cd DenOfIz
```

## Prerequisites

All platforms: CMake 3.21 or newer and a C++20 compiler. The package lists below are what CI installs.

### Windows

- Visual Studio 2026 with the C++ workload and a Windows SDK, the presets use the `Visual Studio 18 2026` generator
- C# bindings: LLVM (`choco install llvm`) and the .NET 8 SDK

### macOS (arm64)

- Xcode, the presets use the Xcode generator
- `brew install ninja sdl2 autoconf automake autoconf-archive`
- C# bindings: `brew install llvm` and the .NET 8 SDK

### Linux

- Clang and Ninja, the presets set `CC=clang` and `CXX=clang++`, CI uses Clang 18 on Ubuntu 24.04
- Packages:
  ```bash
  sudo apt-get install -y ninja-build build-essential
  sudo apt-get install -y libglu1-mesa-dev freeglut3-dev libvulkan-dev libsdl2-dev
  sudo apt-get install -y libltdl-dev libtool autoconf automake autoconf-archive
  sudo apt-get install -y libwayland-dev libxkbcommon-dev libegl1-mesa-dev
  sudo apt-get install -y libibus-1.0-dev libxtst-dev
  sudo apt-get install -y python3 python3-pip python3-jinja2 python3-packaging
  sudo apt-get install -y libcap-dev libmount-dev libsystemd-dev
  sudo apt-get install -y gperf m4
  ```

## Presets

| Platform      | Presets                                                                                                          |
|---------------|------------------------------------------------------------------------------------------------------------------|
| Windows x64   | `Debug_MSVC`, `Release_MSVC`, `Debug_Shared_MSVC`, `Release_Shared_MSVC`                                         |
| Windows arm64 | `Debug_MSVC_ARM64`, `Release_MSVC_ARM64`                                                                         |
| macOS arm64   | `Debug_OSX`, `Release_OSX`                                                                                       |
| Linux x64     | `Debug_Linux`, `Release_Linux`, `Debug_Shared_Linux`, `Release_Shared_Linux`                                     |
| Linux arm64   | `Debug_Linux_ARM64`, `Release_Linux_ARM64`                                                                       |
| Install       | `Install_Windows`, `Install_Windows_Debug`, `Install_Windows_ARM64`, `Install_OSX`, `Install_Linux`, `Install_Linux_ARM64` |

Debug presets build the tests and examples (`Debug_MSVC` the tools too), Release presets only the library. Install presets build a shared library and the bindings and install
them to `install/DenOfIz/<preset>`.

## Building

```bash
cmake --preset Debug_OSX
cmake --build build/DenOfIz/Debug_OSX --config Debug
cmake --build build/DenOfIz/Debug_OSX --config Debug --target DenOfIzExamples-SimpleTriangle
```

`--config` matters for the Visual Studio and Xcode generators, the Linux presets use Ninja with the preset's build type. Each configure preset also has a build preset of the same
name: `cmake --build --preset Debug_OSX`.

Executables, examples included, are written to `build/DenOfIz/<preset>`. Examples are the `DenOfIzExamples-<Name>` targets (`DenOfIzExamples-ImGuiExample` for ImGui), their
`Assets` folder is copied next to them, so run them from that directory. On macOS they are `.app` bundles with the assets inside.

### Using the installed library

```bash
cmake --preset Install_Linux
cmake --build --preset Install_Linux
```

With `CMAKE_PREFIX_PATH` pointing at `install/DenOfIz/Install_Linux`:

```cmake
find_package(DenOfIzGraphics CONFIG REQUIRED)
target_link_libraries(MyApp PRIVATE DenOfIz::DenOfIzGraphics)
denofiz_setup_target(MyApp) # copies DenOfIzGraphics and the shader compiler libraries next to MyApp
```

## Options

| Option               | Description                                                 | Default |
|----------------------|-------------------------------------------------------------|---------|
| `BUILD_TESTS`        | Build `DenOfIzGraphics-Tests`                               | OFF     |
| `BUILD_EXAMPLES`     | Build the examples, needs the `examples` vcpkg feature      | OFF     |
| `BUILD_TOOLS`        | Build `ShaderToEmbeddedInl` and `FontToEmbeddedInl`         | OFF     |
| `BUILD_BINDINGS`     | Build the C# binding generator, skipped when LLVM is missing | OFF     |
| `BUILD_SHARED_LIBS`  | Build DenOfIzGraphics as a shared library                   | OFF     |
| `CRT_LINKAGE_STATIC` | Link the MSVC runtime statically                            | ON      |

vcpkg features go in `VCPKG_MANIFEST_FEATURES`: `examples` (Assimp, required by `BUILD_EXAMPLES`), `pix` (PIX event runtime for DX12 debug markers) and `d3d12-agility`.
For example `-DBUILD_EXAMPLES=ON -DVCPKG_MANIFEST_FEATURES=examples` on a Release preset.

## Tests

GoogleTest, in `Graphics/Tests`:

```bash
cmake --build build/DenOfIz/Debug_Linux --target DenOfIzGraphics-Tests
ctest --test-dir build/DenOfIz/Debug_Linux/Graphics/Tests
./build/DenOfIz/Debug_Linux/DenOfIzGraphics-Tests --gtest_filter=TextGrapheme.*
```

Add `-C Debug` to `ctest` with the Visual Studio and Xcode generators.

## Tools

With `BUILD_TOOLS=ON`:

- `ShaderToEmbeddedInl` regenerates the embedded UI, TextRenderer and ImGui shaders. Run it from the repository root with `--generate-all`, or
  `--generate-ui`, `--generate-text-renderer`, `--generate-imgui`.
- `FontToEmbeddedInl <font file> <output.inl>` embeds a font file.

## C# Bindings

The bindings are generated from the public C headers by the libclang based generator in `Bindings/`. It needs LLVM: on Windows set the `LLVM_DIR` environment variable to the LLVM
install root (for example `C:\Program Files\LLVM`), elsewhere LLVM must be found by `find_package(LLVM CONFIG)`, for example with `-DLLVM_DIR=<llvm>/lib/cmake/llvm`.

```bash
cmake --preset Install_Windows
cmake --build build/DenOfIz/Install_Windows --config Release --target DenOfIzCSharpGenerate
```

`DenOfIzCSharpGenerate` writes `Bindings/CSharp/Generated`, which `Bindings/CSharp/DenOfIzGraphicsCSharp.csproj` compiles together with the hand written `Wrappers` and `Runtime`.
`DenOfIzCSharpBuild` (needs `dotnet`) also copies the native libraries into `Bindings/CSharp/Native` and builds the project and the `Bindings/CSharp/DenOfIzGraphicsExample` sample.

The `DenOfIzGraphics` NuGet package (net8.0, win-x64, linux-x64, osx-arm64) is built by the manually triggered `.github/workflows/nuget-package.yml`: it builds the Install
presets on each platform, generates the C# sources on Windows and packs everything with the version from `version.txt`.
