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

    class MetalPipeline : public IPipeline
    {
    private:
        MetalContext *m_context;
        PipelineDesc  m_desc;

        id<MTLDepthStencilState>    m_depthStencilState;
        id<MTLRenderPipelineState>  m_graphicsPipelineState;
        id<MTLComputePipelineState> m_computePipelineState;
        MTLCullMode                 m_cullMode;

    public:
        MetalPipeline( MetalContext *context, const PipelineDesc &desc );
        ~MetalPipeline( ) override;

        const MTLCullMode& CullMode( ) const
        {
            return m_cullMode;
        }

        const id<MTLDepthStencilState> &DepthStencilState( ) const
        {
            return m_depthStencilState;
        }

        const id<MTLRenderPipelineState> &GraphicsPipelineState( ) const
        {
            return m_graphicsPipelineState;
        }

        const id<MTLComputePipelineState> &ComputePipelineState( ) const
        {
            return m_computePipelineState;
        }

    private:
        void CreateGraphicsPipeline( );
        void CreateComputePipeline( );
        void CreateRayTracingPipeline( );
        void InitStencilFace( MTLStencilDescriptor *stencilDesc, const StencilFace &stencilFace );

        id<MTLFunction> CreateShaderFunction( IDxcBlob *&blob, const std::string &entryPoint );
    };

} // namespace DenOfIz
