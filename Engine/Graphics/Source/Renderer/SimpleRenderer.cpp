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

	auto compiler = ShaderCompiler();
	compiler.Init();
	auto vs = compiler.HLSLtoSPV(ShaderStage::Vertex, "Shaders/vs.hlsl");
	auto fs = compiler.HLSLtoSPV(ShaderStage::Fragment, "Shaders/fs.hlsl");
	compiler.Destroy();

	m_program = std::make_unique<SpvProgram>(std::vector<CompiledShader>{ CompiledShader{ .Stage = ShaderStage::Vertex, .Data = std::move(vs) },
																		  CompiledShader{ .Stage = ShaderStage::Fragment, .Data = std::move(fs) }});

	m_rootSignature = m_logicalDevice->CreateRootSignature(RootSignatureCreateInfo{});
	m_swapChain = m_logicalDevice->CreateSwapChain(SwapChainCreateInfo{});

	auto firstDevice = m_logicalDevice->ListPhysicalDevices()[0];
	m_logicalDevice->LoadPhysicalDevice(firstDevice);

	PipelineCreateInfo pipelineCreateInfo{ .SpvProgram = *m_program };
	pipelineCreateInfo.BlendModes = { BlendMode::None };
	pipelineCreateInfo.RootSignature = m_rootSignature.get();
	pipelineCreateInfo.Rendering.ColorAttachmentFormats.push_back(m_swapChain->GetPreferredFormat());

	m_pipeline = m_logicalDevice->CreatePipeline(pipelineCreateInfo);
	m_commandListRing = std::make_unique<CommandListRing>(m_logicalDevice.get());
	for (int i = 0; i < mc_framesInFlight; ++i)
	{
		m_commandListRing->NewCommandList(CommandListCreateInfo());
	}

	BufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.Location = MemoryLocation::GPU;
	bufferCreateInfo.Usage = BufferMemoryUsage::VertexBuffer;
	bufferCreateInfo.UseStaging = true;

	m_vertexBuffer = m_logicalDevice->CreateBufferResource("vb", bufferCreateInfo);
	m_vertexBuffer->Allocate(m_Triangle.data(), m_Triangle.size() * sizeof(float));

	BufferCreateInfo deltaTimeBufferCreateInfo {};
	deltaTimeBufferCreateInfo.Location = MemoryLocation::CPU_GPU;
	deltaTimeBufferCreateInfo.Usage = BufferMemoryUsage::UniformBuffer;

	m_timePassedBuffer = m_logicalDevice->CreateBufferResource("time", deltaTimeBufferCreateInfo);

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

	auto nextCommandList = m_commandListRing->GetNextInFlight();
	nextCommandList->Reset();
	nextCommandList->Begin();

	RenderingInfo renderingInfo{};
	renderingInfo.ColorAttachments.push_back(RenderingAttachmentInfo{ .Resource = m_swapChain.get() });

	nextCommandList->BeginRendering(renderingInfo);
	nextCommandList->BindPipeline(m_pipeline.get());
	nextCommandList->BindVertexBuffer(m_vertexBuffer.get());
	nextCommandList->Draw(1, 3);
	nextCommandList->End();
	nextCommandList->Submit(m_fences[m_FrameIndex].get(), {});
}

}