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

#include <DenOfIzGraphics/Backends/Interface/ICommandList.h>
#include "DX12Context.h"
#include "DX12DescriptorTable.h"
#include "DX12Pipeline.h"
#include "DX12EnumConverter.h"
#include "DX12SwapChain.h"
#include "Resource/DX12ImageResource.h"
#include "Resource/DX12BufferResource.h"

namespace DenOfIz
{

class DX12CommandList : public ICommandList
{
private:
	CommandListCreateInfo m_createInfo;
	DX12Context* m_context;

	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList10> m_commandList;
	ID3D12RootSignature * m_currentRootSignature = nullptr;

	CD3DX12_RECT m_scissor;
	D3D12_VIEWPORT m_viewport;
public:
	DX12CommandList(DX12Context* context, CommandListCreateInfo createInfo);

	void Begin() override;
	void BeginRendering(const RenderingInfo& renderingInfo) override;
	void EndRendering() override;
	void Execute(const ExecuteInfo& executeInfo) override;
	void Present(ISwapChain* swapChain, uint32_t imageIndex, std::vector<ISemaphore*> waitOnLocks) override;
	void BindPipeline(IPipeline* pipeline) override;
	void BindVertexBuffer(IBufferResource* buffer) override;
	void BindIndexBuffer(IBufferResource* buffer, const IndexType& indexType) override;
	void BindViewport(float x, float y, float width, float height) override;
	void BindScissorRect(float x, float y, float width, float height) override;
	void BindDescriptorTable(IDescriptorTable* table) override;
	void BindPushConstants(ShaderStage stage, uint32_t offset, uint32_t size, void* data) override;
	void BindBufferResource(IBufferResource* resource) override;
	void BindImageResource(IImageResource* resource) override;
	void SetDepthBias(float constantFactor, float clamp, float slopeFactor) override;
	void SetPipelineBarrier(const PipelineBarrier& barrier) override;
	void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) override;
	void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) override;
	void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) override;
	void TransitionImageLayout(IImageResource* image, ImageLayout oldLayout, ImageLayout newLayout) override;
private:
	void SetRootSignature(ID3D12RootSignature* rootSignature);
};

}