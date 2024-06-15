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

#include <DenOfIzGraphics/Backends/Interface/IResource.h>
#include <DenOfIzGraphics/Backends/DirectX12/DX12Context.h>

namespace DenOfIz
{

class DX12ImageResource : public IImageResource
{
private:
	DX12Context* m_context;
	ImageCreateInfo m_createInfo;
	ID3D12Resource2* m_resource;
	D3D12_CPU_DESCRIPTOR_HANDLE m_cpuHandle;
	bool isExternalResource = false;
public:
	DX12ImageResource(DX12Context* context, const ImageCreateInfo& createInfo);
	DX12ImageResource(ID3D12Resource2* resource, const D3D12_CPU_DESCRIPTOR_HANDLE& cpuHandle);
	void AttachSampler(SamplerCreateInfo& info) override;
	void Deallocate() override;

	ID3D12Resource* GetResource() const { return m_resource; }
	const D3D12_CPU_DESCRIPTOR_HANDLE& GetCpuHandle() const { return m_cpuHandle; };
protected:
	void Allocate(const void* data) override;
};

}