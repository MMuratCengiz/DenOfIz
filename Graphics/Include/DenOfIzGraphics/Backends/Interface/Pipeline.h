/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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

    DENOFIZ_DEFINE_HANDLE( DenOfIz_Pipeline )

    typedef enum DenOfIz_BindPoint
    {
        DENOFIZ_BIND_POINT_GRAPHICS,
        DENOFIZ_BIND_POINT_COMPUTE,
        DENOFIZ_BIND_POINT_RAYTRACING,
        DENOFIZ_BIND_POINT_MESH
    } DenOfIz_BindPoint;

    typedef enum DenOfIz_BlendMode
    {
        DENOFIZ_BLEND_MODE_NONE,
        DENOFIZ_BLEND_MODE_ALPHA_BLEND
    } DenOfIz_BlendMode;

    typedef enum DenOfIz_CullMode
    {
        DENOFIZ_CULL_MODE_NONE,
        DENOFIZ_CULL_MODE_BACK_FACE,
        DENOFIZ_CULL_MODE_FRONT_FACE
    } DenOfIz_CullMode;

    typedef enum DenOfIz_FillMode
    {
        DENOFIZ_FILL_MODE_SOLID,
        DENOFIZ_FILL_MODE_WIREFRAME
    } DenOfIz_FillMode;

    typedef enum DenOfIz_Blend
    {
        DENOFIZ_BLEND_ZERO,
        DENOFIZ_BLEND_ONE,
        DENOFIZ_BLEND_SRC_COLOR,
        DENOFIZ_BLEND_INV_SRC_COLOR,
        DENOFIZ_BLEND_SRC_ALPHA,
        DENOFIZ_BLEND_INV_SRC_ALPHA,
        DENOFIZ_BLEND_DST_ALPHA,
        DENOFIZ_BLEND_INV_DST_ALPHA,
        DENOFIZ_BLEND_DST_COLOR,
        DENOFIZ_BLEND_INV_DST_COLOR,
        DENOFIZ_BLEND_SRC_ALPHA_SATURATE,
        DENOFIZ_BLEND_BLEND_FACTOR,
        DENOFIZ_BLEND_INV_BLEND_FACTOR,
        DENOFIZ_BLEND_SRC1_COLOR,
        DENOFIZ_BLEND_INV_SRC1_COLOR,
        DENOFIZ_BLEND_SRC1_ALPHA,
        DENOFIZ_BLEND_INV_SRC1_ALPHA
    } DenOfIz_Blend;

    typedef enum DenOfIz_BlendOp
    {
        DENOFIZ_BLEND_OP_ADD,
        DENOFIZ_BLEND_OP_SUBTRACT,
        DENOFIZ_BLEND_OP_REVERSE_SUBTRACT,
        DENOFIZ_BLEND_OP_MIN,
        DENOFIZ_BLEND_OP_MAX
    } DenOfIz_BlendOp;

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

    typedef struct DenOfIz_BlendDesc
    {
        bool            Enable;
        DenOfIz_Blend   SrcBlend;
        DenOfIz_Blend   DstBlend;
        DenOfIz_Blend   SrcBlendAlpha;
        DenOfIz_Blend   DstBlendAlpha;
        DenOfIz_BlendOp BlendOp;
        DenOfIz_BlendOp BlendOpAlpha;
        uint8_t         RenderTargetWriteMask; /**< Bit 0 R, 1 G, 2 B, 3 A. Zero writes nothing, 0x0F every channel. */
    } DenOfIz_BlendDesc;

    typedef struct DenOfIz_RenderTargetDesc
    {
        DenOfIz_BlendDesc Blend;
        DenOfIz_Format    Format;
    } DenOfIz_RenderTargetDesc;

    typedef struct DenOfIz_RenderTargetDescArray
    {
        DenOfIz_RenderTargetDesc *Elements;
        uint32_t                  NumElements;
    } DenOfIz_RenderTargetDescArray;

    typedef struct DenOfIz_DepthTest
    {
        bool              Enable;
        DenOfIz_CompareOp CompareOp;
        bool              Write;
    } DenOfIz_DepthTest;

    typedef struct DenOfIz_StencilFace
    {
        DenOfIz_CompareOp CompareOp;
        DenOfIz_StencilOp FailOp;
        DenOfIz_StencilOp PassOp;
        DenOfIz_StencilOp DepthFailOp;
    } DenOfIz_StencilFace;

    /** The stencil reference value is always 0. */
    typedef struct DenOfIz_StencilTest
    {
        bool                Enable;
        uint32_t            WriteMask;
        uint32_t            ReadMask;
        DenOfIz_StencilFace FrontFace;
        DenOfIz_StencilFace BackFace;
    } DenOfIz_StencilTest;

    /** Depth bias = DepthBias * r + SlopeScaledDepthBias * maxSlope, clamped to DepthBiasClamp, r being the depth format's minimum resolvable difference. */
    typedef struct DenOfIz_RasterizationDesc
    {
        int32_t DepthBias;
        float   DepthBiasClamp;
        float   SlopeScaledDepthBias;
        bool    FrontCounterClockwise; /**< False: clockwise triangles are front facing. */
    } DenOfIz_RasterizationDesc;

    /** For GRAPHICS and MESH pipelines. */
    typedef struct DenOfIz_GraphicsPipelineDesc
    {
        DenOfIz_ViewMask              ViewMask; /**< Vulkan multiview only. */
        bool                          AlphaToCoverageEnable;
        bool                          IndependentBlendEnable; /**< Read by DX12. Vulkan always uses each render target's own Blend. */
        bool                          BlendLogicOpEnable;     /**< DX12 and Vulkan, Metal has no logic ops. */
        DenOfIz_LogicOp               BlendLogicOp;
        DenOfIz_RenderTargetDescArray RenderTargets;
        DenOfIz_Format                DepthStencilAttachmentFormat; /**< UNDEFINED is no depth attachment. */

        DenOfIz_PrimitiveTopology PrimitiveTopology;
        DenOfIz_CullMode          CullMode;
        DenOfIz_FillMode          FillMode;
        DenOfIz_RasterizationDesc Rasterization;
        DenOfIz_DepthTest         DepthTest;
        DenOfIz_StencilTest       StencilTest;
        DenOfIz_MSAASampleCount   MSAASampleCount;
    } DenOfIz_GraphicsPipelineDesc;

    /**
     * Shader indices index the ShaderProgram's stages, -1 is none. Set unused ones to -1 explicitly, zero is a valid index. Name is
     * required, shader binding tables refer to the group by it. AABBS groups require an intersection shader.
     */
    typedef struct DenOfIz_HitGroupDesc
    {
        DenOfIz_StringView         Name;
        int32_t                    IntersectionShaderIndex;
        int32_t                    AnyHitShaderIndex;
        int32_t                    ClosestHitShaderIndex;
        DenOfIz_LocalRootSignature LocalRootSignature; /**< Optional. */
        DenOfIz_HitGroupType       Type;
    } DenOfIz_HitGroupDesc;

    typedef struct DenOfIz_HitGroupDescArray
    {
        DenOfIz_HitGroupDesc *Elements;
        uint32_t              NumElements;
    } DenOfIz_HitGroupDescArray;

    typedef struct DenOfIz_RayTracingPipelineDesc
    {
        DenOfIz_HitGroupDescArray       HitGroups;
        DenOfIz_LocalRootSignatureArray LocalRootSignatures; /**< Indexed like the ShaderProgram's stages, a null handle for a stage without one. */
    } DenOfIz_RayTracingPipelineDesc;

    typedef struct DenOfIz_PipelineDesc
    {
        DenOfIz_BindPoint     BindPoint;
        DenOfIz_InputLayout   InputLayout;   /**< Optional, read at creation. Ignored by ray tracing pipelines. */
        DenOfIz_RootSignature RootSignature; /**< Required. The pipeline keeps using it, it must outlive the pipeline. */
        DenOfIz_ShaderProgram ShaderProgram; /**< Required. */
        DenOfIz_PipelineCache PipelineCache; /**< Optional. */

        DenOfIz_GraphicsPipelineDesc   Graphics;
        DenOfIz_RayTracingPipelineDesc RayTracing;
    } DenOfIz_PipelineDesc;

    /** Release is deferred until the GPU is done with it, see DenOfIz_LogicalDevice_ReleaseDestroyedResources. */
    DZ_API void DenOfIz_Pipeline_Destroy( DenOfIz_Pipeline pipeline );

#ifdef __cplusplus
}
#endif
