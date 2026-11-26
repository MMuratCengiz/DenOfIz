# Shortcomings

## Local Root Signatures

Local root signatures only exist for ray tracing. A stage lists its local bindings in `DenOfIz_RayTracingShaderDesc::LocalBindings`, they are reflected into
`DenOfIz_ShaderReflectDesc::LocalRootSignatures`, created as a `DenOfIz_LocalRootSignature` and filled per shader record through `DenOfIz_ShaderLocalData`.

On Metal every shader of a program is compiled against one local root signature made of the local bindings of all its shaders, so a binding can't be local to a single
shader. Root level buffers (space 30) can't be local on Metal, use a local constant buffer instead.

## Metal

- macOS on arm64 only.
- `DenOfIz_CommandList_PipelineBarrier` is a no-op.
- A bind group keeps only the bindings of its latest update, bind everything the shaders use in every update.
- No pipeline statistics queries, occlusion queries only work inside a rendering pass.
- No blend logic ops.
- The shaders of AABBS hit groups, intersection shaders included, need `DenOfIz_RayTracingShaderDesc::HitGroupType` set to `DENOFIZ_HIT_GROUP_TYPE_AABBS`.

## DirectX 12

- Bindless sampler arrays are not implemented.
- Debug markers are only recorded in debug builds with PIX (the `pix` vcpkg feature).

## Shader Conversion

Metal libraries are only produced on Windows x64 and macOS, Linux and Windows arm64 builds have no Metal Shader Converter.
