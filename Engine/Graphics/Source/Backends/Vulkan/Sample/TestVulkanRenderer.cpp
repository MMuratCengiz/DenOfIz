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

#include <DenOfIzGraphics/Backends/Vulkan/Sample/TestVulkanRenderer.h>

using namespace DenOfIz;

void TestVulkanRenderer::Setup(SDL_Window* w)
{
	this->m_Window = w;

	auto compiler = ShaderCompiler();
	compiler.Init();
	auto vs = compiler.HLSLtoSPV(ShaderStage::Vertex, "vs.hlsl");
	auto fs = compiler.HLSLtoSPV(ShaderStage::Fragment, "fs.hlsl");
	compiler.Destroy();
	m_Program = std::make_unique<SpvProgram>(std::vector<CompiledShader>{ CompiledShader{ .Stage = ShaderStage::Vertex, .Data = std::move(vs) },
																		  CompiledShader{ .Stage = ShaderStage::Fragment, .Data = std::move(fs) }});

	m_Device.CreateDevice(m_Window);
	auto firstDevice = m_Device.ListPhysicalDevices()[0];
	m_Device.LoadPhysicalDevice(firstDevice);

	PipelineCreateInfo pipelineCreateInfo{ .SpvProgram = *m_Program };
	pipelineCreateInfo.BlendModes = { BlendMode::None };
	pipelineCreateInfo.Rendering.ColorAttachmentFormats.push_back(m_Device.GetContext()->SurfaceImageFormat);

	m_Pipeline = std::make_unique<VulkanPipeline>(m_Device.GetContext(), pipelineCreateInfo);

	BufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.MemoryCreateInfo.Size = m_Triangle.size() * sizeof(float);
	bufferCreateInfo.MemoryCreateInfo.Location = MemoryLocation::GPU;
	bufferCreateInfo.MemoryCreateInfo.Usage = BufferMemoryUsage::VertexBuffer;
	bufferCreateInfo.UseStaging = true;

	m_VertexBuffer = std::make_unique<VulkanBufferResource>(m_Device.GetContext(), bufferCreateInfo);
	m_VertexBuffer->Allocate(m_Triangle.data());

	BufferCreateInfo deltaTimeBufferCreateInfo;
	deltaTimeBufferCreateInfo.MemoryCreateInfo.Size = sizeof(float);
	deltaTimeBufferCreateInfo.MemoryCreateInfo.Location = MemoryLocation::CPU_GPU;
	deltaTimeBufferCreateInfo.MemoryCreateInfo.Usage = BufferMemoryUsage::UniformBuffer;

	m_TimePassedBuffer = std::make_unique<VulkanBufferResource>(m_Device.GetContext(), deltaTimeBufferCreateInfo);
	m_TimePassedBuffer->Name = "time";

	RenderPassCreateInfo createInfo{};
	createInfo.RenderToSwapChain = true;
	createInfo.Format = m_Device.GetSwapChainImageFormat();
	createInfo.RenderTargetType = RenderTargetType::Color;

	for (int i = 0; i < 3; i++)
	{
		createInfo.SwapChainImageIndex = i;
//		m_RenderPasses.push_back(std::make_unique<VulkanRenderPass>(m_Device.GetContext(), createInfo));
		m_Fences.push_back(std::make_unique<VulkanLock>(m_Device.GetContext(), LockType::Fence));
	}

	m_Time->ListenFps = [](const double fps)
	{
		std::cout << "FPS: " << fps << "\n";
	};
}

void TestVulkanRenderer::Render()
{
	float timePassed = (m_Time->DoubleEpochNow() - m_Time->GetFirstTickTime()) / 1000000.0f;
	//	timePassedBuffer->Allocate(&timePassed);
	m_Time->Tick();

	m_Fences[m_FrameIndex]->Wait();
	m_RenderPasses[m_FrameIndex]->Begin({ 0.0f, 1.0f, 0.0f, 1.0f });
//	m_RenderPasses[m_FrameIndex]->BindPipeline(m_Pipeline.get());
	m_RenderPasses[m_FrameIndex]->BindVertexBuffer(m_VertexBuffer.get());
	//	renderPasses[frameIndex]->BindResource((IResource*)timePassedBuffer.get());
	m_RenderPasses[m_FrameIndex]->Draw(1, 3);
	m_RenderPasses[m_FrameIndex]->Submit({}, m_Fences[m_FrameIndex].get());
	m_FrameIndex = (m_FrameIndex + 1) % 3;
}

void TestVulkanRenderer::Exit()
{
	m_Device.WaitIdle();
}
