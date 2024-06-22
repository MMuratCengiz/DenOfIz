### To include the engine in your project, you need to add the following lines to your `CMakeLists.txt`:

```cmake
add_subdirectory(Engine)
target_link_libraries(YourProjectName Engine)
```

Some packages are required to build the engine. This is following vcpkg.json:

```json
{
  "dependencies": [
    "wil",
    "glog",
    "vulkan",
    "vulkan-memory-allocator",
    "glslang",
    "spirv-cross",
    "stb",
    "tinygltf",
    "python3",
    {
      "name": "sdl2",
      "features": ["vulkan"]
    },
    { "name": "d3d12-memory-allocator", "platform": "windows" },
    { "name": "directx12-agility", "platform": "windows" },
    { "name": "directxtk12", "platform": "windows" },
    { "name": "directxtex", "platform": "windows" },
    { "name": "directx-headers", "platform": "windows" }
  ]
}
```

In the future this will probably be bundled into the Engine project.

For windows you need to setup the DirectX Agility SDK.