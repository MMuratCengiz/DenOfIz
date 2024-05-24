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

#include "ILock.h"
#include "IFence.h"
#include "ISemaphore.h"
#include "IPipeline.h"
#include "IResource.h"
#include "ISwapChain.h"

namespace DenOfIz
{

struct BufferBarrier
{
	ResourceState CurrentState;
	ResourceState NewState;
};

struct ImageBarrier
{
	ResourceState CurrentState;
	ResourceState NewState;

	bool SubresourceBarrier = false;
	uint8_t MipLevel = 7; // Todo check this whole mip levels thing
	uint16_t ArrayLayer;
};

class ICommandList
{
public:
	virtual ~ICommandList() = default;

	virtual void Reset() = 0;
	virtual void Begin() = 0;
	virtual void End() = 0;
	virtual void Submit(IFence* notify) = 0;
	virtual uint32_t AcquireNextImage(ISwapChain* swapChain, ISemaphore* lock) = 0;
	virtual void BindPipeline(IPipeline* pipeline) = 0;
	virtual void BindVertexBuffer(IBufferResource* buffer) = 0;
	virtual void BindIndexBuffer(IBufferResource* buffer) = 0;
	virtual void BindViewport(float x, float y, float width, float height) = 0;
	virtual void BindScissorRect(float x, float y, float width, float height) = 0;
	virtual void BindPushConstants(ShaderStage stage, uint32_t offset, uint32_t size, void* data) = 0;
	virtual void BindBufferResource(IBufferResource* resource) = 0;
	virtual void BindImageResource(IImageResource* resource) = 0;
	virtual void SetDepthBias(float constantFactor, float clamp, float slopeFactor) = 0;
	virtual void ClearColor(float r, float g, float b, float a) = 0;
	virtual void ClearDepthStencil(float depth, uint32_t stencil) = 0;
	virtual void SetImageBarrier(IImageResource* resource, ImageBarrier barrier) = 0;
	virtual void SetBufferBarrier(IBufferResource* resource, BufferBarrier barrier) = 0;
	virtual void DrawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex, uint32_t vertexOffset, uint32_t firstInstance) = 0;
	virtual void Draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex, uint32_t firstInstance) = 0;
	virtual void Dispatch(uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ) = 0;
	virtual void CopyBuffer(IBufferResource* src, IBufferResource* dst, uint32_t size) = 0;
	virtual void TransitionImageLayout(IImageResource* image, ImageLayout oldLayout, ImageLayout newLayout) = 0;
	virtual void ClearColorAttachment(uint32_t attachmentIndex, float r, float g, float b, float a) = 0;
	virtual void ClearDepthStencilAttachment(float depth, uint32_t stencil) = 0;
	virtual void Present(ISwapChain* swapChain, uint32_t imageIndex, std::vector<ISemaphore*> waitOnLocks) = 0;
};

}