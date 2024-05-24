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

#include <DenOfIzGraphics/Backends/DirectX12/DX12RenderPass.h>

using namespace DenOfIz;

DX12RenderPass::DX12RenderPass(DX12Context* context, const RenderPassCreateInfo& createInfo)
	:m_context(context), m_createInfo(createInfo)
{

}

DX12RenderPass::~DX12RenderPass()
{

}

void DX12RenderPass::UpdateViewport(const uint32_t& width, const uint32_t& height)
{

}

void DX12RenderPass::SetDepthBias(float constant, float clamp, float slope) const
{

}

void DX12RenderPass::Begin(const std::array<float, 4> clearColor)
{

}

void DX12RenderPass::BindPipeline(IPipeline* pipeline)
{

}

void DX12RenderPass::BindResource(IResource* resource)
{

}

void DX12RenderPass::BindIndexBuffer(IBufferResource* resource)
{

}

void DX12RenderPass::BindVertexBuffer(IBufferResource* resource) const
{

}

void DX12RenderPass::Draw(const uint32_t& instanceCount, const uint32_t& vertexCount) const
{

}

SubmitResult DX12RenderPass::Submit(const std::vector<std::shared_ptr<ILock>>& waitOnLock, ILock* notifyFence)
{

	return SubmitResult::Success;
}

SubmitResult DX12RenderPass::PresentPassToSwapChain() const
{
	return SubmitResult::Success;
}
