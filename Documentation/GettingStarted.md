# Getting Started with DenOfIz

DenOfIz is used through its C API in `Graphics/Include/DenOfIzGraphics`, include the headers you need or `DenOfIzGraphics/DenOfIzGraphics_Ext.h` for all of them. See
[How to Build](HowToBuild.md) to build it and [Public API](PublicApi.md) for the conventions. From .NET, add the `DenOfIzGraphics` NuGet package,
`Bindings/CSharp/DenOfIzGraphicsExample` draws a triangle in C#.

## Project Structure

- **Graphics**: the library, public headers in `Graphics/Include`, tests in `Graphics/Tests`, tools in `Graphics/Tools`
- **Bindings**: the C# binding generator, the generated sources and hand written wrappers
- **Examples**: examples featuring common use cases, `Examples/Base` holds the shared framework (`IExample.h`, `Main.cpp`)
- **Documentation**: this documentation

## Setup

The same setup the examples use, see `Examples/Base/Source/Main.cpp` and `Examples/Base/Include/DenOfIzExamples/IExample.h`:

```cpp
DenOfIz_EngineDesc engineDesc{ };
engineDesc.LogLevel = DENOFIZ_LOG_LEVEL_INFO;
DenOfIz_Engine_Init( &engineDesc );

DenOfIz_WindowDesc windowDesc{ };
windowDesc.Title           = DENOFIZ_STRING( "DenOfIz" );
windowDesc.Width           = 1280;
windowDesc.Height          = 720;
windowDesc.Position        = DENOFIZ_WINDOW_POSITION_CENTERED;
windowDesc.Flags.Resizable = true;
DenOfIz_Window window      = DenOfIz_Window_Create( &windowDesc );

DenOfIz_APIPreference apiPreference{ };
apiPreference.Windows           = DENOFIZ_API_PREFERENCE_WINDOWS_DIRECTX12;
apiPreference.Linux             = DENOFIZ_API_PREFERENCE_LINUX_VULKAN;
apiPreference.OSX               = DENOFIZ_API_PREFERENCE_OSX_METAL;
DenOfIz_GraphicsApi graphicsApi = DenOfIz_GraphicsApi_Create( &apiPreference );

DenOfIz_LogicalDeviceDesc deviceDesc{ };
deviceDesc.EnableValidationLayers = true;
DenOfIz_LogicalDevice device      = DenOfIz_GraphicsApi_CreateAndLoadOptimalLogicalDevice( graphicsApi, &deviceDesc );

DenOfIz_CommandQueueDesc queueDesc{ };
queueDesc.QueueType                        = DENOFIZ_QUEUE_TYPE_GRAPHICS;
queueDesc.Flags.RequirePresentationSupport = true;
DenOfIz_CommandQueue graphicsQueue;
DenOfIz_LogicalDevice_CreateCommandQueue( device, &queueDesc, &graphicsQueue );

DenOfIz_SwapChainDesc swapChainDesc{ };
swapChainDesc.WindowHandle     = DenOfIz_Window_GetGraphicsWindowHandle( window );
swapChainDesc.NumBuffers       = 3;
swapChainDesc.BackBufferFormat = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
swapChainDesc.CommandQueue     = graphicsQueue;
DenOfIz_SwapChain swapChain;
DenOfIz_LogicalDevice_CreateSwapChain( device, &swapChainDesc, &swapChain );

DenOfIz_ResourceTracking resourceTracking;
DenOfIz_ResourceTracking_Create( &resourceTracking );

DenOfIz_FrameSyncDesc frameSyncDesc{ };
frameSyncDesc.Device           = device;
frameSyncDesc.SwapChain        = swapChain;
frameSyncDesc.CommandQueue     = graphicsQueue;
frameSyncDesc.NumFrames        = 3;
frameSyncDesc.ResourceTracking = resourceTracking; // BeginFrame then tracks the back buffer
DenOfIz_FrameSync frameSync;
DenOfIz_FrameSync_Create( &frameSyncDesc, &frameSync );
```

## Pipeline and Vertex Buffer

`Triangle.hlsl`, one file with both entry points:

```hlsl
struct VSInput  { float3 Position : POSITION; float4 Color : COLOR; };
struct PSInput  { float4 Position : SV_POSITION; float4 Color : COLOR; };

PSInput VSMain( VSInput input )
{
    PSInput output;
    output.Position = float4( input.Position, 1.0 );
    output.Color    = input.Color;
    return output;
}

float4 PSMain( PSInput input ) : SV_TARGET { return input.Color; }
```

The shader program compiles the stages for every backend and its reflection gives the input layout and the bind group layouts of the root signature. Shader code can also be
passed in memory through `DenOfIz_ShaderStageDesc::Data`, see [Compiling Shaders](CompilingShaders.md).

```cpp
DenOfIz_ShaderStageDesc stages[ 2 ]{ };
stages[ 0 ].Stage      = DENOFIZ_SHADER_STAGE_VERTEX_BIT;
stages[ 0 ].Path       = DENOFIZ_STRING( "Triangle.hlsl" );
stages[ 0 ].EntryPoint = DENOFIZ_STRING( "VSMain" );
stages[ 1 ].Stage      = DENOFIZ_SHADER_STAGE_PIXEL_BIT;
stages[ 1 ].Path       = DENOFIZ_STRING( "Triangle.hlsl" );
stages[ 1 ].EntryPoint = DENOFIZ_STRING( "PSMain" );

DenOfIz_ShaderProgramDesc programDesc{ };
programDesc.ShaderStages.Elements    = stages;
programDesc.ShaderStages.NumElements = 2;
DenOfIz_ShaderProgram program        = DenOfIz_ShaderProgram_Create( &programDesc );

DenOfIz_ShaderReflectDesc reflectDesc{ };
DenOfIz_ShaderProgram_Reflect( program, &reflectDesc );

// No resources in this shader, so no bind group layouts, see ResourceBinding.md for shaders that have some.
DenOfIz_RootSignatureDesc rootSignatureDesc{ };
rootSignatureDesc.RootConstants = reflectDesc.RootConstants;
DenOfIz_RootSignature rootSignature;
DenOfIz_LogicalDevice_CreateRootSignature( device, &rootSignatureDesc, &rootSignature );

DenOfIz_InputLayout inputLayout;
DenOfIz_LogicalDevice_CreateInputLayout( device, &reflectDesc.InputLayout, &inputLayout );

DenOfIz_RenderTargetDesc renderTarget{ };
renderTarget.Format                      = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
renderTarget.Blend.RenderTargetWriteMask = 0x0F;

DenOfIz_PipelineDesc pipelineDesc{ };
pipelineDesc.BindPoint                          = DENOFIZ_BIND_POINT_GRAPHICS;
pipelineDesc.ShaderProgram                      = program;
pipelineDesc.RootSignature                      = rootSignature;
pipelineDesc.InputLayout                        = inputLayout;
pipelineDesc.Graphics.PrimitiveTopology         = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;
pipelineDesc.Graphics.RenderTargets.Elements    = &renderTarget;
pipelineDesc.Graphics.RenderTargets.NumElements = 1;
DenOfIz_Pipeline pipeline;
DenOfIz_LogicalDevice_CreatePipeline( device, &pipelineDesc, &pipeline );
```

The vertex buffer lives on the GPU heap and is filled with a `DenOfIz_BatchResourceCopy`:

```cpp
const float vertices[] = {
    //  X      Y     Z     R     G     B     A
     0.0f,  0.5f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
    -0.5f, -0.5f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
     0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f,
};

DenOfIz_BufferDesc bufferDesc{ };
bufferDesc.Usage     = DENOFIZ_BUFFER_USAGE_VERTEX_BIT | DENOFIZ_BUFFER_USAGE_COPY_DST_BIT;
bufferDesc.NumBytes  = sizeof( vertices );
bufferDesc.DebugName = DENOFIZ_STRING( "TriangleVertexBuffer" );
DenOfIz_Buffer vertexBuffer;
DenOfIz_LogicalDevice_CreateBuffer( device, &bufferDesc, &vertexBuffer );

DenOfIz_BatchResourceCopyDesc copyBatchDesc{ };
copyBatchDesc.Device        = device;
copyBatchDesc.IssueBarriers = true;
DenOfIz_BatchResourceCopy copyBatch;
DenOfIz_BatchResourceCopy_Create( &copyBatchDesc, &copyBatch );
DenOfIz_BatchResourceCopy_Begin( copyBatch );

DenOfIz_CopyToGpuBufferDesc copyDesc{ };
copyDesc.DstBuffer        = vertexBuffer;
copyDesc.Data.Elements    = reinterpret_cast<const Byte *>( vertices );
copyDesc.Data.NumElements = sizeof( vertices );
DenOfIz_BatchResourceCopy_CopyToGPUBuffer( copyBatch, &copyDesc );
DenOfIz_BatchResourceCopy_Submit( copyBatch, DENOFIZ_NULL_HANDLE );
DenOfIz_BatchResourceCopy_Destroy( copyBatch );

DenOfIz_ResourceTracking_TrackBuffer( resourceTracking, vertexBuffer, DENOFIZ_QUEUE_TYPE_GRAPHICS );
```

## Frame Loop

`DenOfIz_FrameSync_BeginFrame` waits for the frame's previous work, acquires the back buffer and hands out the frame's command list, `DenOfIz_FrameSync_EndFrame` submits it and
presents:

```cpp
bool running = true;
while ( running )
{
    DenOfIz_Event event;
    while ( DenOfIz_InputSystem_PollEvent( &event ) )
    {
        if ( event.Type == DENOFIZ_EVENT_TYPE_QUIT )
        {
            running = false;
        }
    }

    DenOfIz_Frame frame{ };
    if ( DenOfIz_FrameSync_BeginFrame( frameSync, &frame ) != DENOFIZ_SUCCESS )
    {
        continue; // Minimized, occluded or device lost: skip the frame, don't call EndFrame.
    }
    // frame.Resized: the GPU is idle, recreate anything sized after the swap chain.

    DenOfIz_CommandList commandList = frame.CommandList;
    DenOfIz_CommandList_Begin( commandList );
    DenOfIz_ResourceTracking_TransitionTexture( resourceTracking, commandList, frame.BackBuffer, DENOFIZ_RESOURCE_USAGE_RENDER_TARGET_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );

    DenOfIz_RenderingAttachmentDesc attachment{ };
    attachment.Resource = frame.BackBuffer;
    DenOfIz_RenderingDesc renderingDesc{ };
    renderingDesc.RTAttachments.Elements    = &attachment;
    renderingDesc.RTAttachments.NumElements = 1;
    DenOfIz_CommandList_BeginRendering( commandList, &renderingDesc );

    DenOfIz_CommandList_BindViewport( commandList, 0, 0, frame.Width, frame.Height );
    DenOfIz_CommandList_BindScissorRect( commandList, 0, 0, frame.Width, frame.Height );
    DenOfIz_CommandList_BindPipeline( commandList, pipeline );
    DenOfIz_CommandList_BindVertexBuffer( commandList, vertexBuffer, 0, 7 * sizeof( float ), 0 );
    DenOfIz_CommandList_Draw( commandList, 3, 1, 0, 0 );
    DenOfIz_CommandList_EndRendering( commandList );

    DenOfIz_ResourceTracking_TransitionTexture( resourceTracking, commandList, frame.BackBuffer, DENOFIZ_RESOURCE_USAGE_PRESENT_BIT, DENOFIZ_QUEUE_TYPE_GRAPHICS );
    DenOfIz_CommandList_End( commandList );
    DenOfIz_FrameSync_EndFrame( frameSync, nullptr );
}
```

On shutdown call `DenOfIz_FrameSync_WaitIdle`, then the `Destroy` function of every object you created, `DenOfIz_LogicalDevice_Destroy`, `DenOfIz_GraphicsApi_Destroy`,
`DenOfIz_Window_Destroy` and `DenOfIz_Engine_Shutdown`.

## Example Projects

The best way to learn is to explore the example projects:

1. `SimpleTriangle` is the code above in the example framework
2. `RootConstants` and `Bindless` are small examples of resource binding, `Transparency` a bigger one
3. `Examples/Base` holds the framework code: `IExample.h`, `Main.cpp`
4. `RayTracedTriangle` is a good ray tracing starting point

## Next Steps

- [Compiling Shaders](CompilingShaders.md) on how to compile shaders
- [Resource Binding](ResourceBinding.md) for understanding the resource binding model
