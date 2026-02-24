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

#include "CommonData.h"
#include "DenOfIzGraphics/Handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Opaque handle to a vertex input layout definition.
     *
     * An input layout describes how vertex data is organized in vertex buffers and
     * how it maps to shader input attributes. It defines the format, offset, and
     * semantic meaning of each vertex element.
     *
     * @par Backend Implementations
     * - **DirectX 12**: D3D12_INPUT_LAYOUT_DESC with D3D12_INPUT_ELEMENT_DESC array
     * - **Vulkan**: VkPipelineVertexInputStateCreateInfo with VkVertexInputBindingDescription
     *               and VkVertexInputAttributeDescription arrays
     * - **Metal**: MTLVertexDescriptor
     * - **WebGPU**: WGPUVertexBufferLayout array with WGPUVertexAttribute arrays
     *
     * @par Usage
     * Input layouts are typically created from shader reflection data via
     * DenOfIz_ShaderProgram_Reflect, which populates InputLayout automatically.
     * The input layout is then passed to pipeline creation via DenOfIz_PipelineDesc.
     *
     * @par Example
     * @code
     * // From shader reflection (recommended):
     * DenOfIz_ShaderReflectDesc reflectDesc = {0};
     * DenOfIz_ShaderProgram_Reflect(program, &reflectDesc);
     *
     * DenOfIz_InputLayout inputLayout;
     * DenOfIz_LogicalDevice_CreateInputLayout(device, &reflectDesc.InputLayout, &inputLayout);
     *
     * // Use in pipeline:
     * DenOfIz_PipelineDesc pipelineDesc = {0};
     * pipelineDesc.InputLayout   = inputLayout;
     * pipelineDesc.ShaderProgram = program;
     * // ... other fields ...
     * @endcode
     *
     * @see DenOfIz_InputLayoutDesc
     * @see DenOfIz_LogicalDevice_CreateInputLayout
     * @see DenOfIz_PipelineDesc
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_InputLayout )

    /**
     * @brief Vertex attribute fetch rate.
     */
    typedef enum DenOfIz_StepRate
    {
        DENOFIZ_STEP_RATE_PER_VERTEX,  /**< Attribute advances per vertex (standard vertex data). */
        DENOFIZ_STEP_RATE_PER_INSTANCE /**< Attribute advances per instance (instanced data). */
    } DenOfIz_StepRate;

    /**
     * @brief Standard vertex attribute semantics.
     *
     * Maps to HLSL semantics (POSITION, NORMAL, etc.). These values are primarily
     * for reference; the actual semantic string is specified in DenOfIz_InputLayoutElementDesc.
     */
    typedef enum DenOfIz_Semantic
    {
        DENOFIZ_SEMANTIC_POSITION,          /**< Vertex position (POSITION). */
        DENOFIZ_SEMANTIC_NORMAL,            /**< Vertex normal (NORMAL). */
        DENOFIZ_SEMANTIC_COLOR,             /**< Vertex color (COLOR). */
        DENOFIZ_SEMANTIC_TANGENT,           /**< Tangent vector (TANGENT). */
        DENOFIZ_SEMANTIC_BINORMAL,          /**< Binormal vector (BINORMAL). */
        DENOFIZ_SEMANTIC_BITANGENT,         /**< Bitangent vector (BITANGENT). */
        DENOFIZ_SEMANTIC_BLEND_INDICES,     /**< Bone indices for skinning (BLENDINDICES). */
        DENOFIZ_SEMANTIC_BLEND_WEIGHT,      /**< Bone weights for skinning (BLENDWEIGHT). */
        DENOFIZ_SEMANTIC_TEXTURE_COORDINATE /**< Texture coordinates (TEXCOORD). */
    } DenOfIz_Semantic;

    /**
     * @brief Describes a single vertex attribute element.
     */
    typedef struct DenOfIz_InputLayoutElementDesc
    {
        DenOfIz_StringView Semantic;      /**< Semantic name (e.g., "POSITION", "TEXCOORD"). */
        uint32_t           SemanticIndex; /**< Semantic index for multiple attributes of same type (e.g., TEXCOORD0, TEXCOORD1). */
        DenOfIz_Format     Format;        /**< Data format (e.g., DENOFIZ_FORMAT_R32G32B32_FLOAT for float3). */
    } DenOfIz_InputLayoutElementDesc;

    /**
     * @brief Array of input layout element descriptors.
     */
    typedef struct DenOfIz_InputLayoutElementDescArray
    {
        DenOfIz_InputLayoutElementDesc *Elements;    /**< Pointer to array of element descriptors. */
        size_t                          NumElements; /**< Number of elements in the array. */
    } DenOfIz_InputLayoutElementDescArray;

    /**
     * @brief Describes a vertex buffer binding with its elements.
     *
     * Groups vertex attributes that come from the same vertex buffer.
     * Multiple input groups allow interleaved or separate vertex buffers.
     */
    typedef struct DenOfIz_InputGroupDesc
    {
        DenOfIz_StepRate                    StepRate; /**< Per-vertex or per-instance stepping. */
        DenOfIz_InputLayoutElementDescArray Elements; /**< Vertex attributes in this buffer. */
        uint32_t                            Stride;   /**< Bytes between consecutive vertices (0 = tightly packed). */
    } DenOfIz_InputGroupDesc;

    /**
     * @brief Array of input group descriptors.
     */
    typedef struct DenOfIz_InputGroupDescArray
    {
        DenOfIz_InputGroupDesc *Elements;    /**< Pointer to array of group descriptors. */
        size_t                  NumElements; /**< Number of groups (vertex buffer bindings). */
    } DenOfIz_InputGroupDescArray;

    /**
     * @brief Describes the complete vertex input layout for a pipeline.
     *
     * @par Manual Construction Example
     * @code
     * // Define vertex elements
     * DenOfIz_InputLayoutElementDesc elements[3];
     * elements[0].Semantic      = DENOFIZ_STRING("POSITION");
     * elements[0].SemanticIndex = 0;
     * elements[0].Format        = DENOFIZ_FORMAT_R32G32B32_FLOAT;
     *
     * elements[1].Semantic      = DENOFIZ_STRING("NORMAL");
     * elements[1].SemanticIndex = 0;
     * elements[1].Format        = DENOFIZ_FORMAT_R32G32B32_FLOAT;
     *
     * elements[2].Semantic      = DENOFIZ_STRING("TEXCOORD");
     * elements[2].SemanticIndex = 0;
     * elements[2].Format        = DENOFIZ_FORMAT_R32G32_FLOAT;
     *
     * // Create input group
     * DenOfIz_InputGroupDesc group = {0};
     * group.StepRate         = DENOFIZ_STEP_RATE_PER_VERTEX;
     * group.Elements.Elements    = elements;
     * group.Elements.NumElements = 3;
     * group.Stride           = sizeof(float) * 8;  // 3 + 3 + 2 floats
     *
     * // Create input layout desc
     * DenOfIz_InputLayoutDesc layoutDesc = {0};
     * layoutDesc.InputGroups.Elements    = &group;
     * layoutDesc.InputGroups.NumElements = 1;
     *
     * DenOfIz_InputLayout inputLayout;
     * DenOfIz_LogicalDevice_CreateInputLayout(device, &layoutDesc, &inputLayout);
     * @endcode
     */
    typedef struct DenOfIz_InputLayoutDesc
    {
        DenOfIz_InputGroupDescArray InputGroups; /**< Vertex buffer bindings with their attributes. */
    } DenOfIz_InputLayoutDesc;

    /**
     * @brief Destroys the input layout and releases associated resources.
     *
     * @param inputLayout Input layout handle to destroy.
     *
     * @par Valid Usage
     * - @p inputLayout must be a valid DenOfIz_InputLayout handle or DENOFIZ_NULL_HANDLE
     * - If @p inputLayout is DENOFIZ_NULL_HANDLE, this function returns immediately
     * - The input layout must not be in use by any pipeline
     * - After destruction, the handle is invalid and must not be used
     */
    DZ_API void DenOfIz_InputLayout_Destroy( DenOfIz_InputLayout inputLayout );

#ifdef __cplusplus
}
#endif
