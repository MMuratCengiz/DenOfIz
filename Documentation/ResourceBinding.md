# Resource Binding in DenOfIz

Shader resources are grouped by HLSL register space. Every register space a pipeline uses has a bind group layout, the root signature is those layouts plus the root
constants, and while recording one bind group per register space provides the resources.

| Object                    | Created from                                                        | Holds                                   |
|---------------------------|---------------------------------------------------------------------|-----------------------------------------|
| `DenOfIz_ShaderProgram`   | `DenOfIz_ShaderProgram_Create`                                      | Compiled stages and their reflection    |
| `DenOfIz_BindGroupLayout` | A `DenOfIz_BindGroupLayoutDesc` of the reflection                   | The bindings of one register space      |
| `DenOfIz_RootSignature`   | The bind group layouts and the reflected root constants             | The resource layout of a pipeline       |
| `DenOfIz_BindGroup`       | A bind group layout                                                 | The resources bound to a register space |

## Layouts from Reflection

```cpp
DenOfIz_ShaderReflectDesc reflectDesc{ };
DenOfIz_ShaderProgram_Reflect( program, &reflectDesc );

std::vector<DenOfIz_BindGroupLayout> layouts( reflectDesc.BindGroupLayouts.NumElements );
for ( uint32_t i = 0; i < reflectDesc.BindGroupLayouts.NumElements; ++i )
{
    // reflectDesc.BindGroupLayouts.Elements[ i ].RegisterSpace is the space of the layout
    DenOfIz_LogicalDevice_CreateBindGroupLayout( device, &reflectDesc.BindGroupLayouts.Elements[ i ], &layouts[ i ] );
}

DenOfIz_RootSignatureDesc rootSignatureDesc{ };
rootSignatureDesc.BindGroupLayouts.Elements    = layouts.data( );
rootSignatureDesc.BindGroupLayouts.NumElements = layouts.size( );
rootSignatureDesc.RootConstants                = reflectDesc.RootConstants;
DenOfIz_RootSignature rootSignature;
DenOfIz_LogicalDevice_CreateRootSignature( device, &rootSignatureDesc, &rootSignature );
```

Build the layouts from the reflection: Metal compiles the shaders against the layout the reflection describes.

## Bind Groups

```hlsl
Texture2D    Albedo        : register(t0, space0);
SamplerState LinearSampler : register(s0, space0);
cbuffer PerFrame           : register(b0, space1) { float4x4 ViewProjection; };
```

```cpp
DenOfIz_BindGroupDesc bindGroupDesc{ };
bindGroupDesc.Layout = materialLayout; // The layout of space0
DenOfIz_BindGroup materialGroup;
DenOfIz_LogicalDevice_CreateBindGroup( device, &bindGroupDesc, &materialGroup );

DenOfIz_BindGroup_BeginUpdate( materialGroup );
DenOfIz_BindGroup_SrvTexture( materialGroup, 0, albedo ); // t0
DenOfIz_BindGroup_Sampler( materialGroup, 0, sampler );   // s0
DenOfIz_BindGroup_EndUpdate( materialGroup );

// While recording
DenOfIz_CommandList_BindGroup( commandList, materialGroup );
DenOfIz_CommandList_BindGroup( commandList, perFrameGroup ); // space1, its Cbv bound the same way
```

- The binding passed to `Cbv`, `SrvTexture`, `UavBuffer`, `Sampler`, ... is the register number (b, t, u or s).
- A `Cbv` needs `DENOFIZ_BUFFER_USAGE_UNIFORM_BIT`, a UAV buffer `DENOFIZ_BUFFER_USAGE_STORAGE_BIT`, an SRV texture `DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT` and a UAV texture
  `DENOFIZ_TEXTURE_USAGE_STORAGE_BINDING_BIT`.
- Updates overwrite the group in place, don't update a group that command lists still running on the GPU use. Keep one group per frame in flight
  (`Examples/Base/Source/QuadPipeline.cpp`).
- Metal keeps only the bindings of the latest update, bind everything the shaders use in every update.
- Bind groups and root constants are applied at the next draw or dispatch, matched to the bound pipeline by register space and binding. They can be set before or after
  `DenOfIz_CommandList_BindPipeline` and stay set across pipeline changes. Binding a group replaces the one bound at the same register space.

## Buffer Offsets

`DenOfIz_BindBufferDesc` binds a buffer from an offset, for example the slice of one draw in a uniform buffer shared by many:

```cpp
DenOfIz_BindBufferDesc bindDesc{ };
bindDesc.Binding        = 0;
bindDesc.Resource       = uniformBuffer;
bindDesc.ResourceOffset = drawIndex * alignedNumBytes; // A multiple of DenOfIz_DeviceConstants::ConstantBufferAlignment

DenOfIz_BindGroup_BeginUpdate( bindGroup );
DenOfIz_BindGroup_CbvWithDesc( bindGroup, &bindDesc );
DenOfIz_BindGroup_EndUpdate( bindGroup );
```

`DenOfIz_LogicalDevice_DeviceInfo` returns the device constants. `SrvBufferWithDesc` and `UavBufferWithDesc` take the same desc.

## Root Constants

Constant buffers in space 31 (`DENOFIZ_ROOT_CONSTANT_REGISTER_SPACE`) are root constants, push constants on Vulkan, and are not part of any bind group:

```hlsl
struct PushConstants { float4 Color; };
[[vk::push_constant]] ConstantBuffer<PushConstants> pushConstants : register(b0, space31);
```

```cpp
float             color[ 4 ] = { 1.0f, 0.0f, 0.0f, 1.0f };
DenOfIz_ByteArray data{ reinterpret_cast<Byte *>( color ), sizeof( color ) };
DenOfIz_CommandList_SetRootConstants( commandList, 0, &data ); // 0 is the b register
```

- The size must be a multiple of 4 and at most the reflected `NumBytes`.
- `[[vk::push_constant]]` is needed for Vulkan, which allows one push constant block per shader.
- DX12 applies root constants to graphics and compute pipelines only.

See `Examples/RootConstants`.

## Root Level Buffers

Buffers in space 30 (`DENOFIZ_ROOT_LEVEL_BUFFER_REGISTER_SPACE`) are bound as root descriptors on DX12 and Metal instead of through a descriptor table, Vulkan binds them like any
other space. They are still set through the bind group of space 30.

## Bindless

Unbounded arrays are declared per shader stage with `DenOfIz_BindlessSlot` and filled with `DenOfIz_BindGroup_SrvArray` or `DenOfIz_BindGroup_SrvArrayIndex`:

```hlsl
Texture2D<float4> g_Textures[] : register(t0, space0);
```

```cpp
DenOfIz_BindlessSlot slot{ };
slot.RegisterSpace = 0;
slot.Binding       = 0;
slot.MaxArraySize  = NumTextures;
slot.Descriptor    = DENOFIZ_RESOURCE_DESCRIPTOR_TEXTURE_BIT;

pixelStage.Bindless.BindlessArrays.Elements    = &slot;
pixelStage.Bindless.BindlessArrays.NumElements = 1;

// Once the program, layouts and the bind group of space 0 exist
DenOfIz_TextureArray textureArray{ textures, NumTextures };
DenOfIz_BindGroup_BeginUpdate( bindGroup );
DenOfIz_BindGroup_SrvArray( bindGroup, 0, &textureArray );
DenOfIz_BindGroup_EndUpdate( bindGroup );
```

See `Examples/Bindless`.

## Vulkan Register Shifts

Vulkan has a single binding range per descriptor set, so the shader compiler shifts each register type: b by `DENOFIZ_VK_SHIFT_CBV` (100), t by `DENOFIZ_VK_SHIFT_SRV` (200),
u by `DENOFIZ_VK_SHIFT_UAV` (300) and s by `DENOFIZ_VK_SHIFT_SAMPLER` (400). Registers of a type must stay below 100 within a space to not collide.

## Ray Tracing

Bindings listed in `DenOfIz_RayTracingShaderDesc::LocalBindings` are left out of the root signature and reflected into `DenOfIz_ShaderReflectDesc::LocalRootSignatures` instead, to be
filled per shader record through `DenOfIz_ShaderLocalData`. See `Examples/RayTracedProceduralGeometry` and [Known Limitations](Shortcomings.md).
