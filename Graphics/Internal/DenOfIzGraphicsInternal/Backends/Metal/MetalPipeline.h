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

#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
#include "MetalContext.h"
#include "MetalInputLayout.h"
#include "MetalRootSignature.h"

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
        MetalContext       *m_context;
        MetalRootSignature *m_rootSignature;
        PipelineDesc        m_desc;

        id<MTLRenderPipelineState>  m_graphicsPipelineState;
        id<MTLComputePipelineState> m_computePipelineState;
        // Graphics specific
        MTLCullMode              m_cullMode;
        MTLTriangleFillMode      m_fillMode;
        id<MTLDepthStencilState> m_depthStencilState;
        // Thread group sizes (pre-calculated from shader reflection)
        MTLSize              m_computeThreadsPerThreadgroup; // For compute shaders
        MTLSize              m_meshThreadsPerThreadgroup;    // For mesh shaders
        MTLSize              m_objectThreadsPerThreadgroup;  // For task/amplification shaders
        MeshShaderUsedStages m_usedMeshShaderStages;
        // Ray tracing specific
        std::unordered_map<std::string, uint64_t>       m_visibleFunctions;
        std::unordered_map<std::string, HitGroupExport> m_hitGroupExports;
        std::vector<std::string>                        m_hitGroupShaders;
        id<MTLVisibleFunctionTable>                     m_visibleFunctionTable;
        id<MTLIntersectionFunctionTable>                m_intersectionFunctionTable;

    public:
        MetalPipeline( MetalContext *context, const PipelineDesc &desc );
        ~MetalPipeline( ) override;

        MetalRootSignature                *RootSignature( ) const;
        const MTLCullMode                 &CullMode( ) const;
        const MTLTriangleFillMode         &FillMode( ) const;
        const id<MTLDepthStencilState>    &DepthStencilState( ) const;
        const id<MTLRenderPipelineState>  &GraphicsPipelineState( ) const;
        const id<MTLComputePipelineState> &ComputePipelineState( ) const;
        const BindPoint                   &BindPoint( ) const;
        const PipelineDesc                &Desc( ) const;
        const MTLSize                     &ComputeThreadsPerThreadgroup( ) const;
        const MTLSize                     &MeshThreadsPerThreadgroup( ) const;
        const MTLSize                     &ObjectThreadsPerThreadgroup( ) const;
        const MeshShaderUsedStages        &MeshShaderUsedStages( );

        // Ray tracing specific:
        const uint64_t                                &FindVisibleShaderIndexByName( const std::string &name ) const;
        const HitGroupExport                          &FindHitGroupExport( const std::string &name ) const;
        [[nodiscard]] id<MTLVisibleFunctionTable>      VisibleFunctionTable( ) const;
        [[nodiscard]] id<MTLIntersectionFunctionTable> IntersectionFunctionTable( ) const;

    private:
        void CreateGraphicsPipeline( );
        void CreateComputePipeline( );
        void CreateRayTracingPipeline( );
        void CreateMeshPipeline( );
        void InitStencilFace( MTLStencilDescriptor *stencilDesc, const StencilFace &stencilFace );

        id<MTLLibrary>  LoadLibrary( ByteArray &blob );
        id<MTLFunction> CreateShaderFunction( id<MTLLibrary> library, const std::string &entryPoint );
        id<MTLLibrary>  NewIndirectDispatchLibrary( );
        id<MTLLibrary>  NewSynthesizedIntersectionLibrary( const IRHitGroupType &hitGroupType );
    };

} // namespace DenOfIz
