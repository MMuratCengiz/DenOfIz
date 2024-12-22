# Shortcomings


# Local Root Signature

In  general local root signatures are not really supported. They are only intended to be used with the RayTracing API.
The concept of a LocalRootSignature from DirectX12 is exposed via IShaderLocalDataLayout.

# Metal 

1 In Metal RayTracing, local root signature is tied to the source code. It is not possible to specify a binding
local to a specific shader. This is a limitation of Metal RayTracing API. If you need to do this, you will have to
split the shader into multiple shaders and use a different local root signature for each shader.
