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

        m_logicalDevice = GraphicsAPI::CreateLogicalDevice(m_window);
        auto firstDevice = m_logicalDevice->ListPhysicalDevices()[ 0 ];
        m_logicalDevice->LoadPhysicalDevice(firstDevice);

        m_program.AddShader(ShaderDesc{ .Stage = ShaderStage::Vertex, .Path = "Assets/Shaders/vs.hlsl" });
        m_program.AddShader(ShaderDesc{ .Stage = ShaderStage::Fragment, .Path = "Assets/Shaders/fs.hlsl" });
        m_program.Compile();

        m_rootSignature = m_logicalDevice->CreateRootSignature(RootSignatureDesc{});
        ResourceBinding timePassedBinding{};
        timePassedBinding.Name = "time";
        timePassedBinding.Binding = 0;
        timePassedBinding.Type = ResourceBindingType::Buffer;
        timePassedBinding.Stages = { ShaderStage::Vertex };
        m_rootSignature->AddResourceBinding(timePassedBinding);
        m_rootSignature->Create();

        m_inputLayout =
            m_logicalDevice->CreateInputLayout({ .InputGroups = { { .Elements =
                                                                    {
                                                                            InputLayoutElement{ .Semantic = Semantic::Position, .Format = Format::R32G32B32A32Float },
                                                                            InputLayoutElement{ .Semantic = Semantic::Color, .Format = Format::R32G32B32A32Float,} ,
                                                                    },
                                                                    .StepRate = StepRate::PerVertex } } });

        const GraphicsWindowSurface &surface = m_window->GetSurface();
        m_swapChain = m_logicalDevice->CreateSwapChain(SwapChainDesc{
            .Width = surface.Width,
            .Height = surface.Height,
            .BufferCount = mc_framesInFlight
        });

        PipelineDesc pipelineDesc{ .ShaderProgram = m_program };
        pipelineDesc.BlendModes = { BlendMode::None };
        pipelineDesc.RootSignature = m_rootSignature.get();
        pipelineDesc.InputLayout = m_inputLayout.get();
        pipelineDesc.Rendering.ColorAttachmentFormats.push_back(m_swapChain->GetPreferredFormat());

        m_pipeline = m_logicalDevice->CreatePipeline(pipelineDesc);
        m_commandListRing = std::make_unique<CommandListRing>(m_logicalDevice.get());

        for ( uint32_t i = 0; i < mc_framesInFlight; ++i )
        {
            m_fences.push_back(m_logicalDevice->CreateFence());
            m_imageReadySemaphores.push_back(m_logicalDevice->CreateSemaphore());
            m_imageRenderedSemaphores.push_back(m_logicalDevice->CreateSemaphore());
        }

        BufferDesc bufferDesc{};
        bufferDesc.HeapType = HeapType::GPU;
        bufferDesc.Usage.VertexBuffer = 1;

        m_vertexBuffer = m_logicalDevice->CreateBufferResource("vb", bufferDesc);
        m_vertexBuffer->Allocate(m_triangle.data(), m_triangle.size() * sizeof(float));

        BufferDesc deltaTimeBufferDesc{};
        deltaTimeBufferDesc.HeapType = HeapType::CPU_GPU;
        deltaTimeBufferDesc.Usage.UniformBuffer = 1;
        deltaTimeBufferDesc.KeepMemoryMapped = true;

        m_timePassedBuffer = m_logicalDevice->CreateBufferResource("time", deltaTimeBufferDesc);
        float timePassed = 1.0f;
        m_timePassedBuffer->Allocate(&timePassed, sizeof(float));

        m_descriptorTable = m_logicalDevice->CreateDescriptorTable(DescriptorTableDesc{ .RootSignature = m_rootSignature.get() });
        m_descriptorTable->BindBuffer(m_timePassedBuffer.get());
        m_time->ListenFps = [](const double fps) { DLOG(INFO) << std::format("FPS: {}", fps); };

        LOG(INFO) << "Initialization Complete.";
    }

    void SimpleRenderer::Render()
    {
        float timePassed = std::fmax(1.0f, (m_time->DoubleEpochNow() - m_time->GetFirstTickTime()) / 1000000.0f);
        m_timePassedBuffer->Allocate(&timePassed, sizeof(float));
        m_time->Tick();

        auto nextCommandList = m_commandListRing->GetNext();
        uint32_t currentFrame = m_commandListRing->GetCurrentFrame();
        m_fences[ currentFrame ]->Wait();

        uint32_t nextImage = m_swapChain->AcquireNextImage(m_imageReadySemaphores[ currentFrame ].get());
        nextCommandList->Begin();

        RenderingAttachmentDesc renderingAttachmentDesc{};
        renderingAttachmentDesc.Resource = m_swapChain->GetRenderTarget(nextImage);

        RenderingDesc renderingInfo{};
        renderingInfo.RTAttachments.push_back(std::move(renderingAttachmentDesc));

        nextCommandList->SetPipelineBarrier(PipelineBarrier::UndefinedToRenderTarget(m_swapChain->GetRenderTarget(nextImage)));
        nextCommandList->BeginRendering(renderingInfo);

        const Viewport &viewport = m_swapChain->GetViewport();
        nextCommandList->BindViewport(viewport.X, viewport.Y, viewport.Width, viewport.Height);
        nextCommandList->BindScissorRect(viewport.X, viewport.Y, viewport.Width, viewport.Height);

        nextCommandList->BindPipeline(m_pipeline.get());
        nextCommandList->BindDescriptorTable(m_descriptorTable.get());
        nextCommandList->BindVertexBuffer(m_vertexBuffer.get());
        nextCommandList->Draw(3, 1);
        nextCommandList->EndRendering();
        nextCommandList->SetPipelineBarrier(PipelineBarrier::RenderTargetToPresent(m_swapChain->GetRenderTarget(nextImage)));

        ExecuteDesc submitInfo{};
        submitInfo.Notify = m_fences[ currentFrame ].get();
        submitInfo.WaitOnLocks.push_back(m_imageReadySemaphores[ currentFrame ].get());
        submitInfo.SignalLocks.push_back(m_imageRenderedSemaphores[ currentFrame ].get());
        nextCommandList->Execute(submitInfo);
        nextCommandList->Present(m_swapChain.get(), nextImage, { m_imageRenderedSemaphores[ currentFrame ].get() });
    }

    void SimpleRenderer::Quit()
    {
        m_logicalDevice->WaitIdle();
        m_commandListRing.reset();
        m_swapChain.reset();
        m_rootSignature.reset();
        m_inputLayout.reset();
        m_pipeline.reset();
        m_logicalDevice->WaitIdle();
        m_vertexBuffer.reset();
        m_timePassedBuffer.reset();
        for ( uint32_t i = 0; i < mc_framesInFlight; ++i )
        {
            m_fences[ i ].reset();
            m_imageReadySemaphores[ i ].reset();
            m_imageRenderedSemaphores[ i ].reset();
        }
        m_logicalDevice.reset();
        GfxGlobal::Destroy();
        GraphicsAPI::ReportLiveObjects();
    }

} // namespace DenOfIz
