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
	this->window = w;

	auto compiler = ShaderCompiler();
	compiler.Init();
	auto vs = compiler.HLSLtoSPV(ShaderStage::Vertex, "vs.hlsl");
	auto fs = compiler.HLSLtoSPV(ShaderStage::Fragment, "fs.hlsl");
	compiler.Destroy();
	program = std::make_unique<SpvProgram>(std::vector<CompiledShader>{
			CompiledShader{
					.Stage = ShaderStage::Vertex,
					.Data = std::move(vs)
			},
			CompiledShader{
					.Stage = ShaderStage::Fragment,
					.Data = std::move(fs)
			}});

	device.CreateDevice(window);
	device.ListDevices()[0].Select();
	device.WaitIdle();

	PipelineCreateInfo pipelineCreateInfo{
			.SpvProgram = *program.get()
	};

	pipeline = std::make_unique<VulkanPipeline>(device.GetContext(), pipelineCreateInfo);

	BufferCreateInfo bufferCreateInfo{};
	bufferCreateInfo.MemoryCreateInfo.Size = triangle.size() * sizeof(float);
	bufferCreateInfo.MemoryCreateInfo.Location = MemoryLocation::GPU;
	bufferCreateInfo.MemoryCreateInfo.Usage = MemoryUsage::VertexBuffer;
	bufferCreateInfo.UseStaging = true;

	vertexBuffer = std::make_unique<VulkanBufferResource>(device.GetContext(), bufferCreateInfo);
	vertexBuffer->Allocate(triangle.data());

	BufferCreateInfo deltaTimeBufferCreateInfo;
	deltaTimeBufferCreateInfo.MemoryCreateInfo.Size = sizeof(float);
	deltaTimeBufferCreateInfo.MemoryCreateInfo.Location = MemoryLocation::CPU_GPU;
	deltaTimeBufferCreateInfo.MemoryCreateInfo.Usage = MemoryUsage::UniformBuffer;

	timePassedBuffer = std::make_unique<VulkanBufferResource>(device.GetContext(), deltaTimeBufferCreateInfo);
	timePassedBuffer->Name = "time";

	RenderPassCreateInfo createInfo{};
	createInfo.RenderToSwapChain = true;
	createInfo.Format = device.GetSwapChainImageFormat();
	createInfo.RenderTargetType = RenderTargetType::Color;

	renderPass = std::make_unique<VulkanRenderPass>(device.GetContext(), createInfo);

	for (int i = 0; i < 3; i++)
	{
		fences.push_back(std::make_unique<VulkanLock>(device.GetContext(), LockType::Fence));
	}

	time->ListenFps = [](double fps)
	{
		std::cout << "FPS: " << fps << std::endl;
	};
}

void TestVulkanRenderer::Render()
{
	float timePassed = (time->DoubleEpochNow() - time->GetFirstTickTime()) / 1000000.0f;
//	timePassedBuffer->Allocate(&timePassed);
	time->Tick();

	fences[frameIndex]->Wait();
	renderPass->Begin(frameIndex, { 0.0f, 0.0f, 0.0f, 1.0f });
	renderPass->BindPipeline(pipeline.get());
	renderPass->BindVertexBuffer(vertexBuffer.get());
//	renderPass->BindResource((IResource*)timePassedBuffer.get());
	renderPass->Draw(1, 3);
	renderPass->Submit({}, fences[frameIndex].get());
	frameIndex = (frameIndex + 1) % 3;
}

void TestVulkanRenderer::Exit()
{
	device.WaitIdle();
}