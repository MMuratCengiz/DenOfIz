# DenOfIz Graphics Library

> **⚠️ ALPHA SOFTWARE**: DenOfIz is currently in alpha stage. The API is subject to change, and the library may contain bugs or missing features. Use in production environments is not recommended at this time.

DenOfIz is a modern, cross-platform graphics library that provides a unified interface over multiple graphics APIs including DirectX12, Vulkan, and Metal. It focuses on modern rendering features and binding methods. 

## Features

- **Cross-Platform Support**: Works on Windows, macOS(Requires metal support currently), and Linux
- **Supported Backends**:
  - DirectX12 (Windows)
  - Vulkan (Windows, Linux)
  - Metal (macOS)
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

DenOfIz includes several example projects demonstrating various aspects of the library:

- SimpleTriangle: Basic rendering
- RenderTarget: Offscreen rendering
- TextRendering: Font loading and rendering
- RayTracedTriangle: Basic ray tracing
- RayTracedProceduralGeometry: Advanced ray tracing
- AnimatedFox: Model loading and animation
- MeshShaderGrass: Using mesh shaders
- RootConstants: Performance optimization techniques

## Building the Library

Build instructions for all supported platforms: [How to Build](Documentation/HowToBuild.md).

## Getting Started

Quick introduction on how to get up and running: [Getting Started](Documentation/GettingStarted.md).

## Documentation(WIP)

- [Public API Documentation](Documentation/PublicApi.md)
- [Resource Binding Guide](Documentation/ResourceBinding.md)
- [Known Limitations](Documentation/Shortcomings.md)

## License

DenOfIz is licensed under the GNU General Public License v3.0. See the [LICENSE](LICENSE) file for details.