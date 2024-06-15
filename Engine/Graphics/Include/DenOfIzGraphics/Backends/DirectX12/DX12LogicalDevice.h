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

#include "DX12Context.h"
#include <DenOfIzGraphics/Backends/Interface/ILogicalDevice.h>
#include <DenOfIzCore/Common.h>

#include "Resource/DX12BufferResource.h"
#include "Resource/DX12ImageResource.h"
#include "Resource/DX12Fence.h"
#include "DX12DescriptorTable.h"
#include "DX12CommandList.h"

#include <dxgidebug.h>

namespace DenOfIz
{

class DX12LogicalDevice final : public ILogicalDevice
{
private:
	D3D_FEATURE_LEVEL m_minFeatureLevel = D3D_FEATURE_LEVEL_12_0;
	std::unique_ptr<DX12Context> m_context;
	DWORD m_dxgiFactoryFlags =  0;
public:
	DX12LogicalDevice();
	~DX12LogicalDevice() override;

	// Override methods
	void CreateDevice(GraphicsWindowHandle* window) override;
	std::vector<PhysicalDeviceInfo> ListPhysicalDevices() override;
	void LoadPhysicalDevice(const PhysicalDeviceInfo& device) override;
	inline bool IsDeviceLost() override {
		return m_context->IsDeviceLost;
	}

	std::unique_ptr<ICommandList> CreateCommandList(const CommandListCreateInfo& createInfo) override;
	std::unique_ptr<IPipeline> CreatePipeline(const PipelineCreateInfo& createInfo) override;
	std::unique_ptr<ISwapChain> CreateSwapChain(const SwapChainCreateInfo& createInfo) override;
	std::unique_ptr<IRootSignature> CreateRootSignature(const RootSignatureCreateInfo& createInfo) override;
	std::unique_ptr<IInputLayout> CreateInputLayout(const InputLayoutCreateInfo& createInfo) override;
	std::unique_ptr<IDescriptorTable> CreateDescriptorTable(const DescriptorTableCreateInfo& createInfo) override;
	std::unique_ptr<IFence> CreateFence() override;
	std::unique_ptr<ISemaphore> CreateSemaphore() override;
	std::unique_ptr<IBufferResource> CreateBufferResource(std::string name, const BufferCreateInfo& createInfo) override;
	std::unique_ptr<IImageResource> CreateImageResource(std::string name, const ImageCreateInfo& createInfo) override;
	void WaitIdle() override;
	// --
private:
	void CreateDeviceInfo(IDXGIAdapter1& adapter, PhysicalDeviceInfo& deviceInfo);
	void Dispose();
};

}

#endif
