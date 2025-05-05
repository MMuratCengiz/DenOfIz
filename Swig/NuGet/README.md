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

## License

This project is licensed under the GPL-3.0 License - see the LICENSE file for details.