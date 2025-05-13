# DenOfIz Graphics Library

> **⚠️ ALPHA SOFTWARE**: DenOfIz is currently in alpha stage. The API is subject to change, and the library may(will definitely) contain bugs or missing features. You may or may
> not know why your code just crashed. I still do hope you use and contribute to the library though!

DenOfIz is a modern, cross-platform graphics library that provides an abstraction layer over multiple graphics APIs including DirectX12, Vulkan, and Metal. It focuses on modern
rendering features and binding methods. No setUniforms or large state machines.

The API style should be familiar to anyone who used Vulkan or DirectX12, large inputs are passed via input structs, and CommandList recording is very similar in all backends.

## Why:

There are many great libraries that do what I attempt to do here, but I felt like they didn't quite satisfy what I wanted:

- Build system, DenOfIz uses CMake + vcpkg.json, and provides ready-made presets for all targeted platforms.
- I wanted a single widely used shading language: HLSL
- Designed with interoperability in mind, while C# is the supported language for now, everything is designed to help SWIG generate bindings for us.

## Features

- **Supported Backends**:
    - DirectX12 (Windows)
    - Vulkan (Windows, Linux)
    - Metal (macOS, arm64 only for now)
- **Language Bindings**:
    - Native C++ API
    - C# bindings via SWIG
- **Modern Rendering Features**:
    - HLSL on all backends (DirectX12, Vulkan, Metal)
    - Modern resource binding
    - Compute pipelines
    - Ray tracing support
    - Mesh shaders
- **Asset Management**:
    - Custom asset serialization/deserialization to optimize runtime loading
    - Model loading (via Assimp)
    - Texture loading
    - Animation Support using [ozz-animation](https://github.com/guillaumeblanc/ozz-animation)
- **Font Rendering**:
    - Freetype font rendering [freetype](https://github.com/freetype/freetype)
    - HarfBuzz for text layout [harfbuzz](https://github.com/harfbuzz/harfbuzz)
    - Multi-channel signed distance field (MSDF) support using [msdfgen](https://github.com/Chlumsky/msdfgen)
- **Windowing & Input**:
    - Wrapped SDL2 Input & window management

## Example Projects

DenOfIz includes several example projects demonstrating various aspects of the library, these are very crude for now but do try to sink your teeth in.

Shaders are pretty hard to read as they are ported from all over, I'll probably work on it at some point

- SimpleTriangle: Basic rendering
- RenderTarget: Offscreen rendering
- TextRendering: Font loading and rendering
- RayTracedTriangle: Basic ray tracing([This is a port from DX12Samples](https://github.com/MMuratCengiz/DenOfIz/blob/964db57cbd344deb2c38e1f47455f2fe15b973fd/Examples/RayTracedTriangle/CREDITS.md#L1))
- RayTracedProceduralGeometry: Advanced ray tracing([This is a port from DX12Samples](https://github.com/MMuratCengiz/DenOfIz/blob/964db57cbd344deb2c38e1f47455f2fe15b973fd/Examples/RayTracedProceduralGeometry/CREDITS.md#L1))
- AnimatedFox: Model loading and animation, uses ozz-animation
- MeshShaderGrass: Using mesh shaders, not quite functional
- RootConstants: Demostrates use of push constants, not really a big deal

## Building the Library

Build instructions for all supported platforms: [How to Build](Documentation/HowToBuild.md).

## Getting Started

Quick introduction on how to get up and running: [Getting Started](Documentation/GettingStarted.md).

## Documentation(WIP)

- [Public API Documentation](Documentation/PublicApi.md)
- [Resource Binding Guide](Documentation/ResourceBinding.md)
- [Known Limitations](Documentation/Shortcomings.md)

## Roadmap:

There are some features missing in DenOfIz surely, and while we want to make sure the scope of this project remains to rendering api abstraction there are some things we would like
to yet implement:

#### Near future:

* [ ] Command list query interface
* [ ] Command list debug interface
* [ ] Pipeline caching

#### Soon™:

* [ ] Java bindings
* [ ] WebGPU support, not yet clear how
* [ ] Some fallback for Intel MacBooks, not sure how OpenGL fares with HLSL
* [ ] Mobile Support
* [ ] arm64 support for Windows and Linux

## License

DenOfIz is licensed under the GNU General Public License v3.0. See the [LICENSE](LICENSE) file for details.