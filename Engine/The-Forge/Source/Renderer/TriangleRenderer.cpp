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

#include <TF/Renderer/TriangleRenderer.h>

using namespace DenOfIz;

void TriangleRenderer::Update(float deltaTime) {}

void TriangleRenderer::Draw()
{
    m_time->Tick();

    uint32_t swapchainImageIndex = TFCommon::AcquireNextImage();
    RenderTarget *pRenderTarget = p_SwapChain->ppRenderTargets[ swapchainImageIndex ];
    GpuCmdRingElement elem = TFCommon::NextCmdRingElement();

    float timePassed = (m_time->DoubleEpochNow() - m_time->GetFirstTickTime()) / 1000000.0f;

    BufferUpdateDesc deltaTimeUpdateDesc = { mp_deltaTimeBuffer[ m_FrameIndex ] };
    beginUpdateResource(&deltaTimeUpdateDesc);
    memcpy(deltaTimeUpdateDesc.pMappedData, &timePassed, sizeof(float));
    endUpdateResource(&deltaTimeUpdateDesc);

    Cmd *cmd = elem.pCmds[ 0 ];
    beginCmd(cmd);

    RenderTargetBarrier barriers[] = {
        { pRenderTarget, RESOURCE_STATE_PRESENT, RESOURCE_STATE_RENDER_TARGET },
    };
    cmdResourceBarrier(cmd, 0, NULL, 0, NULL, 1, barriers);

    BindRenderTargetsDesc bindRenderTargets = {};
    bindRenderTargets.mRenderTargetCount = 1;
    bindRenderTargets.mRenderTargets[ 0 ] = { pRenderTarget, LOAD_ACTION_CLEAR };
    cmdBindRenderTargets(cmd, &bindRenderTargets);
    cmdSetViewport(cmd, 0.0f, 0.0f, (float)pRenderTarget->mWidth, (float)pRenderTarget->mHeight, 0.0f, 1.0f);
    cmdSetScissor(cmd, 0, 0, pRenderTarget->mWidth, pRenderTarget->mHeight);
    cmdBindPipeline(cmd, mp_pipeline);
    cmdBindDescriptorSet(cmd, 0, mp_descriptorSet);
    cmdBindVertexBuffer(cmd, 1, &mp_vertexBuffer, 0, NULL);
    cmdBindIndexBuffer(cmd, mp_indexBuffer, INDEX_TYPE_UINT16, 0);
    cmdDrawIndexed(cmd, 3, 0, 0);
    cmdBindRenderTargets(cmd, NULL);

    barriers[ 0 ] = { pRenderTarget, RESOURCE_STATE_RENDER_TARGET, RESOURCE_STATE_PRESENT };
    cmdResourceBarrier(cmd, 0, NULL, 0, NULL, 1, barriers);

    endCmd(cmd);
    FlushResourceUpdateDesc flushUpdateDesc = {};
    flushUpdateDesc.mNodeIndex = 0;
    flushResourceUpdates(&flushUpdateDesc);
    Semaphore *waitSemaphores[ 2 ] = { flushUpdateDesc.pOutSubmittedSemaphore, p_ImageAcquiredSemaphore };

    QueueSubmitDesc submitDesc = {};
    submitDesc.mCmdCount = 1;
    submitDesc.mSignalSemaphoreCount = 1;
    submitDesc.mWaitSemaphoreCount = TF_ARRAY_COUNT(waitSemaphores);
    submitDesc.ppCmds = &cmd;
    submitDesc.ppSignalSemaphores = &elem.pSemaphore;
    submitDesc.ppWaitSemaphores = waitSemaphores;
    submitDesc.pSignalFence = elem.pFence;
    queueSubmit(p_GraphicsQueue, &submitDesc);
    TFCommon::Present({ elem.pSemaphore }, swapchainImageIndex);
}

bool TriangleRenderer::Init()
{
    if ( !TFCommon::Init() )
    {
        return false;
    }

    BufferLoadDesc ubDesc = {};
    ubDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    ubDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_CPU_TO_GPU;
    ubDesc.mDesc.mFlags = BUFFER_CREATION_FLAG_PERSISTENT_MAP_BIT;
    ubDesc.pData = NULL;

    for ( uint32_t i = 0; i < g_DataBufferCount; ++i )
    {
        ubDesc.mDesc.pName = "deltaTime";
        ubDesc.mDesc.mSize = sizeof(float);
        ubDesc.ppBuffer = &mp_deltaTimeBuffer[ i ];
        addResource(&ubDesc, NULL);
    }

    BufferLoadDesc vbDesc = {};
    vbDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_VERTEX_BUFFER;
    vbDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
    vbDesc.mDesc.mSize = m_triangle.size() * sizeof(float);
    vbDesc.pData = m_triangle.data();
    vbDesc.ppBuffer = &mp_vertexBuffer;
    addResource(&vbDesc, nullptr);

    BufferLoadDesc sphereIbDesc = {};
    sphereIbDesc.mDesc.mDescriptors = DESCRIPTOR_TYPE_INDEX_BUFFER;
    sphereIbDesc.mDesc.mMemoryUsage = RESOURCE_MEMORY_USAGE_GPU_ONLY;
    sphereIbDesc.mDesc.mSize = m_indices.size() * sizeof(uint32_t);
    sphereIbDesc.pData = m_indices.data();
    sphereIbDesc.ppBuffer = &mp_indexBuffer;
    addResource(&sphereIbDesc, nullptr);

    m_time->ListenFps = [](const double fps) { std::cout << "FPS: " << fps << "\n"; };
    return true;
}

void TriangleRenderer::Exit()
{
    removeResource(mp_vertexBuffer);
    removeResource(mp_indexBuffer);
    TFCommon::Exit();
}

bool TriangleRenderer::Load(ReloadDesc *pReloadDesc)
{
    if ( !TFCommon::Load(pReloadDesc) )
    {
        return false;
    }

    if ( pReloadDesc->mType & RELOAD_TYPE_SHADER )
    {
        ShaderLoadDesc basicShader = {};
        basicShader.mStages[ 0 ].pFileName = "basic.vert";
        basicShader.mStages[ 1 ].pFileName = "basic.frag";
        addShader(p_Renderer, &basicShader, &mp_basicShader);

        RootSignatureDesc rootDesc = {};
        rootDesc.mShaderCount = 1;
        rootDesc.ppShaders = &mp_basicShader;
        addRootSignature(p_Renderer, &rootDesc, &mp_rootSignature);

        DescriptorSetDesc desc = { mp_rootSignature, DESCRIPTOR_UPDATE_FREQ_PER_FRAME, g_DataBufferCount };
        addDescriptorSet(p_Renderer, &desc, &mp_descriptorSet);
    }

    if ( pReloadDesc->mType & (RELOAD_TYPE_SHADER | RELOAD_TYPE_RENDERTARGET) )
    {
        RasterizerStateDesc rasterizerStateDesc = {};
        rasterizerStateDesc.mCullMode = CULL_MODE_NONE;

        VertexLayout vertexLayout = {};
        vertexLayout.mBindingCount = 1;
        vertexLayout.mBindings[ 0 ].mStride = 2 * sizeof(float4);
        vertexLayout.mAttribCount = 2;

        vertexLayout.mAttribs[ 0 ].mSemantic = SEMANTIC_POSITION;
        vertexLayout.mAttribs[ 0 ].mFormat = TinyImageFormat_R32G32B32A32_SFLOAT;
        vertexLayout.mAttribs[ 0 ].mBinding = 0;
        vertexLayout.mAttribs[ 0 ].mLocation = 0;
        vertexLayout.mAttribs[ 0 ].mOffset = 0;

        vertexLayout.mAttribs[ 1 ].mSemantic = SEMANTIC_COLOR;
        vertexLayout.mAttribs[ 1 ].mFormat = TinyImageFormat_R32G32B32A32_SFLOAT;
        vertexLayout.mAttribs[ 1 ].mBinding = 0;
        vertexLayout.mAttribs[ 1 ].mLocation = 1;
        vertexLayout.mAttribs[ 1 ].mOffset = 4 * sizeof(float);

        PipelineDesc desc = {};
        desc.mType = PIPELINE_TYPE_GRAPHICS;
        GraphicsPipelineDesc &pipelineSettings = desc.mGraphicsDesc;
        pipelineSettings.mPrimitiveTopo = PRIMITIVE_TOPO_TRI_LIST;
        pipelineSettings.mRenderTargetCount = 1;
        pipelineSettings.pColorFormats = &p_SwapChain->ppRenderTargets[ 0 ]->mFormat;
        pipelineSettings.mSampleCount = p_SwapChain->ppRenderTargets[ 0 ]->mSampleCount;
        pipelineSettings.mSampleQuality = p_SwapChain->ppRenderTargets[ 0 ]->mSampleQuality;
        pipelineSettings.pRootSignature = mp_rootSignature;
        pipelineSettings.pShaderProgram = mp_basicShader;
        pipelineSettings.pVertexLayout = &vertexLayout;
        pipelineSettings.pRasterizerState = &rasterizerStateDesc;
        addPipeline(p_Renderer, &desc, &mp_pipeline);
    }

    for ( uint32_t i = 0; i < g_DataBufferCount; ++i )
    {
        DescriptorData params[ 1 ] = {};
        params[ 0 ].pName = "uniformBlock";
        params[ 0 ].ppBuffers = &mp_deltaTimeBuffer[ i ];
        updateDescriptorSet(p_Renderer, i * 2 + 1, mp_descriptorSet, 1, params);
    }

    return true;
}

void TriangleRenderer::Unload(ReloadDesc *pReloadDesc)
{
    TFCommon::Unload(pReloadDesc);

    if ( pReloadDesc->mType & RELOAD_TYPE_SHADER )
    {
        removeShader(p_Renderer, mp_basicShader);
        removeRootSignature(p_Renderer, mp_rootSignature);
        removeDescriptorSet(p_Renderer, mp_descriptorSet);
    }

    if ( pReloadDesc->mType & (RELOAD_TYPE_SHADER | RELOAD_TYPE_RENDERTARGET) )
    {
        removePipeline(p_Renderer, mp_pipeline);
    }
}
