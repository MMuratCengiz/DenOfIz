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
	auto vs = compiler.HLSLtoSPV(ShaderStage::Vertex, "vs.hlsl");
	auto fs = compiler.HLSLtoSPV(ShaderStage::Fragment, "fs.hlsl");
	compiler.Destroy();

	m_program = std::make_unique<SpvProgram>(std::vector<CompiledShader>{ CompiledShader{ .Stage = ShaderStage::Vertex, .Data = std::move(vs) },
																		  CompiledShader{ .Stage = ShaderStage::Fragment, .Data = std::move(fs) }});

	auto firstDevice = m_logicalDevice->ListPhysicalDevices()[0];
	m_logicalDevice->LoadPhysicalDevice(firstDevice);

	PipelineCreateInfo pipelineCreateInfo{ .SpvProgram = *m_program };
	pipelineCreateInfo.BlendModes = { BlendMode::None };
//	pipelineCreateInfo.Rendering.ColorAttachmentFormats.push_back(m_logicalDevice->GetContext()->SurfaceImageFormat); Todo

	m_pipeline = m_logicalDevice->CreatePipeline(pipelineCreateInfo);

	BufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.MemoryCreateInfo.Size = m_Triangle.size() * sizeof(float);
	bufferCreateInfo.MemoryCreateInfo.Location = MemoryLocation::GPU;
	bufferCreateInfo.MemoryCreateInfo.Usage = BufferMemoryUsage::VertexBuffer;
	bufferCreateInfo.UseStaging = true;

	m_VertexBuffer = m_logicalDevice->CreateBufferResource("vb", bufferCreateInfo);
	m_VertexBuffer->Allocate(m_Triangle.data());

	BufferCreateInfo deltaTimeBufferCreateInfo;
	deltaTimeBufferCreateInfo.MemoryCreateInfo.Size = sizeof(float);
	deltaTimeBufferCreateInfo.MemoryCreateInfo.Location = MemoryLocation::CPU_GPU;
	deltaTimeBufferCreateInfo.MemoryCreateInfo.Usage = BufferMemoryUsage::UniformBuffer;

	m_TimePassedBuffer = m_logicalDevice->CreateBufferResource("time", deltaTimeBufferCreateInfo);

	RenderPassCreateInfo createInfo{};
	createInfo.RenderToSwapChain = true;
	createInfo.RenderTargetType = RenderTargetType::Color;

	for (int i = 0; i < 3; i++)
	{
		createInfo.SwapChainImageIndex = i;
		m_renderPasses.push_back(std::move(m_logicalDevice->CreateRenderPass(createInfo)));
		m_fences.push_back(m_logicalDevice->CreateFence());
	}

	m_time->ListenFps = [](const double fps)
	{
		std::cout << "FPS: " << fps << "\n";
	};
}

void SimpleRenderer::Render()
{
	/*
	 * RenderPass.Begin()
	 * 		.BindPipeline()
	 * 		.BindResource()
	 * RenderPass.End()
	 * 		.Render();
	 */
}

}