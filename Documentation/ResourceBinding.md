# Binding Resources

Binding resources is the main challenge of abstracting the rendering API. It is the process of making data available to
the GPU. This is how you can access a texture or the model view projection matrix in your shaders.

In DenOfIz the process of the binding resources starts all the way from the ShaderProgram and ends up with resource bind
groups.

First create a ShaderProgram including all the shaders:

```cpp
std::vector<ShaderDesc> shaders{ };
ShaderDesc              vertexShaderDesc{ };
vertexShaderDesc.Path  = "Assets/Shaders/FullscreenQuad.vs.hlsl";
vertexShaderDesc.Stage = ShaderStage::Vertex;
shaders.push_back( vertexShaderDesc );
ShaderDesc pixelShaderDesc{ };
pixelShaderDesc.Path  = pixelShader;
pixelShaderDesc.Stage = ShaderStage::Pixel;
shaders.push_back( pixelShaderDesc );
std::unique_ptr<ShaderProgram> program = graphicsApi->CreateShaderProgram( shaders );
```

Then create a RootSignature and ResourceBindGroup from the reflection data from the ShaderProgram.

```cpp
ShaderReflectDesc programReflection = m_program->Reflect( );

std::unique_ptr<IRootSignature> rootSignature = logicalDevice->CreateRootSignature( programReflection.RootSignature );
std::unique_ptr<IResourceBindGroup> inputLayout = logicalDevice->CreateInputLayout( programReflection.InputLayout );
```

Finally, bind the resources, note that in most cases this cannot be done while the command list is being recorded with
the bind group. It is the responsibility of the user to ensure that the resources and the bind group are not in use.

Binding is done exactly they are defined in HLSL shaders themselves. You provide an UpdateDesc item given a register
space. And you can bind a buffer, texture, or sampler to certain bindings.

```cpp
UpdateDesc updateDesc = 
    UpdateDesc( 0 )
        .Cbv( 0, buffer.get( ) )
        .Srv( 0, texture.get( ) )
        .Sampler( 0, sampler.get( ) );
```

### RootConstants

Root constants are also bound to the resource bind group. They use the special register space of 99.

Make sure to also add the `[[vk::push_constant]]` attribute to the shader constant buffer.

You can then simply set the root constants in the bind group. The size is inferred from the reflection data. And the
register space is hardcoded to 31. So you just need to pass the data.

```cpp
resourceBindGroup->SetRootConstants( 0, data );
```

### Optimized Register Space

DirectX allows a limited number of buffers/textures to be bound directly without a descriptor table, any buffer you bind
to register space 2 will automatically be created as such a resource. Vulkan has no such concept so it doesn't
quite apply there.

Note only buffers can be bound to root level descriptors, therefore textures/samplers will still be placed into
descriptor tables as normal.