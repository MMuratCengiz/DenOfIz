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

#include "TF/Pipelines/QuadPipeline.h"

using namespace DenOfIz;

QuadPipeline::QuadPipeline(TFCommon *common) : m_common(common)
{
    ShaderLoadDesc shaderDesc{
        .mStages{
            {
                .pFileName = "quad.vert",
            },
            {
                .pFileName = "basic.frag",
            },
        },
    };

    addShader(common->p_Renderer, &shaderDesc, &m_program);
    ASSERT(m_program);

    //	PipelineDesc desc = {
    //			.mGraphicsDesc =
    //					{
    //							.pShaderProgram = m_program,
    //							.pRootSignature = pRSSingle,
    //							.pVertexLayout = &vertexLayout,
    //							.pDepthState = &depthStateDesc,
    //							.pRasterizerState = &sphereRasterizerStateDesc,
    //							.pColorFormats = &pRenderTarget->mFormat,
    //							.mRenderTargetCount = 1,
    //							.mSampleCount = pRenderTarget->mSampleCount,
    //							.mSampleQuality = p_RenderTarget->mSampleQuality,
    //							.mDepthStencilFormat = depthBufferFormat,
    //							.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST
    //					},
    //			.mType = PIPELINE_TYPE_GRAPHICS,
    //	};
    //
    //	addPipeline(m_common->p_Renderer, &desc, &p_PipelineQuad);
    ASSERT(p_PipelineQuad);
}

QuadPipeline::~QuadPipeline()
{
    removeShader(m_common->p_Renderer, m_program);
    removePipeline(m_common->p_Renderer, p_PipelineQuad);
}
