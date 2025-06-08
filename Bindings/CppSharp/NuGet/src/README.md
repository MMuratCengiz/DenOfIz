# DenOfIz Graphics Library for .NET

> **⚠️ ALPHA SOFTWARE**: DenOfIz is currently in alpha stage. The API is subject to change, and the library may contain bugs or missing features. Use in production environments is not recommended at this time.

DenOfIz is a modern, cross-platform graphics library that provides a unified interface over multiple graphics APIs including DirectX12, Vulkan, and Metal. It offers a high-level abstraction layer that makes it easier to develop graphics applications across different platforms while leveraging the performance benefits of modern low-level APIs.

## Features

- **Cross-Platform Support**: Works on Windows, macOS, and Linux
- **Multiple Graphics API Support**:
  - DirectX12 (Windows)
  - Vulkan (Windows, Linux)
  - Metal (macOS)
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

## Getting Started

To get started with DenOfIz in your .NET project:

1. Install the NuGet package:
   ```
   dotnet add package DenOfIzGraphics
   ```

2. Make sure to initialize the library at the start of your application:
   ```csharp
   // Very important to ensure necessary libraries are loaded (especially dxil and dxcompiler for Windows)
   DenOfIzGraphics.Initialize(new EngineDesc());
   ```

3. Create a window and set up the graphics API:
   ```csharp
   // Create window
   var windowProps = new WindowProperties();
   windowProps.Title = new InteropString("DenOfIz Example");
   windowProps.Width = 1920;
   windowProps.Height = 1080;
   windowProps.Flags.Shown = true;
   windowProps.Flags.Resizable = true;
   windowProps.Position = WindowPosition.Centered;
   
   var window = new Window(windowProps);
   var windowHandle = window.GetGraphicsWindowHandle();
   
   // Initialize graphics API
   var apiPreference = new APIPreference();
   apiPreference.Windows = APIPreferenceWindows.DirectX12;
   apiPreference.Linux = APIPreferenceLinux.Vulkan;
   apiPreference.OSX = APIPreferenceOSX.Metal;
   
   var graphicsApi = new GraphicsApi(apiPreference);
   var device = graphicsApi.CreateAndLoadOptimalLogicalDevice();
   ```

4. Check the included `Example.cs` file for a complete triangle rendering example that demonstrates the fundamentals of the API.

## Platform Support

- **Windows**: x64 architecture with DirectX12 or Vulkan
- **macOS**: arm64 architecture with Metal
- **Linux**: x64 architecture with Vulkan

## Dependencies

This library includes all necessary native dependencies in the NuGet package. The native libraries are automatically loaded when you initialize the library.

## API Overview

- **GraphicsApi**: The main entry point to the library
- **ILogicalDevice**: Represents a logical device for rendering
- **ICommandQueue**: Handles command submission to the GPU
- **ISwapChain**: Manages presentation of rendered frames
- **IPipeline**: Represents a graphics or compute pipeline
- **FrameSync**: Synchronizes rendering across multiple frames

## Additional Resources

For more detailed information and advanced usage, refer to the official GitHub repository: [https://github.com/DenOfIz/DenOfIz](https://github.com/DenOfIz/DenOfIz)

## License

DenOfIz is licensed under the GNU General Public License v3.0.