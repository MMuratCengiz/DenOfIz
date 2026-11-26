# Public API

The public api is completely implemented in C, specifically designed to be ABI and binding friendly. The headers are in `Graphics/Include/DenOfIzGraphics`,
`DenOfIzGraphics_Ext.h` includes all of them. The C# bindings are generated from these headers (see [How to Build](HowToBuild.md#c-bindings)).

## Conventions

- **Handles**: objects are opaque `uint64_t` handles declared with `DENOFIZ_DEFINE_HANDLE`. `DENOFIZ_NULL_HANDLE` is the null handle, `DENOFIZ_HANDLE_IS_VALID` tests one.
- **Functions**: `DenOfIz_<Object>_<Action>`, the object handle first, for example `DenOfIz_CommandList_Draw( commandList, ... )`.
- **Creation**: device objects are created by the logical device from a desc and returned through an out parameter, for example
  `DenOfIz_LogicalDevice_CreateBuffer( device, &desc, &outBuffer )`. Other objects have `DenOfIz_<Object>_Create`. Everything is released with `DenOfIz_<Object>_Destroy`.
- **Descs**: plain structs passed by pointer, start from a zero initialized desc and set what you need.
- **Arrays**: `{ Elements, NumElements }` structs such as `DenOfIz_ByteArray` and `DenOfIz_TextureArray`.
- **Strings**: `DenOfIz_StringView`, `{ Chars, NumChars }`, `DENOFIZ_STRING( "literal" )` builds one.
- **Errors**: `DenOfIz_Result`, returned by every logical device create function among others, test it with `DENOFIZ_SUCCEEDED` / `DENOFIZ_FAILED`.

## Lifetimes

The caller owns every object it creates. Buffers, textures, samplers, bind groups, bind group layouts, root signatures, input layouts, pipelines, query pools, acceleration
structures, shader binding tables, local root signatures and shader local data are released late: destroying them is safe while the GPU may still use them, they are freed once
a frame boundary has passed and the GPU finished the work submitted before it. Presenting is a frame boundary, applications that never present call
`DenOfIz_LogicalDevice_ReleaseDestroyedResources` once per iteration.

Command queues, command list pools, pipeline caches, fences, semaphores and swap chains are released by their `Destroy` call, so the GPU must be done with them first.
