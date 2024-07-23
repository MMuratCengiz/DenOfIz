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

#include <DenOfIzGraphics/Renderer/SimpleRenderer.h>

namespace DenOfIz
{

    void SimpleRenderer::Init(GraphicsWindowHandle *window)
    {
        m_window = window;
        GraphicsAPI::SetAPIPreference(APIPreference{
            //            .Windows = APIPreferenceWindows::Vulkan,
        });

        m_logicalDevice  = GraphicsAPI::CreateLogicalDevice(m_window);
        auto firstDevice = m_logicalDevice->ListPhysicalDevices()[ 0 ];
        m_logicalDevice->LoadPhysicalDevice(firstDevice);
        m_batchResourceCopy = std::make_unique<BatchResourceCopy>(m_logicalDevice.get());

        m_program       = std::make_unique<ShaderProgram>(ShaderProgramDesc{ .Shaders = { ShaderDesc{ .Stage = ShaderStage::Vertex, .Path = "Assets/Shaders/vs.hlsl" },
                                                                                          ShaderDesc{ .Stage = ShaderStage::Pixel, .Path = "Assets/Shaders/fs.hlsl" } } });
        m_rootSignature = m_logicalDevice->CreateRootSignature(
            RootSignatureDesc{ .ResourceBindings = {
                                   ResourceBindingDesc{ .Name = "mvp", .Binding = 0, .Descriptor = ResourceDescriptor::UniformBuffer, .Stages = { ShaderStage::Vertex } },
                                   ResourceBindingDesc{ .Name = "time", .Binding = 1, .Descriptor = ResourceDescriptor::UniformBuffer, .Stages = { ShaderStage::Vertex } },
                                   ResourceBindingDesc{ .Name = "texture1", .Binding = 0, .Descriptor = ResourceDescriptor::Texture, .Stages = { ShaderStage::Pixel } },
                                   ResourceBindingDesc{ .Name = "sampler1", .Binding = 0, .Descriptor = ResourceDescriptor::Sampler, .Stages = { ShaderStage::Pixel } },
                               } });

        m_inputLayout = m_logicalDevice->CreateInputLayout(VertexPositionNormalTexture::InputLayout);

        const GraphicsWindowSurface &surface = m_window->GetSurface();
        m_swapChain = m_logicalDevice->CreateSwapChain(SwapChainDesc{ .Width = surface.Width, .Height = surface.Height, .BufferCount = mc_framesInFlight });

        PipelineDesc pipelineDesc{ .ShaderProgram = m_program.get() };
        pipelineDesc.BlendModes    = { BlendMode::None };
        pipelineDesc.RootSignature = m_rootSignature.get();
        pipelineDesc.InputLayout   = m_inputLayout.get();
        pipelineDesc.Rendering.ColorAttachmentFormats.push_back(m_swapChain->GetPreferredFormat());

        m_pipeline        = m_logicalDevice->CreatePipeline(pipelineDesc);
        m_commandListRing = std::make_unique<CommandListRing>(m_logicalDevice.get());

        for ( uint32_t i = 0; i < mc_framesInFlight; ++i )
        {
            m_fences.push_back(m_logicalDevice->CreateFence());
            m_imageReadySemaphores.push_back(m_logicalDevice->CreateSemaphore());
            m_imageRenderedSemaphores.push_back(m_logicalDevice->CreateSemaphore());
        }

        UpdateMVPMatrix();

        BufferDesc deltaTimeBufferDesc{};
        deltaTimeBufferDesc.HeapType   = HeapType::CPU_GPU;
        deltaTimeBufferDesc.Descriptor = ResourceDescriptor::UniformBuffer;
        deltaTimeBufferDesc.NumBytes   = sizeof(float);

        float timePassed         = 1.0f;
        m_timePassedBuffer       = m_logicalDevice->CreateBufferResource("time", deltaTimeBufferDesc);
        m_mappedTimePassedBuffer = m_timePassedBuffer->MapMemory();
        memcpy(m_mappedTimePassedBuffer, &timePassed, sizeof(float));

        BufferDesc mvpBufferDesc{};
        mvpBufferDesc.HeapType     = HeapType::GPU;
        mvpBufferDesc.Descriptor   = ResourceDescriptor::UniformBuffer;
        mvpBufferDesc.InitialState = ResourceState::CopyDst;
        mvpBufferDesc.NumBytes     = sizeof(XMMATRIX);
        m_mvpMatrixBuffer          = m_logicalDevice->CreateBufferResource("mvp", mvpBufferDesc);

        BufferDesc vBufferDesc{};
        vBufferDesc.HeapType     = HeapType::GPU;
        vBufferDesc.Descriptor   = ResourceDescriptor::VertexBuffer;
        vBufferDesc.InitialState = ResourceState::CopyDst;
        vBufferDesc.NumBytes     = m_rect.SizeOfVertices();
        m_vertexBuffer           = m_logicalDevice->CreateBufferResource("vb", vBufferDesc);

        BufferDesc iBufferDesc{};
        iBufferDesc.HeapType     = HeapType::GPU;
        iBufferDesc.Descriptor   = ResourceDescriptor::IndexBuffer;
        iBufferDesc.InitialState = ResourceState::CopyDst;
        iBufferDesc.NumBytes     = m_rect.SizeOfIndices();
        m_indexBuffer            = m_logicalDevice->CreateBufferResource("ib", iBufferDesc);

        m_batchResourceCopy->Begin();
        m_texture = m_batchResourceCopy->CreateAndLoadTexture("texture1", "Assets/Textures/Dracolich.png");
        m_sampler = m_logicalDevice->CreateSampler("sampler1", SamplerDesc{});
        m_batchResourceCopy->CopyToGPUBuffer({ .DstBuffer = m_mvpMatrixBuffer.get(), .Data = &m_mvpMatrix, .NumBytes = sizeof(XMFLOAT4X4) });
        m_batchResourceCopy->CopyToGPUBuffer({ .DstBuffer = m_vertexBuffer.get(), .Data = m_rect.Vertices.data(), .NumBytes = vBufferDesc.NumBytes });
        m_batchResourceCopy->CopyToGPUBuffer({ .DstBuffer = m_indexBuffer.get(), .Data = m_rect.Indices.data(), .NumBytes = iBufferDesc.NumBytes });
        m_batchResourceCopy->End(nullptr);

        ResourceBindGroupDesc bindGroupDesc = {};
        bindGroupDesc.RootSignature         = m_rootSignature.get();
        bindGroupDesc.RootParameterIndex    = 0;
        bindGroupDesc.MaxNumBuffers         = 2;
        bindGroupDesc.MaxNumTextures        = 1;
        bindGroupDesc.MaxNumSamplers        = 1;
        m_resourceBindGroup                 = m_logicalDevice->CreateResourceBindGroup(bindGroupDesc);
        UpdateDesc updateDesc               = {};
        updateDesc.Buffers                  = { m_mvpMatrixBuffer.get(), m_timePassedBuffer.get() };
        updateDesc.Textures                 = { m_texture.get() };
        updateDesc.Samplers                 = { m_sampler.get() };
        m_resourceBindGroup->Update(updateDesc);

        m_time->ListenFps = [](const double fps) { DLOG(INFO) << std::format("FPS: {}", fps); };

        LOG(INFO) << "Initialization Complete.";
    }

    void SimpleRenderer::UpdateMVPMatrix()
    {
        XMFLOAT3 eyePosition = XMFLOAT3(0.0f, -1.0f, -2.0f);
        XMFLOAT3 focusPoint  = XMFLOAT3(0.0f, 0.0f, 0.0f);
        XMFLOAT3 upDirection = XMFLOAT3(0.0f, 1.0f, 0.0f);
        float    aspectRatio = 800.0f / 600.0f;
        float    nearZ       = 0.1f;
        float    farZ        = 100.0f;

        // Set up the matrices
        XMMATRIX modelMatrix      = XMMatrixIdentity();
        XMMATRIX viewMatrix       = XMMatrixLookAtLH(XMLoadFloat3(&eyePosition), XMLoadFloat3(&focusPoint), XMLoadFloat3(&upDirection));
        XMMATRIX projectionMatrix = XMMatrixPerspectiveFovLH(XM_PIDIV4, aspectRatio, nearZ, farZ);

        // Compute the MVP matrix
        XMMATRIX mvpMatrix = modelMatrix * viewMatrix * projectionMatrix;
        XMStoreFloat4x4(&m_mvpMatrix, XMMatrixTranspose(mvpMatrix));
    }

    void SimpleRenderer::Render()
    {
        float timePassed = std::fmax(1.0f, (m_time->DoubleEpochNow() - m_time->GetFirstTickTime()) / 1000000.0f);
        memcpy(m_mappedTimePassedBuffer, &timePassed, sizeof(float));
        m_time->Tick();

        auto     nextCommandList = m_commandListRing->GetNext();
        uint32_t currentFrame    = m_commandListRing->GetCurrentFrame();
        m_fences[ currentFrame ]->Wait();

        uint32_t nextImage = m_swapChain->AcquireNextImage(m_imageReadySemaphores[ currentFrame ].get());
        nextCommandList->Begin();

        if ( m_isFirstFrame )
        {
            PipelineBarrierDesc barrier{};
            barrier.TextureBarrier(TextureBarrierDesc{ .Resource = m_texture.get(), .OldState = ResourceState::CopyDst, .NewState = ResourceState::PixelShaderResource });
            nextCommandList->PipelineBarrier(barrier);
        }

        RenderingAttachmentDesc renderingAttachmentDesc{};
        renderingAttachmentDesc.Resource = m_swapChain->GetRenderTarget(nextImage);

        RenderingDesc renderingInfo{};
        renderingInfo.RTAttachments.push_back(std::move(renderingAttachmentDesc));

        nextCommandList->PipelineBarrier(PipelineBarrierDesc::UndefinedToRenderTarget(m_swapChain->GetRenderTarget(nextImage)));
        nextCommandList->BeginRendering(renderingInfo);

        const Viewport &viewport = m_swapChain->GetViewport();
        nextCommandList->BindViewport(viewport.X, viewport.Y, viewport.Width, viewport.Height);
        nextCommandList->BindScissorRect(viewport.X, viewport.Y, viewport.Width, viewport.Height);

        nextCommandList->BindPipeline(m_pipeline.get());
        nextCommandList->BindResourceGroup(m_resourceBindGroup.get());
        nextCommandList->BindVertexBuffer(m_vertexBuffer.get());
        nextCommandList->BindIndexBuffer(m_indexBuffer.get(), IndexType::Uint32);
        nextCommandList->DrawIndexed(m_rect.Indices.size(), 1);
        nextCommandList->EndRendering();
        nextCommandList->PipelineBarrier(PipelineBarrierDesc::RenderTargetToPresent(m_swapChain->GetRenderTarget(nextImage)));

        ExecuteDesc submitInfo{};
        submitInfo.Notify = m_fences[ currentFrame ].get();
        submitInfo.WaitOnSemaphores.push_back(m_imageReadySemaphores[ currentFrame ].get());
        submitInfo.NotifySemaphores.push_back(m_imageRenderedSemaphores[ currentFrame ].get());
        nextCommandList->Execute(submitInfo);
        nextCommandList->Present(m_swapChain.get(), nextImage, { m_imageRenderedSemaphores[ currentFrame ].get() });

        m_isFirstFrame = false;
    }

    void SimpleRenderer::Quit()
    {
        m_timePassedBuffer->UnmapMemory();
        for ( uint32_t i = 0; i < mc_framesInFlight; ++i )
        {
            m_fences[ i ]->Wait();
        }
        m_logicalDevice->WaitIdle();
        GfxGlobal::Destroy();
    }

} // namespace DenOfIz
