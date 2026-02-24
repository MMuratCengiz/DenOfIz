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
#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"
#include "DenOfIzGraphics/Backends/Interface/InputLayout.h"
#include "DenOfIzGraphics/Backends/Interface/PipelineCache.h"
#include "DenOfIzGraphics/Backends/Interface/RayTracing/LocalRootSignature.h"
#include "DenOfIzGraphics/Backends/Interface/RootSignature.h"
#include "DenOfIzGraphics/Handle.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Handle to a GPU pipeline encapsulating shaders and fixed-function state.
     *
     * A pipeline combines shaders with render state (blending, depth/stencil, rasterization)
     * into a single object that can be efficiently bound for rendering. Pipelines are immutable
     * after creation; changes require creating a new pipeline.
     *
     * @par Backend Implementations
     * - DirectX12 (Graphics/Compute): ID3D12PipelineState
     * - DirectX12 (Ray Tracing): ID3D12StateObject
     * - Vulkan: VkPipeline (VK_PIPELINE_BIND_POINT_GRAPHICS/COMPUTE/RAY_TRACING_KHR)
     * - Metal (Graphics): MTLRenderPipelineState + MTLDepthStencilState
     * - Metal (Compute): MTLComputePipelineState
     * - WebGPU: WGPURenderPipeline or WGPUComputePipeline
     *
     * @par Pipeline Types (BindPoint)
     * - DENOFIZ_BIND_POINT_GRAPHICS: Traditional vertex/pixel shader pipeline
     * - DENOFIZ_BIND_POINT_COMPUTE: Compute shader dispatch
     * - DENOFIZ_BIND_POINT_RAYTRACING: Ray tracing with hit groups and SBT
     * - DENOFIZ_BIND_POINT_MESH: Mesh shader pipeline (task/mesh/pixel)
     *
     * @par Usage Example
     * @code
     * // Load shaders
     * DenOfIz_ShaderProgramDesc programDesc = {0};
     * // ... configure shader stages ...
     * DenOfIz_ShaderProgram program = DenOfIz_ShaderProgram_Create(&programDesc);
     *
     * // Get reflection for root signature and input layout
     * DenOfIz_ShaderReflectDesc reflection = {0};
     * DenOfIz_ShaderProgram_Reflect(program, &reflection);
     *
     * DenOfIz_RootSignature rootSignature;
     * DenOfIz_LogicalDevice_CreateRootSignature(device, &reflection.RootSignature, &rootSignature);
     *
     * DenOfIz_InputLayout inputLayout;
     * DenOfIz_LogicalDevice_CreateInputLayout(device, &reflection.InputLayout, &inputLayout);
     *
     * // Create graphics pipeline
     * DenOfIz_PipelineDesc pipelineDesc = {0};
     * pipelineDesc.BindPoint      = DENOFIZ_BIND_POINT_GRAPHICS;
     * pipelineDesc.ShaderProgram  = program;
     * pipelineDesc.RootSignature  = rootSignature;
     * pipelineDesc.InputLayout    = inputLayout;
     *
     * // Configure render targets
     * DenOfIz_RenderTargetDesc rtDesc = {0};
     * rtDesc.Format                   = DENOFIZ_FORMAT_B8G8R8A8_UNORM;
     * rtDesc.Blend.RenderTargetWriteMask = 0x0F;
     * pipelineDesc.Graphics.RenderTargets.Elements    = &rtDesc;
     * pipelineDesc.Graphics.RenderTargets.NumElements = 1;
     *
     * // Configure rasterization
     * pipelineDesc.Graphics.CullMode          = DENOFIZ_CULL_MODE_BACK_FACE;
     * pipelineDesc.Graphics.FillMode          = DENOFIZ_FILL_MODE_SOLID;
     * pipelineDesc.Graphics.PrimitiveTopology = DENOFIZ_PRIMITIVE_TOPOLOGY_TRIANGLE;
     *
     * // Configure depth testing
     * pipelineDesc.Graphics.DepthTest.Enable    = true;
     * pipelineDesc.Graphics.DepthTest.Write     = true;
     * pipelineDesc.Graphics.DepthTest.CompareOp = DENOFIZ_COMPARE_OP_LESS;
     * pipelineDesc.Graphics.DepthStencilAttachmentFormat = DENOFIZ_FORMAT_D32_FLOAT;
     *
     * DenOfIz_Pipeline pipeline;
     * DenOfIz_LogicalDevice_CreatePipeline(device, &pipelineDesc, &pipeline);
     *
     * // In render loop:
     * DenOfIz_CommandList_BindPipeline(cmd, pipeline);
     * @endcode
     *
     * @see DenOfIz_PipelineDesc
     * @see DenOfIz_LogicalDevice_CreatePipeline
     * @see DenOfIz_CommandList_BindPipeline
     */
    DENOFIZ_DEFINE_HANDLE( DenOfIz_Pipeline )

    /**
     * @brief Specifies the type of pipeline.
     */
    typedef enum DenOfIz_BindPoint
    {
        DENOFIZ_BIND_POINT_GRAPHICS,   /**< Vertex/pixel shader graphics pipeline */
        DENOFIZ_BIND_POINT_COMPUTE,    /**< Compute shader pipeline */
        DENOFIZ_BIND_POINT_RAYTRACING, /**< Ray tracing pipeline */
        DENOFIZ_BIND_POINT_MESH        /**< Mesh shader pipeline (task/mesh/pixel) */
    } DenOfIz_BindPoint;

    /**
     * @brief Preset blend modes for common use cases.
     */
    typedef enum DenOfIz_BlendMode
    {
        DENOFIZ_BLEND_MODE_NONE,       /**< No blending, overwrite destination */
        DENOFIZ_BLEND_MODE_ALPHA_BLEND /**< Standard alpha blending */
    } DenOfIz_BlendMode;

    /**
     * @brief Face culling mode for rasterization.
     */
    typedef enum DenOfIz_CullMode
    {
        DENOFIZ_CULL_MODE_NONE,      /**< No culling, render both sides */
        DENOFIZ_CULL_MODE_BACK_FACE, /**< Cull back-facing triangles (default) */
        DENOFIZ_CULL_MODE_FRONT_FACE /**< Cull front-facing triangles */
    } DenOfIz_CullMode;

    /**
     * @brief Polygon fill mode for rasterization.
     */
    typedef enum DenOfIz_FillMode
    {
        DENOFIZ_FILL_MODE_SOLID,    /**< Fill polygons (default) */
        DENOFIZ_FILL_MODE_WIREFRAME /**< Render polygon edges only */
    } DenOfIz_FillMode;

    /**
     * @brief Blend factor for source or destination in blend equations.
     *
     * Used in blend equations: Result = (SrcColor * SrcBlend) BlendOp (DstColor * DstBlend)
     */
    typedef enum DenOfIz_Blend
    {
        DENOFIZ_BLEND_ZERO,               /**< Factor = (0, 0, 0, 0) */
        DENOFIZ_BLEND_ONE,                /**< Factor = (1, 1, 1, 1) */
        DENOFIZ_BLEND_SRC_COLOR,          /**< Factor = source color */
        DENOFIZ_BLEND_INV_SRC_COLOR,      /**< Factor = (1 - source color) */
        DENOFIZ_BLEND_SRC_ALPHA,          /**< Factor = source alpha */
        DENOFIZ_BLEND_INV_SRC_ALPHA,      /**< Factor = (1 - source alpha) */
        DENOFIZ_BLEND_DST_ALPHA,          /**< Factor = destination alpha */
        DENOFIZ_BLEND_INV_DST_ALPHA,      /**< Factor = (1 - destination alpha) */
        DENOFIZ_BLEND_DST_COLOR,          /**< Factor = destination color */
        DENOFIZ_BLEND_INV_DST_COLOR,      /**< Factor = (1 - destination color) */
        DENOFIZ_BLEND_SRC_ALPHA_SATURATE, /**< Factor = min(src alpha, 1 - dst alpha) */
        DENOFIZ_BLEND_BLEND_FACTOR,       /**< Factor = constant blend factor */
        DENOFIZ_BLEND_INV_BLEND_FACTOR,   /**< Factor = (1 - constant blend factor) */
        DENOFIZ_BLEND_SRC1_COLOR,         /**< Factor = second source color (dual-source) */
        DENOFIZ_BLEND_INV_SRC1_COLOR,     /**< Factor = (1 - second source color) */
        DENOFIZ_BLEND_SRC1_ALPHA,         /**< Factor = second source alpha */
        DENOFIZ_BLEND_INV_SRC1_ALPHA      /**< Factor = (1 - second source alpha) */
    } DenOfIz_Blend;

    /**
     * @brief Operation to combine source and destination blend results.
     */
    typedef enum DenOfIz_BlendOp
    {
        DENOFIZ_BLEND_OP_ADD,              /**< Result = Src + Dst */
        DENOFIZ_BLEND_OP_SUBTRACT,         /**< Result = Src - Dst */
        DENOFIZ_BLEND_OP_REVERSE_SUBTRACT, /**< Result = Dst - Src */
        DENOFIZ_BLEND_OP_MIN,              /**< Result = min(Src, Dst) */
        DENOFIZ_BLEND_OP_MAX               /**< Result = max(Src, Dst) */
    } DenOfIz_BlendOp;

    /**
     * @brief Logic operation for blending (when BlendLogicOpEnable is true).
     */
    typedef enum DenOfIz_LogicOp
    {
        DENOFIZ_LOGIC_OP_CLEAR,
        DENOFIZ_LOGIC_OP_SET,
        DENOFIZ_LOGIC_OP_COPY,
        DENOFIZ_LOGIC_OP_COPY_INVERTED,
        DENOFIZ_LOGIC_OP_NO_OP,
        DENOFIZ_LOGIC_OP_INVERT,
        DENOFIZ_LOGIC_OP_AND,
        DENOFIZ_LOGIC_OP_NAND,
        DENOFIZ_LOGIC_OP_OR,
        DENOFIZ_LOGIC_OP_NOR,
        DENOFIZ_LOGIC_OP_XOR,
        DENOFIZ_LOGIC_OP_EQUIV,
        DENOFIZ_LOGIC_OP_AND_REVERSE,
        DENOFIZ_LOGIC_OP_AND_INVERTED,
        DENOFIZ_LOGIC_OP_OR_REVERSE,
        DENOFIZ_LOGIC_OP_OR_INVERTED
    } DenOfIz_LogicOp;

    /**
     * @brief Blend state configuration for a render target.
     *
     * For standard alpha blending, set Enable=true, SrcBlend=SRC_ALPHA,
     * DstBlend=INV_SRC_ALPHA, BlendOp=ADD.
     */
    typedef struct DenOfIz_BlendDesc
    {
        bool            Enable;                /**< Enable blending for this target */
        DenOfIz_Blend   SrcBlend;              /**< Source color blend factor */
        DenOfIz_Blend   DstBlend;              /**< Destination color blend factor */
        DenOfIz_Blend   SrcBlendAlpha;         /**< Source alpha blend factor */
        DenOfIz_Blend   DstBlendAlpha;         /**< Destination alpha blend factor */
        DenOfIz_BlendOp BlendOp;               /**< Color blend operation */
        DenOfIz_BlendOp BlendOpAlpha;          /**< Alpha blend operation */
        uint8_t         RenderTargetWriteMask; /**< RGBA write mask (0x0F = all channels) */
    } DenOfIz_BlendDesc;

    /**
     * @brief Render target description including format and blend state.
     */
    typedef struct DenOfIz_RenderTargetDesc
    {
        DenOfIz_BlendDesc Blend;  /**< Blend state for this target */
        DenOfIz_Format    Format; /**< Texture format of the render target */
    } DenOfIz_RenderTargetDesc;

    /**
     * @brief Array of render target descriptions.
     */
    typedef struct DenOfIz_RenderTargetDescArray
    {
        DenOfIz_RenderTargetDesc *Elements;    /**< Pointer to array of render target descs */
        uint32_t                  NumElements; /**< Number of render targets (max typically 8) */
    } DenOfIz_RenderTargetDescArray;

    /**
     * @brief Depth test configuration.
     */
    typedef struct DenOfIz_DepthTest
    {
        bool              Enable;    /**< Enable depth testing */
        DenOfIz_CompareOp CompareOp; /**< Comparison function for depth test */
        bool              Write;     /**< Enable depth buffer writes */
    } DenOfIz_DepthTest;

    /**
     * @brief Stencil operations for one face (front or back).
     */
    typedef struct DenOfIz_StencilFace
    {
        DenOfIz_CompareOp CompareOp;   /**< Comparison function for stencil test */
        DenOfIz_StencilOp FailOp;      /**< Operation when stencil test fails */
        DenOfIz_StencilOp PassOp;      /**< Operation when stencil and depth pass */
        DenOfIz_StencilOp DepthFailOp; /**< Operation when stencil passes but depth fails */
    } DenOfIz_StencilFace;

    /**
     * @brief Stencil test configuration.
     */
    typedef struct DenOfIz_StencilTest
    {
        bool                Enable;    /**< Enable stencil testing */
        uint32_t            WriteMask; /**< Stencil write mask */
        uint32_t            ReadMask;  /**< Stencil read mask */
        DenOfIz_StencilFace FrontFace; /**< Stencil operations for front-facing triangles */
        DenOfIz_StencilFace BackFace;  /**< Stencil operations for back-facing triangles */
    } DenOfIz_StencilTest;

    /**
     * @brief Rasterization configuration including depth bias settings.
     *
     * Depth bias is used to offset fragment depth values, commonly to prevent
     * z-fighting for coplanar geometry (e.g., shadow mapping, decals).
     *
     * The final depth bias is calculated as:
     * bias = (float)DepthBias * r + SlopeScaledDepthBias * maxSlope
     * where r is the minimum representable value in the depth buffer format,
     * and maxSlope is the maximum depth slope of the triangle.
     */
    typedef struct DenOfIz_RasterizationDesc
    {
        int32_t DepthBias;             /**< Constant depth offset (scaled by format minimum value) */
        float   DepthBiasClamp;        /**< Maximum (or minimum) depth bias of a pixel */
        float   SlopeScaledDepthBias;  /**< Scalar that scales the fragment's depth slope */
        bool    FrontCounterClockwise; /**< Front-facing triangles are counter-clockwise wound */
    } DenOfIz_RasterizationDesc;

    /**
     * @brief Graphics pipeline configuration.
     *
     * Used when BindPoint is DENOFIZ_BIND_POINT_GRAPHICS or DENOFIZ_BIND_POINT_MESH.
     * Configures rasterization, blending, depth/stencil testing, and render targets.
     */
    typedef struct DenOfIz_GraphicsPipelineDesc
    {
        DenOfIz_ViewMask              ViewMask;                     /**< Multiview rendering mask (VR) */
        bool                          AlphaToCoverageEnable;        /**< Enable alpha-to-coverage for MSAA */
        bool                          IndependentBlendEnable;       /**< Allow different blend per RT */
        bool                          BlendLogicOpEnable;           /**< Enable logic operations */
        DenOfIz_LogicOp               BlendLogicOp;                 /**< Logic op when BlendLogicOpEnable */
        DenOfIz_RenderTargetDescArray RenderTargets;                /**< Render target configurations */
        DenOfIz_Format                DepthStencilAttachmentFormat; /**< Depth buffer format, or UNDEFINED */

        DenOfIz_PrimitiveTopology PrimitiveTopology; /**< Primitive assembly type */
        DenOfIz_CullMode          CullMode;          /**< Face culling mode */
        DenOfIz_FillMode          FillMode;          /**< Polygon fill mode */
        DenOfIz_RasterizationDesc Rasterization;     /**< Rasterization and depth bias settings */
        DenOfIz_DepthTest         DepthTest;         /**< Depth test configuration */
        DenOfIz_StencilTest       StencilTest;       /**< Stencil test configuration */
        DenOfIz_MSAASampleCount   MSAASampleCount;   /**< MSAA sample count */
    } DenOfIz_GraphicsPipelineDesc;

    /**
     * @brief Hit group configuration for ray tracing pipelines.
     *
     * A hit group defines the shaders invoked when a ray hits geometry. Each group
     * may contain a closest-hit shader (required for triangles), an any-hit shader
     * (optional, for transparency), and an intersection shader (for procedural geometry).
     *
     * @warning Shader indices must be explicitly set to -1 when not used. Since C
     * structs are zero-initialized by default and 0 is a valid shader index, failing
     * to set unused indices to -1 will cause incorrect shader binding.
     */
    typedef struct DenOfIz_HitGroupDesc
    {
        DenOfIz_StringView         Name;                    /**< Hit group name for SBT reference */
        int32_t                    IntersectionShaderIndex; /**< Index in ShaderProgram, or -1 if not used */
        int32_t                    AnyHitShaderIndex;       /**< Index in ShaderProgram, or -1 if not used */
        int32_t                    ClosestHitShaderIndex;   /**< Index in ShaderProgram, or -1 if not used */
        DenOfIz_LocalRootSignature LocalRootSignature;      /**< Per-hit-group local root signature */
        DenOfIz_HitGroupType       Type;                    /**< Triangle or procedural geometry */
    } DenOfIz_HitGroupDesc;

    /**
     * @brief Array of hit group descriptions.
     */
    typedef struct DenOfIz_HitGroupDescArray
    {
        DenOfIz_HitGroupDesc *Elements;
        uint32_t              NumElements;
    } DenOfIz_HitGroupDescArray;

    /**
     * @brief Ray tracing pipeline configuration.
     *
     * Used when BindPoint is DENOFIZ_BIND_POINT_RAYTRACING. Defines hit groups
     * and local root signatures for ray tracing shader binding table construction.
     */
    typedef struct DenOfIz_RayTracingPipelineDesc
    {
        DenOfIz_HitGroupDescArray       HitGroups;           /**< Hit groups for geometry intersection */
        DenOfIz_LocalRootSignatureArray LocalRootSignatures; /**< Local root signatures for hit groups */
    } DenOfIz_RayTracingPipelineDesc;

    /**
     * @brief Complete pipeline descriptor.
     *
     * @par Valid Usage
     * - @p BindPoint determines which configuration (Graphics or RayTracing) is used
     * - @p RootSignature must be valid and compatible with the ShaderProgram
     * - @p ShaderProgram must be valid and compiled
     * - For DENOFIZ_BIND_POINT_GRAPHICS: InputLayout required for vertex input
     * - For DENOFIZ_BIND_POINT_COMPUTE: Only RootSignature and ShaderProgram needed
     * - For DENOFIZ_BIND_POINT_RAYTRACING: RayTracing.HitGroups must be configured
     */
    typedef struct DenOfIz_PipelineDesc
    {
        DenOfIz_BindPoint     BindPoint;     /**< Pipeline type (graphics/compute/raytracing/mesh) */
        DenOfIz_InputLayout   InputLayout;   /**< Vertex input layout (graphics only) */
        DenOfIz_RootSignature RootSignature; /**< Resource binding layout (required) */
        DenOfIz_ShaderProgram ShaderProgram; /**< Compiled shader program (required) */
        DenOfIz_PipelineCache PipelineCache; /**< Optional cache for faster creation */

        DenOfIz_GraphicsPipelineDesc   Graphics;   /**< Graphics pipeline config */
        DenOfIz_RayTracingPipelineDesc RayTracing; /**< Ray tracing pipeline config */
    } DenOfIz_PipelineDesc;

    /**
     * @brief Destroys a pipeline and releases its resources.
     *
     * @param pipeline Pipeline handle to destroy.
     *
     * @par Valid Usage
     * - @p pipeline must be a valid DenOfIz_Pipeline handle or DENOFIZ_NULL_HANDLE
     * - The pipeline must not be in use by a pending command list
     */
    DZ_API void DenOfIz_Pipeline_Destroy( DenOfIz_Pipeline pipeline );

#ifdef __cplusplus
}
#endif
