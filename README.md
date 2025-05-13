# DenOfIz Graphics Library

DenOfIz is a modern, cross-platform graphics library that provides a unified interface over multiple graphics APIs including DirectX12, Vulkan, and Metal. It offers a high-level abstraction layer that makes it easier to develop graphics applications across different platforms while leveraging the performance benefits of modern low-level APIs.

## Features

- **Cross-Platform Support**: Works on Windows, macOS, and Linux
- **Multiple Graphics API Support**:
  - DirectX12 (Windows)
  - Vulkan (Windows, Linux)
  - Metal (macOS)
- **Language Bindings**:
  - Native C++ API
  - C# bindings via SWIG
- **Modern Rendering Features**:
  - **Write HLSL shaders once, run on all backends** (DirectX12, Vulkan, Metal)
  - Ray tracing support
  - Mesh shaders
  - Resource binding model
- **Asset Management**:
  - Model loading (via Assimp)
  - Texture loading
  - Shader reflection and compilation
- **Font Rendering**:
  - Multi-channel signed distance field (MSDF) support
  - Text layout and rendering
- **Animation and Input**:
  - OzzAnimation integration and wrapper
  - SDL2 window and input system wrapper

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

## Getting Started

For instructions on getting started with DenOfIz, including prerequisites and building the library, see our [Getting Started](Documentation/GettingStarted.md) guide.

## Building the Library

For detailed build instructions for all supported platforms, see our [How to Build](Documentation/HowToBuild.md) document.

## Documentation

- [Public API Documentation](Documentation/PublicApi.md)
- [Resource Binding Guide](Documentation/ResourceBinding.md)
- [Known Limitations](Documentation/Shortcomings.md)

## License

DenOfIz is licensed under the GNU General Public License v3.0. See the [LICENSE](LICENSE) file for details.