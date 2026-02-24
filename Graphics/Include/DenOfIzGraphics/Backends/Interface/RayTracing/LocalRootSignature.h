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

#include "DenOfIzGraphics/Backends/Interface/RootSignature.h"
#include "DenOfIzGraphics/Handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct DenOfIz_LocalResourceBindingDesc
    {
        uint32_t                        Binding;
        uint32_t                        RegisterSpace;
        DenOfIz_ResourceDescriptorFlags Descriptor;
        DenOfIz_ShaderStageFlags        Stages;
        int                             ArraySize;
        uint64_t                        NumBytes;
    } DenOfIz_LocalResourceBindingDesc;

    typedef struct DenOfIz_LocalResourceBindingDescArray
    {
        DenOfIz_LocalResourceBindingDesc *Elements;
        uint32_t                          NumElements;
    } DenOfIz_LocalResourceBindingDescArray;

    /**
     * @brief Handle to a local root signature defining per-shader resource bindings for ray tracing.
     *
     * Local root signatures define resource bindings (CBV, SRV, UAV, samplers) that are unique to each
     * shader in a ray tracing pipeline. Unlike global root signatures which apply to all shaders, local
     * root signatures allow each shader to have its own set of resources.
     *
     * @par Backend Implementations
     * - DirectX12: ID3D12RootSignature (local)
     * - Vulkan: VkDescriptorSetLayout (merged into pipeline during creation)
     * - Metal: Custom descriptor table layout (encoded in shader binding table)
     * - WebGPU: Not supported (ray tracing not available)
     *
     * @par Usage
     * Local root signatures are obtained from shader reflection and define the layout for shader local data.
     * They are used when creating ShaderLocalData objects to bind resources unique to specific shaders
     * (such as per-hit group data).
     *
     * @par Example
     * @code
     * // Reflect shader to get local root signature
     * DenOfIz_ShaderReflectDesc reflection = {0};
     * DenOfIz_ShaderProgram_Reflect(rayTracingProgram, &reflection);
     *
     * // Create local root signature for hit group shader
     * DenOfIz_LocalRootSignature hitGroupLayout;
     * DenOfIz_LogicalDevice_CreateLocalRootSignature(
     *     device,
     *     &reflection.LocalRootSignatures.Elements[1],
     *     &hitGroupLayout
     * );
     *
     * // Use local root signature to create shader local data
     * DenOfIz_ShaderLocalDataDesc localDataDesc = {0};
     * localDataDesc.Layout = hitGroupLayout;
     * DenOfIz_ShaderLocalData localData;
     * DenOfIz_LogicalDevice_CreateShaderLocalData(device, &localDataDesc, &localData);
     *
     * // Bind resources to local data
     * DenOfIz_ShaderLocalData_Begin(localData);
     * DenOfIz_ShaderLocalData_CbvBuffer(localData, 0, materialBuffer);
     * DenOfIz_ShaderLocalData_End(localData);
     *
     * // Cleanup
     * DenOfIz_ShaderLocalData_Destroy(localData);
     * DenOfIz_LocalRootSignature_Destroy(hitGroupLayout);
     * @endcode
     *
     * @see DenOfIz_ShaderLocalData
     * @see DenOfIz_ShaderBindingTable
     * @see DenOfIz_ShaderProgram_Reflect
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_LocalRootSignature )

    /**
     * @brief Descriptor for creating a local root signature.
     *
     * @param ResourceBindings Array of local resource binding descriptors defining the layout.
     *                         Uses DenOfIz_LocalResourceBindingDesc which includes NumBytes
     *                         for inline constant buffer data in ray tracing.
     *                         Typically obtained from shader reflection.
     */
    typedef struct DenOfIz_LocalRootSignatureDesc
    {
        DenOfIz_LocalResourceBindingDescArray ResourceBindings;
    } DenOfIz_LocalRootSignatureDesc;

    typedef struct DenOfIz_LocalRootSignatureDescArray
    {
        DenOfIz_LocalRootSignatureDesc *Elements;
        uint32_t                        NumElements;
    } DenOfIz_LocalRootSignatureDescArray;

    typedef struct DenOfIz_LocalRootSignatureArray
    {
        DenOfIz_LocalRootSignature *Elements;
        size_t                      NumElements;
    } DenOfIz_LocalRootSignatureArray;

    /**
     * @brief Destroys a local root signature and releases associated resources.
     *
     * @param localRootSignature Valid local root signature handle to destroy.
     *
     * @par Valid Usage
     * - @p localRootSignature must be a valid DenOfIz_LocalRootSignature handle
     * - All ShaderLocalData objects created with this signature must be destroyed first
     * - Must not be used in any active shader binding tables
     */
    DZ_API void DenOfIz_LocalRootSignature_Destroy( DenOfIz_LocalRootSignature localRootSignature );

#ifdef __cplusplus
}
#endif
