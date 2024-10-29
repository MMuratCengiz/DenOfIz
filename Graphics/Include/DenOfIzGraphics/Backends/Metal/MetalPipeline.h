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

    struct ShaderFunction
    {
        uint32_t              Index;
        id<MTLFunction>       Function;
        id<MTLFunctionHandle> Handle;
    };

    struct IntersectionExport
    {
        ShaderFunction ClosestHit;
        bool           HasAnyHit = false;
        ShaderFunction AnyHit;
        bool           HasIntersection = false;
        ShaderFunction Intersection;
    };
    class MetalPipeline final : public IPipeline
    {
    private:
        MetalContext *m_context;
        PipelineDesc  m_desc;

        id<MTLRenderPipelineState>  m_graphicsPipelineState;
        id<MTLComputePipelineState> m_computePipelineState;
        // Graphics specific
        MTLCullMode              m_cullMode;
        id<MTLDepthStencilState> m_depthStencilState;
        // Ray tracing specific
        std::unordered_map<std::string, ShaderFunction> m_visibleFunctions;
        IntersectionExport                              m_intersectionExport;
        std::vector<std::string>                        m_hitGroupShaders;
        id<MTLVisibleFunctionTable>                     m_visibleFunctionTable;
        id<MTLIntersectionFunctionTable>                m_intersectionFunctionTable;

    public:
        MetalPipeline( MetalContext *context, const PipelineDesc &desc );
        ~MetalPipeline( ) override;

        const MTLCullMode                 &CullMode( ) const;
        const id<MTLDepthStencilState>    &DepthStencilState( ) const;
        const id<MTLRenderPipelineState>  &GraphicsPipelineState( ) const;
        const id<MTLComputePipelineState> &ComputePipelineState( ) const;
        // Ray tracing specific:
        const ShaderFunction                          &FindVisibleShaderFunctionByName( const std::string &name ) const;
        [[nodiscard]] id<MTLVisibleFunctionTable>      VisibleFunctionTable( ) const;
        [[nodiscard]] id<MTLIntersectionFunctionTable> IntersectionFunctionTable( ) const;
        [[nodiscard]] const IntersectionExport        &IntersectionExport( ) const;

    private:
        void CreateGraphicsPipeline( );
        void CreateComputePipeline( );
        void CreateRayTracingPipeline( );
        void InitStencilFace( MTLStencilDescriptor *stencilDesc, const StencilFace &stencilFace );

        id<MTLLibrary>  LoadLibrary( IDxcBlob *&blob, const std::string &shaderPath );
        id<MTLFunction> CreateShaderFunction( id<MTLLibrary> library, const std::string &entryPoint );
        id<MTLLibrary>  NewIndirectDispatchLibrary( );
    };

} // namespace DenOfIz
