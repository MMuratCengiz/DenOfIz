# DenOfIz Graphics Library

> **⚠️ ALPHA SOFTWARE**: DenOfIz is currently in alpha stage. The API is subject to change, and the library may(will definitely) contain bugs or missing features. You may or may
> not know why your code just crashed. I still do hope you use and contribute to the library though!

DenOfIz is a cross-platform graphics library over DirectX12, Vulkan and Metal, built around their current rendering features and binding model. No setUniforms or large
state machines.

The API style should be familiar to anyone who used Vulkan or DirectX12, large inputs are passed via input structs, and CommandList recording is very similar in all backends.

## Why:

There are many great libraries that do what I attempt to do here, but I felt like they didn't quite satisfy what I wanted:

- Build system, DenOfIz uses CMake + vcpkg.json, and provides ready-made presets for all targeted platforms.
- I wanted a single widely used shading language: HLSL
- Designed with interoperability in mind, the public API is plain C so bindings are generated instead of hand written

## Features

- **Supported Backends**:
    - DirectX12 (Windows)
    - Vulkan (Windows, Linux)
    - Metal (macOS, arm64 only for now)
- **API & Language Bindings**:
    - C API, `DenOfIz_*` functions on opaque handles, see [Public API](Documentation/PublicApi.md)
    - C# (.NET 8) bindings generated from the C headers by a libclang based generator, shipped as the `DenOfIzGraphics` NuGet package
- **Rendering**:
    - HLSL on all backends, compiled with DXC, Metal through the Metal Shader Converter
    - Resource binding: bind groups per register space, root constants, bindless arrays
    - Compute pipelines, indirect draws and dispatches
    - Ray tracing
    - Mesh shaders
    - Pipeline caches, queries (timestamp, occlusion, pipeline statistics) and debug markers
- **Asset Management**:
    - Custom asset serialization/deserialization to optimize runtime loading (shaders, textures)
    - Texture loading, DDS and PNG/JPG/TGA/HDR/... via [stb_image](https://github.com/nothings/stb)
    - Animation with [ozz-animation](https://github.com/guillaumeblanc/ozz-animation), skeletons and animations imported from glTF with [cgltf](https://github.com/jkuhlmann/cgltf)
    - The examples load models with cgltf and [Assimp](https://github.com/assimp/assimp)
- **Font Rendering**:
    - GPU curve rendering with the [Slug](https://github.com/EricLengyel/Slug) algorithm by Eric Lengyel (algorithm patent dedicated to the public domain, reference shaders MIT OR Apache-2.0), straight from the font files, no atlases
    - [HarfBuzz](https://github.com/harfbuzz/harfbuzz) for text shaping and glyph outlines
    - Bidirectional text and script runs with [SheenBidi](https://github.com/Tehreer/SheenBidi) (Apache-2.0)
    - Line and grapheme breaking with [libunibreak](https://github.com/adah1972/libunibreak) (zlib)
    - Font fallback, including the system fonts of the OS (CoreText, DirectWrite, fontconfig)
    - Built-in default font [JetBrains Mono](https://github.com/JetBrains/JetBrainsMono) (SIL Open Font License 1.1)
    - Color glyphs (COLR v0/v1, sbix, CBDT) painted through HarfBuzz, PNG strikes decoded with [stb_image](https://github.com/nothings/stb); emoji presentation data from [Unicode emoji-data.txt](https://www.unicode.org/Public/17.0.0/ucd/emoji/emoji-data.txt) ([Unicode License v3](https://www.unicode.org/license.txt))
- **UI**:
    - Immediate mode UI with [Yoga](https://github.com/facebook/yoga) flexbox layout, drawn with the text stack above
- **Windowing, Input & Audio**:
    - Wrapped [SDL3](https://github.com/libsdl-org/SDL) window management, input and controllers, audio through [SDL_mixer](https://github.com/libsdl-org/SDL_mixer)
- **Also built on**: [DirectXShaderCompiler](https://github.com/microsoft/DirectXShaderCompiler), [Metal Shader Converter](https://developer.apple.com/metal/shader-converter/), [D3D12MemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/D3D12MemoryAllocator), [VulkanMemoryAllocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator), [volk](https://github.com/zeux/volk), [DirectXMath](https://github.com/microsoft/DirectXMath), [spdlog](https://github.com/gabime/spdlog), [miniz](https://github.com/richgel999/miniz)

## Example Projects

The examples are very crude for now, but do try to sink your teeth in.

Shaders are pretty hard to read as they are ported from all over, I'll probably work on it at some point

- SimpleTriangle: Basic rendering
- RootConstants: Demonstrates use of push constants, not really a big deal
- Bindless: Bindless texture array
- RenderTarget: Offscreen rendering
- Transparency: Opaque and transparent spheres
- CascadedShadowMaps: Cascaded shadow maps
- IndirectRendering: Indirect draws of geometry animated by a compute shader
- RayTracedTriangle: Basic ray tracing([This is a port from DX12Samples](https://github.com/MMuratCengiz/DenOfIz/blob/964db57cbd344deb2c38e1f47455f2fe15b973fd/Examples/RayTracedTriangle/CREDITS.md#L1))
- RayTracedProceduralGeometry: Advanced ray tracing([This is a port from DX12Samples](https://github.com/MMuratCengiz/DenOfIz/blob/964db57cbd344deb2c38e1f47455f2fe15b973fd/Examples/RayTracedProceduralGeometry/CREDITS.md#L1))
- RayTracedScene: Ray traced Bistro scene, [niagara_bistro](https://github.com/zeux/niagara_bistro) is cloned at configure time
- AnimatedFox: Model loading and animation, uses ozz-animation
- TextRendering: Font loading and rendering
- UIExample: The immediate mode UI
- UIBenchmark: UI and text benchmark, can capture frames and compare them against an earlier capture
- ImGui: A [Dear ImGui](https://github.com/ocornut/imgui) backend

## Building the Library

Build instructions for all supported platforms: [How to Build](Documentation/HowToBuild.md).

## Getting Started

Quick introduction on how to get up and running: [Getting Started](Documentation/GettingStarted.md).

## Documentation(WIP)

- [Public API](Documentation/PublicApi.md)
- [Compiling Shaders](Documentation/CompilingShaders.md)
- [Resource Binding](Documentation/ResourceBinding.md)
- [Known Limitations](Documentation/Shortcomings.md)

## Roadmap:

There are some features missing in DenOfIz surely, and while we want to make sure the scope of this project remains to rendering api abstraction there are some things we would like
to yet implement:

#### Soon™:

* [ ] Java bindings
* [ ] Some fallback for Intel MacBooks, not sure how OpenGL fares with HLSL
* [ ] Mobile Support
* [ ] arm64 support for Windows and Linux

## License

DenOfIz is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.
