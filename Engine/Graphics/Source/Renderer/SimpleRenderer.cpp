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

void SimpleRenderer::Init(SDL_Window* window)
{
	m_window = window;
	
	APIPreference apiPreference = {};
	apiPreference.Windows = APIPreferenceWindows::Vulkan;
	GraphicsAPIInit gapiInit(apiPreference);

	m_logicalDevice = gapiInit.CreateLogicalDevice(m_window);
	auto firstDevice = m_logicalDevice->ListPhysicalDevices()[0];
	m_logicalDevice->LoadPhysicalDevice(firstDevice);

	auto compiler = ShaderCompiler();
	compiler.Init();
	auto vs = compiler.HLSLtoSPV(ShaderStage::Vertex, "Assets/Shaders/vs.hlsl");
	auto fs = compiler.HLSLtoSPV(ShaderStage::Fragment, "Assets/Shaders/fs.hlsl");
	compiler.Destroy();

	m_program = std::make_unique<ShaderProgram>(std::vector<CompiledShader>{ CompiledShader{ .Stage = ShaderStage::Vertex, .Data = std::move(vs) },
																			 CompiledShader{ .Stage = ShaderStage::Fragment, .Data = std::move(fs) }});

	m_rootSignature = m_logicalDevice->CreateRootSignature(RootSignatureCreateInfo{});
	ResourceBinding timePassedBinding{};
	timePassedBinding.Name = "time";
	timePassedBinding.Binding = 0;
	timePassedBinding.Type = ResourceBindingType::Buffer;
	timePassedBinding.Stages = { ShaderStage::Vertex };
	m_rootSignature->AddResourceBinding(timePassedBinding);
	m_rootSignature->Create();

	m_swapChain = m_logicalDevice->CreateSwapChain(SwapChainCreateInfo{});

	PipelineCreateInfo pipelineCreateInfo{ .ShaderProgram = *m_program };
	pipelineCreateInfo.BlendModes = { BlendMode::None };
	pipelineCreateInfo.RootSignature = m_rootSignature.get();
	pipelineCreateInfo.Rendering.ColorAttachmentFormats.push_back(m_swapChain->GetPreferredFormat());

	m_pipeline = m_logicalDevice->CreatePipeline(pipelineCreateInfo);
	m_commandListRing = std::make_unique<CommandListRing>(m_logicalDevice.get());
	for (int i = 0; i < mc_framesInFlight; ++i)
	{
		m_commandListRing->NewCommandList(CommandListCreateInfo());
		m_fences.push_back(m_logicalDevice->CreateFence());
		m_imageReadySemaphores.push_back(m_logicalDevice->CreateSemaphore());
		m_imageRenderedSemaphores.push_back(m_logicalDevice->CreateSemaphore());
	}

	BufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.HeapType = HeapType::GPU;
	bufferCreateInfo.Usage = BufferMemoryUsage::VertexBuffer;
	bufferCreateInfo.UseStaging = true;

	m_vertexBuffer = m_logicalDevice->CreateBufferResource("vb", bufferCreateInfo);
	m_vertexBuffer->Allocate(m_triangle.data(), m_triangle.size() * sizeof(float));

	BufferCreateInfo deltaTimeBufferCreateInfo {};
	deltaTimeBufferCreateInfo.HeapType = HeapType::CPU_GPU;
	deltaTimeBufferCreateInfo.Usage = BufferMemoryUsage::UniformBuffer;

	m_timePassedBuffer = m_logicalDevice->CreateBufferResource("time", deltaTimeBufferCreateInfo);
	float timePassed = 0.0f;
	m_timePassedBuffer->Allocate(&timePassed, sizeof(float));

	m_descriptorTable = m_logicalDevice->CreateDescriptorTable(DescriptorTableCreateInfo{ .RootSignature = m_rootSignature.get() });
	m_descriptorTable->BindBuffer(m_timePassedBuffer.get());

//	RenderPassCreateInfo createInfo{};
//	createInfo.RenderToSwapChain = true;
//	createInfo.RenderTargetType = RenderTargetType::Color;
//
//	for (int i = 0; i < 3; i++)
//	{
//		createInfo.SwapChainImageIndex = i;
//		m_renderPasses.push_back(std::move(m_logicalDevice->CreateRenderPass(createInfo)));
//		m_fences.push_back(m_logicalDevice->CreateFence());
//	}

	m_time->ListenFps = [](const double fps)
	{
		std::cout << "FPS: " << fps << "\n";
	};
}

void SimpleRenderer::Render()
{
	float timePassed = (m_time->DoubleEpochNow() - m_time->GetFirstTickTime()) / 1000000.0f;
	m_timePassedBuffer->Allocate(&timePassed, sizeof(float));
	m_time->Tick();

	auto nextCommandList = m_commandListRing->GetNext();
	uint32_t currentFrame = m_commandListRing->GetCurrentFrame();

	m_fences[currentFrame]->Wait();
	uint32_t nextImage = m_swapChain->AcquireNextImage(m_imageReadySemaphores[currentFrame].get());
	nextCommandList->Reset();
	nextCommandList->Begin();

	RenderingAttachmentInfo renderingAttachmentInfo{};
	renderingAttachmentInfo.Resource = m_swapChain->GetRenderTarget(nextImage);

	RenderingInfo renderingInfo{};
	renderingInfo.ColorAttachments.push_back(std::move(renderingAttachmentInfo));

	PipelineBarrier pipelineBarrier{};
	ImageBarrierInfo imageBarrier{};
	imageBarrier.OldState.Undefined = 1;
	imageBarrier.NewState.Present = 1;
	imageBarrier.Resource = m_swapChain->GetRenderTarget(nextImage);
	pipelineBarrier.ImageBarrier(imageBarrier);
	nextCommandList->SetPipelineBarrier(pipelineBarrier);

	nextCommandList->BeginRendering(renderingInfo);
	const Viewport& viewport = m_swapChain->GetViewport();
	nextCommandList->BindViewport(viewport.X, viewport.Y, viewport.Width, viewport.Height);
	nextCommandList->BindScissorRect(viewport.X, viewport.Y, viewport.Width, viewport.Height);
	nextCommandList->BindPipeline(m_pipeline.get());
	nextCommandList->BindDescriptorTable(m_descriptorTable.get());
	nextCommandList->BindVertexBuffer(m_vertexBuffer.get());
	nextCommandList->Draw(3, 1);
	nextCommandList->EndRendering();
	nextCommandList->End();

	SubmitInfo submitInfo{};
	submitInfo.Notify = m_fences[currentFrame].get();
	submitInfo.WaitOnLocks.push_back(m_imageReadySemaphores[currentFrame].get());
	submitInfo.SignalLocks.push_back(m_imageRenderedSemaphores[currentFrame].get());

	nextCommandList->Submit(submitInfo);
	nextCommandList->Present(m_swapChain.get(), nextImage, { m_imageRenderedSemaphores[currentFrame].get() });
}

void SimpleRenderer::Quit()
{
	m_logicalDevice->WaitIdle();
}

}