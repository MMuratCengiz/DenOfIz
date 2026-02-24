/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include "DenOfIzGraphics/Backends/Interface/Buffer.h"
#include "DenOfIzGraphics/Backends/Interface/Texture.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "LocalRootSignature.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Handle to shader local data for binding per-shader resources in ray tracing.
     *
     * Shader local data objects bind resources (constant buffers, textures, samplers) that are unique
     * to individual shaders in a ray tracing pipeline. They are associated with hit groups, miss shaders,
     * or ray generation shaders via the shader binding table.
     *
     * @par Backend Implementations
     * - DirectX12: Raw byte buffer with GPU virtual addresses encoded
     * - Vulkan: VkDescriptorSet with inline data buffer
     * - Metal: MTLArgumentBuffer (descriptor table) with inline data
     * - WebGPU: Not supported (ray tracing not available)
     *
     * @par Usage
     * Shader local data provides per-shader resource binding for ray tracing. Unlike global bindings
     * (BindGroup), local data is embedded in the shader binding table and allows each shader
     * to have unique resource bindings.
     *
     * Typical use cases:
     * - Per-geometry material parameters (albedo, roughness, normal maps)
     * - Per-instance data in hit groups
     * - Light parameters in miss shaders
     *
     * @par Example
     * @code
     * // Create local root signature from shader reflection
     * DenOfIz_ShaderReflectDesc reflection = {0};
     * DenOfIz_ShaderProgram_Reflect(rayTracingProgram, &reflection);
     *
     * DenOfIz_LocalRootSignature hitGroupLayout;
     * DenOfIz_LogicalDevice_CreateLocalRootSignature(
     *     device,
     *     &reflection.LocalRootSignatures.Elements[1],
     *     &hitGroupLayout
     * );
     *
     * // Create shader local data
     * DenOfIz_ShaderLocalDataDesc localDataDesc = {0};
     * localDataDesc.Layout = hitGroupLayout;
     * DenOfIz_ShaderLocalData materialData;
     * DenOfIz_LogicalDevice_CreateShaderLocalData(device, &localDataDesc, &materialData);
     *
     * // Bind material properties
     * float color[4] = {1.0f, 0.0f, 0.0f, 1.0f};
     * DenOfIz_ByteArrayView colorData = {0};
     * colorData.Elements = (const Byte*)color;
     * colorData.NumElements = sizeof(color);
     *
     * DenOfIz_ShaderLocalData_Begin(materialData);
     * DenOfIz_ShaderLocalData_CbvData(materialData, 0, &colorData);
     * DenOfIz_ShaderLocalData_SrvTexture(materialData, 1, albedoTexture);
     * DenOfIz_ShaderLocalData_End(materialData);
     *
     * // Associate with hit group in shader binding table
     * DenOfIz_HitGroupBindingDesc hitGroupBinding = {0};
     * hitGroupBinding.HitGroupExportName = DENOFIZ_STRING("MaterialHitGroup");
     * hitGroupBinding.Data = materialData;
     * DenOfIz_ShaderBindingTable_BindHitGroup(sbt, &hitGroupBinding);
     *
     * // Cleanup
     * DenOfIz_ShaderLocalData_Destroy(materialData);
     * DenOfIz_LocalRootSignature_Destroy(hitGroupLayout);
     * @endcode
     *
     * @see DenOfIz_LocalRootSignature
     * @see DenOfIz_ShaderBindingTable
     * @see DenOfIz_HitGroupBindingDesc
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_ShaderLocalData )

    /**
     * @brief Descriptor for creating shader local data.
     *
     * @param Layout Local root signature defining the resource binding layout.
     */
    typedef struct
    {
        DenOfIz_LocalRootSignature Layout;
    } DenOfIz_ShaderLocalDataDesc;

    /**
     * @brief Begins a resource binding update sequence for shader local data.
     *
     * Must be called before binding any resources. Pair with DenOfIz_ShaderLocalData_End.
     *
     * @param localData Valid shader local data handle.
     *
     * @par Valid Usage
     * - @p localData must be a valid DenOfIz_ShaderLocalData handle
     * - Must not call Begin twice without calling End
     *
     * @see DenOfIz_ShaderLocalData_End
     */
    DZ_API void DenOfIz_ShaderLocalData_Begin( DenOfIz_ShaderLocalData localData );

    /**
     * @brief Binds a constant buffer resource to the specified binding slot.
     *
     * @param localData Valid shader local data handle.
     * @param binding Binding slot index matching shader declaration.
     * @param bufferResource Buffer resource to bind.
     *
     * @par Valid Usage
     * - @p localData must be a valid DenOfIz_ShaderLocalData handle
     * - Must be called between Begin and End
     * - @p binding must exist in the local root signature layout
     * - @p bufferResource must be a valid buffer handle with DENOFIZ_RESOURCE_DESCRIPTOR_UNIFORM_BUFFER_BIT
     *
     * @see DenOfIz_ShaderLocalData_CbvData
     */
    DZ_API void DenOfIz_ShaderLocalData_CbvBuffer( DenOfIz_ShaderLocalData localData, uint32_t binding, DenOfIz_Buffer bufferResource );

    /**
     * @brief Binds inline constant buffer data to the specified binding slot.
     *
     * Embeds small amounts of constant data directly in the shader binding table without requiring
     * a separate buffer resource. Preferred for small per-shader data.
     *
     * @param localData Valid shader local data handle.
     * @param binding Binding slot index matching shader declaration.
     * @param data Pointer to byte array view containing constant data.
     *
     * @par Valid Usage
     * - @p localData must be a valid DenOfIz_ShaderLocalData handle
     * - Must be called between Begin and End
     * - @p binding must exist in the local root signature layout
     * - @p data must not be NULL
     * - @p data->Elements must not be NULL
     * - @p data->NumElements must match the size expected by the binding
     *
     * @see DenOfIz_ShaderLocalData_CbvBuffer
     */
    DZ_API void DenOfIz_ShaderLocalData_CbvData( DenOfIz_ShaderLocalData localData, uint32_t binding, const DenOfIz_ByteArrayView *data );

    /**
     * @brief Binds a shader resource view buffer to the specified binding slot.
     *
     * @param localData Valid shader local data handle.
     * @param binding Binding slot index matching shader declaration.
     * @param bufferResource Buffer resource to bind as SRV.
     *
     * @par Valid Usage
     * - @p localData must be a valid DenOfIz_ShaderLocalData handle
     * - Must be called between Begin and End
     * - @p binding must exist in the local root signature layout
     * - @p bufferResource must be a valid buffer handle with DENOFIZ_RESOURCE_DESCRIPTOR_BUFFER_BIT
     */
    DZ_API void DenOfIz_ShaderLocalData_SrvBuffer( DenOfIz_ShaderLocalData localData, uint32_t binding, DenOfIz_Buffer bufferResource );

    /**
     * @brief Binds a shader resource view texture to the specified binding slot.
     *
     * @param localData Valid shader local data handle.
     * @param binding Binding slot index matching shader declaration.
     * @param textureResource Texture resource to bind as SRV.
     *
     * @par Valid Usage
     * - @p localData must be a valid DenOfIz_ShaderLocalData handle
     * - Must be called between Begin and End
     * - @p binding must exist in the local root signature layout
     * - @p textureResource must be a valid texture handle with DENOFIZ_RESOURCE_DESCRIPTOR_TEXTURE_BIT
     */
    DZ_API void DenOfIz_ShaderLocalData_SrvTexture( DenOfIz_ShaderLocalData localData, uint32_t binding, DenOfIz_Texture textureResource );

    /**
     * @brief Binds an unordered access view buffer to the specified binding slot.
     *
     * @param localData Valid shader local data handle.
     * @param binding Binding slot index matching shader declaration.
     * @param bufferResource Buffer resource to bind as UAV.
     *
     * @par Valid Usage
     * - @p localData must be a valid DenOfIz_ShaderLocalData handle
     * - Must be called between Begin and End
     * - @p binding must exist in the local root signature layout
     * - @p bufferResource must be a valid buffer handle with DENOFIZ_RESOURCE_DESCRIPTOR_RW_BUFFER_BIT
     */
    DZ_API void DenOfIz_ShaderLocalData_UavBuffer( DenOfIz_ShaderLocalData localData, uint32_t binding, DenOfIz_Buffer bufferResource );

    /**
     * @brief Binds an unordered access view texture to the specified binding slot.
     *
     * @param localData Valid shader local data handle.
     * @param binding Binding slot index matching shader declaration.
     * @param textureResource Texture resource to bind as UAV.
     *
     * @par Valid Usage
     * - @p localData must be a valid DenOfIz_ShaderLocalData handle
     * - Must be called between Begin and End
     * - @p binding must exist in the local root signature layout
     * - @p textureResource must be a valid texture handle with DENOFIZ_RESOURCE_DESCRIPTOR_RW_TEXTURE_BIT
     */
    DZ_API void DenOfIz_ShaderLocalData_UavTexture( DenOfIz_ShaderLocalData localData, uint32_t binding, DenOfIz_Texture textureResource );

    /**
     * @brief Binds a sampler to the specified binding slot.
     *
     * @param localData Valid shader local data handle.
     * @param binding Binding slot index matching shader declaration.
     * @param sampler Sampler to bind.
     *
     * @par Valid Usage
     * - @p localData must be a valid DenOfIz_ShaderLocalData handle
     * - Must be called between Begin and End
     * - @p binding must exist in the local root signature layout
     * - @p sampler must be a valid sampler handle
     */
    DZ_API void DenOfIz_ShaderLocalData_Sampler( DenOfIz_ShaderLocalData localData, uint32_t binding, DenOfIz_Sampler sampler );

    /**
     * @brief Ends a resource binding update sequence for shader local data.
     *
     * Finalizes resource bindings and prepares data for use in shader binding table.
     * Must be called after all resources are bound.
     *
     * @param localData Valid shader local data handle.
     *
     * @par Valid Usage
     * - @p localData must be a valid DenOfIz_ShaderLocalData handle
     * - Begin must have been called prior
     *
     * @see DenOfIz_ShaderLocalData_Begin
     */
    DZ_API void DenOfIz_ShaderLocalData_End( DenOfIz_ShaderLocalData localData );

    /**
     * @brief Destroys shader local data and releases associated resources.
     *
     * @param localData Valid shader local data handle to destroy.
     *
     * @par Valid Usage
     * - @p localData must be a valid DenOfIz_ShaderLocalData handle
     * - Must not be used in any active shader binding tables
     * - If Begin was called, End must be called before destruction
     */
    DZ_API void DenOfIz_ShaderLocalData_Destroy( DenOfIz_ShaderLocalData localData );

#ifdef __cplusplus
}
#endif
