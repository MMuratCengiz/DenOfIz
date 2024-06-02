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
#include "DirectXHelpers.h"

#include "Resource/DX12Fence.h"

namespace DenOfIz
{

class DX12LogicalDevice final : public ILogicalDevice
{
private:
	D3D_FEATURE_LEVEL m_minFeatureLevel = D3D_FEATURE_LEVEL_12_0;
	std::unique_ptr<DX12Context> m_context;
	DWORD m_dxgiFactoryFlags;
public:
	DX12LogicalDevice();
	~DX12LogicalDevice() override;

	// Override methods
	void CreateDevice(SDL_Window* window) override;
	std::vector<PhysicalDeviceInfo> ListPhysicalDevices() override;
	void LoadPhysicalDevice(const PhysicalDeviceInfo& device) override;
	inline bool IsDeviceLost() override {
		return m_context->IsDeviceLost;
	}
	void WaitIdle() override;
	// --
private:
	void CreateDeviceInfo(IDXGIAdapter1& adapter, PhysicalDeviceInfo& deviceInfo);
	void CreateSwapChain();
	void Dispose();
};

}

#endif
