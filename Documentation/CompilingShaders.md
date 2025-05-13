# Compiling Shaders in DenOfIz

DenOfIz uses a unified shader compilation system that translates HLSL shaders to the appropriate format for the target graphics API: DirectX 12 (DXIL), Vulkan (SPIRV), or Metal (MSL). This document explains the shader compilation process and cross-platform considerations.

## Shader Compilation Overview

DenOfIz uses Microsoft's DirectX Shader Compiler (DXC) for all shader compilation, even when targeting non-DirectX APIs. This approach allows developers to write shaders once in HLSL and automatically compile them for any supported platform.

### Basic Shader Compilation

```cpp
// Create shader compiler
ShaderCompiler compiler;

// Set up compile descriptor
CompileDesc compileDesc;
compileDesc.Path = "Assets/Shaders/MyShader.hlsl";
compileDesc.Stage = ShaderStage::Pixel;
compileDesc.EntryPoint = "main";
compileDesc.TargetIL = TargetIL::DXIL;  // Or SPIRV for Vulkan, MSL for Metal

// Compile the shader
CompileResult result = compiler.CompileHLSL(compileDesc);
```

## Cross-Platform Shader Considerations

### Common Features for All Platforms

- HLSL version 2021 is used for all platforms
- Row-major matrix layout (`-Zpr` flag) is used by default
- Debug information is included in debug builds
- Resource binding spaces are used to organize resources

### DirectX 12 (DXIL)

DirectX 12 is the most straightforward target since HLSL was designed for it:

```hlsl
// Standard HLSL for DirectX 12
ConstantBuffer<PerFrameData> frameData : register(b0, space0);
Texture2D<float4> colorTexture : register(t0, space0);
SamplerState linearSampler : register(s0, space0);
```

### Vulkan (SPIRV)

Vulkan requires some special annotations and considerations:

1. **Push Constants**: Vulkan push constants require the `[[vk::push_constant]]` attribute and must be in register space 31:

```hlsl
// Push constant definition for Vulkan
struct PushConstants {
    float4 color;
    float2 scale;
    float2 offset;
};

[[vk::push_constant]] ConstantBuffer<PushConstants> pushConstants : register(b0, space31);
```

2. **Single Push Constant Block**: Vulkan only allows a single push constant block, so all push constant data must be organized in a properly aligned struct.

3. **Register Shifting**: DenOfIz applies register shifting for Vulkan to avoid binding conflicts. In the ShaderCompiler, you can see:

```cpp
// From ShaderCompiler.cpp
// Vulkan requires unique binding for each descriptor
// Docs suggest shifting the binding to avoid conflicts.
static const std::wstring VkShiftCbvWs = std::to_wstring(VkShiftCbv);
static const std::wstring VkShiftSrvWs = std::to_wstring(VkShiftSrv);
static const std::wstring VkShiftUavWs = std::to_wstring(VkShiftUav);
static const std::wstring VkShiftSamplerWs = std::to_wstring(VkShiftSampler);
```

4. **Extensions**: The compiler automatically adds necessary extensions for ray tracing in Vulkan:

```cpp
arguments.push_back(L"-fspv-extension=SPV_KHR_ray_tracing");
arguments.push_back(L"-fspv-extension=SPV_KHR_ray_query");
```

### Metal (MSL)

Metal has the most significant differences in its resource binding model:

1. **Reflection Required**: Metal relies heavily on reflection data to map HLSL bindings to Metal argument buffers:

```cpp
// From ShaderCompiler.cpp
if (compileDesc.TargetIL == TargetIL::MSL)
{
    LOG(FATAL) << "MSL requires a root signature to provide an accurate metallib with the context of all shaders. "
               << "Using shader reflection create an IRRootSignature and pass it to the DxilToMsl class.";
    return {};
}
```

2. **DxilToMsl Conversion**: For Metal, use the DxilToMsl converter which uses reflection to properly bind resources:

```cpp
// Configure MSL conversion
DxilToMslDesc dxilToMslDesc;
dxilToMslDesc.Shaders = shaderStageDescs;
dxilToMslDesc.DXILShaders = compiledDxilShaders;

// Convert DXIL to MSL
DxilToMsl converter;
InteropArray<InteropArray<Byte>> mslData = converter.Convert(dxilToMslDesc);
```

3. **Explicit Binding Order**: Metal binds resources in a strict order within argument buffers, requiring careful organization of resources.

## Shader Program Creation and Reflection

For proper cross-platform compatibility, always use shader reflection to create root signatures:

```cpp
// Create shader program
ShaderProgram* program = graphicsApi->CreateShaderProgram(shaders);

// Get reflection data
ShaderReflectDesc reflectDesc = program->Reflect();

// Create root signature using reflection data
IRootSignature* rootSignature = device->CreateRootSignature(reflectDesc.RootSignature);
```

Reflection is particularly important for Metal, which requires detailed knowledge of the shader resource layout to translate HLSL bindings to Metal's argument buffer system.

## Best Practices for Cross-Platform Shader Development

1. **Always Use Reflection**: Let the shader reflection system handle binding information across platforms.

2. **Use Register Spaces Consistently**:
   - Space 0: Standard descriptor tables
   - Space 2: Root-level buffers (DirectX 12 optimization)
   - Space 31: Push constants

3. **Use Push Constants for Small, Frequently Updated Data**:
   ```hlsl
   [[vk::push_constant]] ConstantBuffer<PushConstants> pushConstants : register(b0, space31);
   ```

4. **Keep Push Constant Structs Compact**:
   - Keep push constant data small and aligned
   - Remember Vulkan only allows one push constant block

5. **Test on All Target Platforms**:
   - Some shader features might compile but behave differently across APIs
   - Use API-specific validation layers and debugging tools

6. **Avoid API-Specific Features When Possible**:
   - Use DenOfIz abstractions rather than platform-specific code
   - If needed, use `#ifdef` directives for platform-specific code

## Debugging Shaders

DenOfIz includes debugging support for shaders:

1. **Debug Information**: Debug builds include shader debug information
2. **Reflection Debugging**: Use the ReflectionDebugOutput class to inspect shader bindings:

```cpp
// Debug root parameters
ReflectionDebugOutput::DumpIRRootParameters(rootParameters, "Root Signature");
```

3. **Error Handling**: The shader compiler provides detailed error messages when compilation fails

## Advanced: Shader Variants and Dynamic Compilation

For advanced usage, you may want to compile shader variants at runtime:

```cpp
// Create shader with defines
CompileDesc compileDesc;
compileDesc.Path = "Assets/Shaders/MyShader.hlsl";
compileDesc.Stage = ShaderStage::Pixel;
compileDesc.EntryPoint = "main";

// Add preprocessor defines
InteropArray<InteropString> defines;
defines.EmplaceElement("USE_NORMAL_MAPPING");
defines.EmplaceElement("MAX_LIGHTS=4");
compileDesc.Defines = defines;

// Compile with the defines
CompileResult result = compiler.CompileHLSL(compileDesc);
```

## Conclusion

DenOfIz's shader compilation system abstracts away most of the complexity of targeting multiple graphics APIs, but understanding the underlying differences is important for effective cross-platform development. Always use shader reflection, follow the register space conventions, and test on all target platforms to ensure compatibility.