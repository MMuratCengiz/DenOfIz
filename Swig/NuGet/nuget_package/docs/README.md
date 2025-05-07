# DenOfIzGraphics

A cross-platform graphics library with support for DirectX12, Vulkan, and Metal backends.

## Features

- Cross-platform rendering support for Windows, macOS, and Linux
- Multiple graphics API backends: DirectX12, Vulkan, and Metal
- Hardware-accelerated font rendering
- Skinned mesh animation system
- Asset management with asset bundles
- Render graph system for optimal performance

## Getting Started

```csharp
using DenOfIzGraphics;
using System;
using System.Numerics;

// Create a graphics API with preferred backend
var graphicsApi = GraphicsApiFactory.CreateDefault();

// Create a logical device
var device = graphicsApi.CreateAndLoadOptimalLogicalDevice();

// Initialize a window for rendering
// ...

// Create a text renderer
var textRenderer = new TextRendering(graphicsApi, device);
textRenderer.LoadFont("path/to/font.ttf");

// Set up orthographic projection
var projectionMatrix = Matrix4x4.CreateOrthographicOffCenter(
    0, windowWidth, windowHeight, 0, 0, 1);
textRenderer.SetProjectionMatrix(projectionMatrix);

// Render text in your render loop
void RenderFrame(DenOfIz.ICommandList commandList)
{
    // Begin text batch
    textRenderer.BeginBatch();
    
    // Add text to the batch
    textRenderer.AddText(
        "Hello, DenOfIz Graphics!", 
        100, 100, 
        new Vector4(1, 1, 1, 1), // White color
        1.0f // Scale
    );
    
    // End batch and render the text
    textRenderer.EndBatch(commandList);
}
```

## Platform Support

This package supports:
- Windows (DirectX12, Vulkan)
- macOS (Metal, Vulkan with MoltenVK)
- Linux (Vulkan)

## Requirements

- .NET Standard 2.0 compatible platform
- Operating system support for at least one of the supported graphics APIs
- NuGet CLI tool for package creation

## Building and Testing the NuGet Package

### Expected Build Directory Structure

The NuGet package scripts expect the following build directory structure:

```
build/
  [Debug_MSVC|Release_MSVC|Debug|Release]/
    CSharp/
      Project/
        Lib/
          win-x64/
            DenOfIzGraphicsCSharp.dll
            D3D12Core.dll
            d3d12SDKLayers.dll
            ...
          win-x86/
            ...
          linux-x64/
            ...
          osx-x64/
            ...
```

Alternatively, the scripts will look for the following paths:
```
build/DenOfIz/Debug_MSVC/CSharp/Project/Lib/win-x64/
build/CSharp/Project/Lib/win-x64/
```

### Cross-Platform Methods (Recommended)
The following scripts will automatically detect your OS and run the appropriate platform-specific script:

1. Build the package:
```
./build_package
```

2. Test the package structure:
```
./test_package
```

### Platform-Specific Methods

#### On Windows
Build options:
1. Run the batch file:
```
.\build_package.bat
```

2. Run the PowerShell script directly:
```
pwsh -ExecutionPolicy Bypass -File .\build_package.ps1
```

Test the package:
```
pwsh -ExecutionPolicy Bypass -File .\test_package.ps1
```

#### On Linux/macOS
Build the package:
```
./build_package.sh
```

Test the package:
```
./test_package.sh
```

The build scripts will create a NuGet package in the `nuget_output` directory. The test scripts will extract the package and verify its structure without installing it.

## License

This project is licensed under the GPL-3.0 License - see the LICENSE file for details.