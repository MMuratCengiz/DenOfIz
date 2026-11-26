/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <DenOfIzGraphicsInternal/Backends/Interface/IPipeline.h>
#include <deque>
#include <memory>
#include "MetalContext.h"
#include "MetalInputLayout.h"
#include "MetalRootSignature.h"
#include "MetalShaderLayout.h"

namespace DenOfIz
{
    struct HitGroupExport
    {
        uint64_t ClosestHit   = 0;
        uint64_t AnyHit       = 0;
        uint64_t Intersection = 0;
    };

    struct MeshShaderUsedStages
    {
        bool Task  = false;
        bool Mesh  = true;
        bool Pixel = false;
    };

    class MetalPipeline final : public IPipeline
    {
        MetalContext        *m_context;
        MetalRootSignature  *m_rootSignature;
        DenOfIz_PipelineDesc m_desc;

        std::unique_ptr<MetalShaderLayout> m_shaderLayout;
        std::unique_ptr<MetalShaderLayout> m_localShaderLayout;

        id<MTLRenderPipelineState>  m_graphicsPipelineState;
        id<MTLComputePipelineState> m_computePipelineState;

        MTLCullMode              m_cullMode;
        MTLPrimitiveType         m_primitiveType;
        MTLTriangleFillMode      m_fillMode;
        MTLWinding               m_frontFaceWinding;
        id<MTLDepthStencilState> m_depthStencilState;

        float m_depthBias;
        float m_depthBiasSlopeScale;
        float m_depthBiasClamp;

        MTLSize              m_computeThreadsPerThreadgroup;
        MTLSize              m_meshThreadsPerThreadgroup;
        MTLSize              m_objectThreadsPerThreadgroup;
        MeshShaderUsedStages m_usedMeshShaderStages;

        std::unordered_map<std::string, uint64_t>       m_visibleFunctions;
        std::unordered_map<std::string, HitGroupExport> m_hitGroupExports;
        std::deque<std::string>                         m_hitGroupShaders;
        id<MTLVisibleFunctionTable>                     m_visibleFunctionTable;
        id<MTLIntersectionFunctionTable>                m_intersectionFunctionTable;

    public:
        MetalPipeline( MetalContext *context, const DenOfIz_PipelineDesc &desc );
        ~MetalPipeline( ) override;

        MetalRootSignature                *RootSignature( ) const;
        const MetalShaderLayout           &ShaderLayout( ) const;
        const MetalShaderLayout           &LocalShaderLayout( ) const;
        const MTLCullMode                 &CullMode( ) const;
        const MTLTriangleFillMode         &FillMode( ) const;
        const MTLWinding                  &FrontFaceWinding( ) const;
        const MTLPrimitiveType            &PrimitiveType( ) const;
        const id<MTLDepthStencilState>    &DepthStencilState( ) const;
        const id<MTLRenderPipelineState>  &GraphicsPipelineState( ) const;
        const id<MTLComputePipelineState> &ComputePipelineState( ) const;
        const DenOfIz_BindPoint           &DenOfIz_BindPoint( ) const;
        const DenOfIz_PipelineDesc        &Desc( ) const;
        float                              DepthBias( ) const;
        float                              DepthBiasSlopeScale( ) const;
        float                              DepthBiasClamp( ) const;
        const MTLSize                     &ComputeThreadsPerThreadgroup( ) const;
        const MTLSize                     &MeshThreadsPerThreadgroup( ) const;
        const MTLSize                     &ObjectThreadsPerThreadgroup( ) const;
        const MeshShaderUsedStages        &MeshShaderUsedStages( );

        uint64_t                                       FindVisibleShaderIndexByName( const std::string &name ) const;
        const HitGroupExport                          &FindHitGroupExport( const std::string &name ) const;
        [[nodiscard]] const id<MTLVisibleFunctionTable>      &VisibleFunctionTable( ) const;
        [[nodiscard]] const id<MTLIntersectionFunctionTable> &IntersectionFunctionTable( ) const;

    private:
        void CreateGraphicsPipeline( );
        void CreateComputePipeline( );
        void CreateRayTracingPipeline( );
        void CreateMeshPipeline( );
        void InitStencilFace( MTLStencilDescriptor *stencilDesc, const DenOfIz_StencilFace &stencilFace );

        id<MTLLibrary>  LoadLibrary( DenOfIz_ByteArray &blob );
        id<MTLFunction> CreateShaderFunction( id<MTLLibrary> library, const std::string &entryPoint );
        id<MTLLibrary>  NewIndirectDispatchLibrary( const DenOfIz_ShaderRayTracingDesc &rtDesc );
        id<MTLLibrary>  NewSynthesizedIntersectionLibrary( const IRHitGroupType &hitGroupType, const DenOfIz_ShaderRayTracingDesc &rtDesc );
    };
} // namespace DenOfIz
