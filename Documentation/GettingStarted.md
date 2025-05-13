# Getting Started with DenOfIz

DenOfIz is a cross-platform graphics library that provides abstractions over multiple graphics APIs (DirectX12, Vulkan, Metal) with a unified interface. This document will help you get started with using DenOfIz in your projects.


## Project Structure

- **Graphics**: Code directory
- **Examples**: Examples featuring common use cases
- **Documentation**: Detailed documentation 
- **Swig**: Language bindings (C#, Java) for using DenOfIz from other languages

## Basic Rendering Flow

Creating a renderer with DenOfIz follows a standard pattern. Here's a step-by-step guide with code examples:

### 1. Create Window and Graphics Context

Start by setting up a window and initializing the graphics API:

```cpp
// Create window
WindowDesc windowDesc;
windowDesc.Title = "DenOfIz Sample";
windowDesc.Width = 1280;
windowDesc.Height = 720;
windowDesc.Flags.Shown = true;
windowDesc.Flags.Resizable = true;
windowDesc.Position = WindowPosition::Centered;

Window window(windowDesc);
GraphicsWindowHandle windowHandle = window.GetGraphicsWindowHandle();

// Initialize graphics API
APIPreference apiPreference;
apiPreference.Windows = APIPreferenceWindows::DirectX12;
apiPreference.Linux = APIPreferenceLinux::Vulkan;
apiPreference.OSX = APIPreferenceOSX::Metal;

GraphicsApi graphicsApi(apiPreference);
ILogicalDevice* device = graphicsApi.CreateAndLoadOptimalLogicalDevice();
```

### 2. Create Command Queue and SwapChain

```cpp
// Create command queue
CommandQueueDesc queueDesc;
queueDesc.QueueType = QueueType::Graphics;
queueDesc.Flags.RequirePresentationSupport = true;
ICommandQueue* graphicsQueue = device->CreateCommandQueue(queueDesc);

// Create swap chain
SwapChainDesc swapChainDesc;
swapChainDesc.Width = windowDesc.Width;
swapChainDesc.Height = windowDesc.Height;
swapChainDesc.WindowHandle = windowHandle;
swapChainDesc.CommandQueue = graphicsQueue;
swapChainDesc.NumBuffers = 3;
ISwapChain* swapChain = device->CreateSwapChain(swapChainDesc);
```

### 3. Compile Shaders and Create Input Layout

```cpp
// Define shader code
const char* vertexShaderCode = R"(
    struct VSInput
    {
        float3 Position : POSITION;
        float4 Color : COLOR;
    };

    struct PSInput
    {
        float4 Position : SV_POSITION;
        float4 Color : COLOR;
    };

    PSInput VSMain(VSInput input)
    {
        PSInput output;
        output.Position = float4(input.Position, 1.0);
        output.Color = input.Color;
        return output;
    }
)";

const char* pixelShaderCode = R"(
    struct PSInput
    {
        float4 Position : SV_POSITION;
        float4 Color : COLOR;
    };

    float4 PSMain(PSInput input) : SV_TARGET
    {
        return input.Color;
    }
)";

// Create shader program descriptor
ShaderProgramDesc shaderProgramDesc;
ShaderStageDesc& vertexShaderDesc = shaderProgramDesc.ShaderStages.EmplaceElement();
vertexShaderDesc.Stage = ShaderStage::Vertex;
vertexShaderDesc.EntryPoint = "VSMain";
vertexShaderDesc.Data = InteropUtilities::StringToBytes(InteropString(vertexShaderCode));

ShaderStageDesc& pixelShaderDesc = shaderProgramDesc.ShaderStages.EmplaceElement();
pixelShaderDesc.Stage = ShaderStage::Pixel;
pixelShaderDesc.EntryPoint = "PSMain";
pixelShaderDesc.Data = InteropUtilities::StringToBytes(InteropString(pixelShaderCode));

// Create shader program
ShaderProgram* program = new ShaderProgram(shaderProgramDesc);
```

### 4. Create Root Signature and Pipeline

```cpp
// Get shader reflection data and create input layout and root signature
const ShaderReflectDesc reflectDesc = program->Reflect();
IInputLayout* inputLayout = device->CreateInputLayout(reflectDesc.InputLayout);
IRootSignature* rootSignature = device->CreateRootSignature(reflectDesc.RootSignature);

// Create pipeline
PipelineDesc pipelineDesc;
pipelineDesc.InputLayout = inputLayout;
pipelineDesc.ShaderProgram = program;
pipelineDesc.RootSignature = rootSignature;
pipelineDesc.Graphics.RenderTargets.AddElement({ .Format = Format::B8G8R8A8Unorm });

IPipeline* pipeline = device->CreatePipeline(pipelineDesc);
```

### 5. Create Resources (Vertex Buffer)

```cpp
// Define vertex data
constexpr std::array vertices = {
    // Position (XYZ)    // Color (RGBA)
    0.0f,  0.5f,  0.0f, 1.0f, 0.0f, 0.0f, 1.0f, // Top vertex (red)
    -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, // Bottom left (green)
    0.5f,  -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f  // Bottom right (blue)
};

// Create vertex buffer
BufferDesc bufferDesc;
bufferDesc.Descriptor.Set(ResourceDescriptor::VertexBuffer);
bufferDesc.NumBytes = vertices.size() * sizeof(float);
bufferDesc.DebugName = "TriangleVertexBuffer";

IBufferResource* vertexBuffer = device->CreateBufferResource(bufferDesc);

// Upload data to GPU
BatchResourceCopy batchCopy(device);
batchCopy.Begin();

CopyToGpuBufferDesc copyDesc;
copyDesc.DstBuffer = vertexBuffer;
copyDesc.Data.MemCpy(vertices.data(), vertices.size() * sizeof(float));
batchCopy.CopyToGPUBuffer(copyDesc);
batchCopy.Submit();

// Track resource usage
ResourceTracking resourceTracking;
resourceTracking.TrackBuffer(vertexBuffer, ResourceUsage::VertexAndConstantBuffer);
```

### 6. Setup Command List and Frame Synchronization

```cpp
// Create frame synchronization
FrameSyncDesc frameSyncDesc;
frameSyncDesc.SwapChain = swapChain;
frameSyncDesc.Device = device;
frameSyncDesc.NumFrames = 3;
frameSyncDesc.CommandQueue = graphicsQueue;
FrameSync* frameSync = new FrameSync(frameSyncDesc);

// Track swap chain render targets
for (uint32_t i = 0; i < swapChainDesc.NumBuffers; i++) {
    resourceTracking.TrackTexture(swapChain->GetRenderTarget(i), ResourceUsage::Common);
}
```

### 7. Record Command List and Render

```cpp
// Begin render loop
while (running) {
    // Process window events...

    // Get frame index and command list
    uint64_t frameIndex = frameSync->NextFrame();
    ICommandList* commandList = frameSync->GetCommandList(frameIndex);

    // Begin command list
    commandList->Begin();

    // Get render target
    ITextureResource* renderTarget = swapChain->GetRenderTarget(frameSync->AcquireNextImage(frameIndex));

    // Resource transition
    BatchTransitionDesc batchTransitionDesc(commandList);
    batchTransitionDesc.TransitionTexture(renderTarget, ResourceUsage::RenderTarget);
    resourceTracking.BatchTransition(batchTransitionDesc);

    // Begin rendering
    RenderingDesc renderingDesc;
    renderingDesc.RTAttachments.AddElement({ .Resource = renderTarget });
    commandList->BeginRendering(renderingDesc);

    // Set viewport and scissor rect
    Viewport viewport = swapChain->GetViewport();
    commandList->BindViewport(viewport.X, viewport.Y, viewport.Width, viewport.Height);
    commandList->BindScissorRect(viewport.X, viewport.Y, viewport.Width, viewport.Height);

    // Bind pipeline and resources
    commandList->BindPipeline(pipeline);
    commandList->BindVertexBuffer(vertexBuffer);

    // Draw
    commandList->Draw(3, 1, 0, 0);

    // End rendering
    commandList->EndRendering();

    // Transition to present
    batchTransitionDesc = BatchTransitionDesc(commandList);
    batchTransitionDesc.TransitionTexture(renderTarget, ResourceUsage::Present);
    resourceTracking.BatchTransition(batchTransitionDesc);

    // End command list
    commandList->End();

    // Execute and present
    frameSync->ExecuteCommandList(frameIndex);
    frameSync->Present(frameSync->AcquireNextImage(frameIndex));
}
```

## Example Projects

The best way to learn is to explore the example projects:

1. `SimpleTriangle` is the simplest starting point, no resource binding
2. `Transparency` is not very straightforward but could provide good guidance on how to bind resources.
3. Refer to `IExample.h` & `Main.cpp` for the framework code
4. `RayTracedTriangle` Is a good raytracing starting point

## Next Steps

Once you're comfortable with the basics, refer to the following resources:

- [Compiling Shaders](CompilingShaders.md) on how to compile shaders
- [Resource Binding Guide](ResourceBinding.md) for understanding the resource binding model