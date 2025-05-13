# Resource Binding in DenOfIz

Resource binding is the process of making data accessible to GPU shaders, allowing shaders to access textures, buffers, and other data. In DenOfIz, the resource binding system provides a unified abstraction over different graphics APIs (DirectX 12, Vulkan, and Metal) with their varying binding models.

## Binding Resources Overview

The resource binding process in DenOfIz involves these key components:

1. **ShaderProgram**: Contains shader code and reflection data
2. **RootSignature**: Defines the layout of resources accessible to shaders
3. **ResourceBindGroup**: Actual container for binding resources to shader registers

## Creating a Shader Program

First, create a ShaderProgram by specifying shader stage descriptors:

```cpp
// Create shader stages
InteropArray<ShaderStageDesc> shaderStages;

// Add vertex shader
ShaderStageDesc &vertexShaderDesc = shaderStages.EmplaceElement();
vertexShaderDesc.Path = "Assets/Shaders/FullscreenQuad.vs.hlsl";
vertexShaderDesc.Stage = ShaderStage::Vertex;

// Add pixel shader
ShaderStageDesc &pixelShaderDesc = shaderStages.EmplaceElement();
pixelShaderDesc.Path = "Assets/Shaders/SampleBasic.ps.hlsl";
pixelShaderDesc.Stage = ShaderStage::Pixel;

// Create shader program
std::unique_ptr<ShaderProgram> program = std::make_unique<ShaderProgram>(ShaderProgramDesc{ .ShaderStages = shaderStages });
```

## Shader Reflection and Creating Root Signature

After creating the shader program, use shader reflection to create a RootSignature and InputLayout:

```cpp
// Get reflection data from program
ShaderReflectDesc programReflection = program->Reflect();

// Create root signature and input layout
std::unique_ptr<IRootSignature> rootSignature = std::unique_ptr<IRootSignature>(device->CreateRootSignature(programReflection.RootSignature));
std::unique_ptr<IInputLayout> inputLayout = std::unique_ptr<IInputLayout>(device->CreateInputLayout(programReflection.InputLayout));
```

## Resource Binding

Resources are bound to the ResourceBindGroup according to how they are defined in HLSL shaders. The current binding API uses a begin/end update pattern:

```cpp
// Create resource bind group
ResourceBindGroupDesc bindGroupDesc;
bindGroupDesc.RegisterSpace = 0; // Specify which register space to use
bindGroupDesc.RootSignature = rootSignature.get();
std::unique_ptr<IResourceBindGroup> resourceBindGroup = std::unique_ptr<IResourceBindGroup>(device->CreateResourceBindGroup(bindGroupDesc));

// Begin resource binding update
resourceBindGroup->BeginUpdate();

// Bind resources using their register indices
resourceBindGroup->Cbv(0, constantBuffer.get());  // Constant buffer view at register b0
resourceBindGroup->Srv(0, texture.get());         // Shader resource view at register t0
resourceBindGroup->Sampler(0, sampler.get());     // Sampler at register s0

// End resource binding update
resourceBindGroup->EndUpdate();
```

## Using Bound Resources

Once resources are bound to the resource bind group, bind it to the command list before drawing:

```cpp
// Bind the resource group to the command list
commandList->BindResourceGroup(resourceBindGroup.get());

// Draw
commandList->Draw(3, 1, 0, 0);
```

## Register Spaces

DenOfIz uses register spaces to organize resources with different visibility and binding performance:

1. **Default Space (0)**: Used for standard descriptor table resources
2. **Root Level Buffers (30)**: For high-performance direct root level descriptors (DirectX 12 feature)
3. **Push Constants (31)**: For small, frequently changed data

When creating a ResourceBindGroup, you specify which register space it targets:

```cpp
ResourceBindGroupDesc bindGroupDesc;
bindGroupDesc.RegisterSpace = 0;  // Target register space 0
bindGroupDesc.RootSignature = rootSignature.get();
```

## Root Constants (Push Constants)

Root constants provide a fast way to update small amounts of constant data without creating buffer resources. Root constants are directly pushed to the GPU using the `SetRootConstants` method:

```cpp
// Define push constant data in HLSL
// [[vk::push_constant]] ConstantBuffer<PushConstants> pushConstants : register(b0, space31);

// Create a resource bind group for root constants
ResourceBindGroupDesc bindGroupDesc = RootConstantBindGroupDesc(rootSignature.get());
std::unique_ptr<IResourceBindGroup> rootConstantBindGroup = std::unique_ptr<IResourceBindGroup>(device->CreateResourceBindGroup(bindGroupDesc));

// Set root constants
PushConstants data;
data.color = float4(1.0f, 0.0f, 0.0f, 1.0f);
rootConstantBindGroup->SetRootConstants(0, &data);
```

The register space for push constants is fixed at 31, which is recognized by all backends. You can use the `RootConstantBindGroupDesc` helper to create the appropriate bind group description.

### HLSL Push Constant Definition

In HLSL, push constants must be declared with the `[[vk::push_constant]]` attribute and use register space 31:

```hlsl
struct PushConstants {
    float4 color;
};

[[vk::push_constant]] ConstantBuffer<PushConstants> pushConstants : register(b0, space31);
```

**Important**: For Vulkan compatibility, only a single push constant block is allowed.

## Binding Resources with Offsets

For more complex scenarios, such as binding parts of a buffer or a buffer array, you can use the `BindBufferDesc` struct:

```cpp
// Create a buffer with multiple instances of data
BufferDesc bufferDesc;
bufferDesc.NumBytes = alignedNumBytes * numInstances;
bufferDesc.Descriptor = BitSet(ResourceDescriptor::UniformBuffer);
bufferDesc.HeapType = HeapType::CPU_GPU;
std::unique_ptr<IBufferResource> buffer = std::unique_ptr<IBufferResource>(device->CreateBufferResource(bufferDesc));

// Bind a specific instance of data
BindBufferDesc bindBufferDesc;
bindBufferDesc.Binding = 0;
bindBufferDesc.Resource = buffer.get();
bindBufferDesc.ResourceOffset = alignedNumBytes * instanceIndex;

resourceBindGroup->BeginUpdate();
resourceBindGroup->Cbv(bindBufferDesc);
resourceBindGroup->EndUpdate();
```

## API Specific Considerations

### DirectX 12

- Uses descriptor tables organized by register spaces
- Supports optimized root-level descriptors for faster access

### Vulkan 

- Uses descriptor sets which correspond to register spaces
- Push constants are bound using the `[[vk::push_constant]]` attribute
- Register bindings are shifted to avoid conflicts (DXC Shift feature, see `ShaderCompiler.cpp`)

### Metal

- Uses argument buffers with explicit binding order
- Metal requires reflection data to properly bind resources
- Always use shader reflection with Metal to ensure correct bindings

## Complete Example: Resource Binding

Here's a complete example of creating and binding resources:

```cpp
// Create shader program
InteropArray<ShaderStageDesc> shaderStages;
ShaderStageDesc &vertexShaderDesc = shaderStages.EmplaceElement();
vertexShaderDesc.Path = "Assets/Shaders/FullscreenQuad.vs.hlsl";
vertexShaderDesc.Stage = ShaderStage::Vertex;

ShaderStageDesc &pixelShaderDesc = shaderStages.EmplaceElement();
pixelShaderDesc.Path = "Assets/Shaders/SampleTexture.ps.hlsl";
pixelShaderDesc.Stage = ShaderStage::Pixel;

std::unique_ptr<ShaderProgram> program = std::make_unique<ShaderProgram>(ShaderProgramDesc{ .ShaderStages = shaderStages });

// Get reflection data and create root signature
ShaderReflectDesc reflectDesc = program->Reflect();
std::unique_ptr<IRootSignature> rootSignature = std::unique_ptr<IRootSignature>(device->CreateRootSignature(reflectDesc.RootSignature));

// Create constant buffer
BufferDesc constantBufferDesc;
constantBufferDesc.NumBytes = sizeof(Constants);
constantBufferDesc.Descriptor = BitSet(ResourceDescriptor::UniformBuffer);
constantBufferDesc.HeapType = HeapType::CPU_GPU;
std::unique_ptr<IBufferResource> constantBuffer = std::unique_ptr<IBufferResource>(device->CreateBufferResource(constantBufferDesc));

// Map and update constant buffer data
Constants* mappedData = static_cast<Constants*>(constantBuffer->MapMemory());
mappedData->color = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
constantBuffer->UnmapMemory();

// Create texture
TextureDesc textureDesc;
textureDesc.Width = 1024;
textureDesc.Height = 1024;
textureDesc.Format = Format::R8G8B8A8Unorm;
std::unique_ptr<ITextureResource> texture = std::unique_ptr<ITextureResource>(device->CreateTextureResource(textureDesc));

// Create sampler
SamplerDesc samplerDesc;
samplerDesc.Filter = Filter::Linear;
samplerDesc.AddressU = TextureAddressMode::Wrap;
samplerDesc.AddressV = TextureAddressMode::Wrap;
samplerDesc.AddressW = TextureAddressMode::Wrap;
std::unique_ptr<ISampler> sampler = std::unique_ptr<ISampler>(device->CreateSampler(samplerDesc));

// Create resource bind group and bind resources
ResourceBindGroupDesc bindGroupDesc;
bindGroupDesc.RegisterSpace = 0;
bindGroupDesc.RootSignature = rootSignature.get();
std::unique_ptr<IResourceBindGroup> resourceBindGroup = std::unique_ptr<IResourceBindGroup>(device->CreateResourceBindGroup(bindGroupDesc));

resourceBindGroup->BeginUpdate();
resourceBindGroup->Cbv(0, constantBuffer.get());
resourceBindGroup->Srv(0, texture.get());
resourceBindGroup->Sampler(0, sampler.get());
resourceBindGroup->EndUpdate();

// In the render loop
commandList->BindResourceGroup(resourceBindGroup.get());
commandList->Draw(3, 1, 0, 0);
```

## Working with Multiple Register Spaces

When your shader uses multiple register spaces, create a separate ResourceBindGroup for each space:

```cpp
// Create bind group for view/projection matrices (space 0)
ResourceBindGroupDesc viewProjBindGroupDesc;
viewProjBindGroupDesc.RegisterSpace = 0;
viewProjBindGroupDesc.RootSignature = rootSignature.get();
std::unique_ptr<IResourceBindGroup> viewProjBindGroup = std::unique_ptr<IResourceBindGroup>(device->CreateResourceBindGroup(viewProjBindGroupDesc));

viewProjBindGroup->BeginUpdate();
viewProjBindGroup->Cbv(0, viewProjBuffer.get());
viewProjBindGroup->EndUpdate();

// Create bind group for model matrices (space 30)
ResourceBindGroupDesc modelBindGroupDesc;
modelBindGroupDesc.RegisterSpace = 30;
modelBindGroupDesc.RootSignature = rootSignature.get();
std::unique_ptr<IResourceBindGroup> modelBindGroup = std::unique_ptr<IResourceBindGroup>(device->CreateResourceBindGroup(modelBindGroupDesc));

modelBindGroup->BeginUpdate();
modelBindGroup->Cbv(0, modelBuffer.get());
modelBindGroup->EndUpdate();

// Bind both resource groups when rendering
commandList->BindResourceGroup(viewProjBindGroup.get());
commandList->BindResourceGroup(modelBindGroup.get());
```