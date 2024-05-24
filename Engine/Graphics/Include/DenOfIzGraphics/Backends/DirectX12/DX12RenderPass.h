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
#ifdef BUILD_DX12

#include <DenOfIzGraphics/Backends/Interface/IRenderPass.h>
#include <DenOfIzGraphics/Backends/DirectX12/DX12Context.h>

namespace DenOfIz
{

class DX12RenderPass : public IRenderPass
{
private:
	DX12Context* m_context;
	RenderPassCreateInfo m_createInfo;
public:
	DX12RenderPass(DX12Context* context, const RenderPassCreateInfo& createInfo);
	void UpdateViewport(const uint32_t& width, const uint32_t& height) override;
	void SetDepthBias(float constant, float clamp, float slope) const override;
	void Begin(std::array<float, 4> clearColor) override;
	void BindPipeline(IPipeline* pipeline) override;
	void BindResource(IResource* resource) override;
	void BindIndexBuffer(IBufferResource* resource) override;
	void BindVertexBuffer(IBufferResource* resource) const override;
	void Draw(const uint32_t& instanceCount, const uint32_t& vertexCount) const override;
	SubmitResult Submit(const std::vector<std::shared_ptr<ILock>>& waitOnLock, ILock* notifyFence) override;
	~DX12RenderPass();
private:
	SubmitResult PresentPassToSwapChain() const;
};

}

#endif