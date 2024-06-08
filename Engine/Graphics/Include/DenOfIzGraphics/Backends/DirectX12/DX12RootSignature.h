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

#include <DenOfIzGraphics/Backends/Interface/IRootSignature.h>
#include "DX12Context.h"
#include "DX12EnumConverter.h"

namespace DenOfIz
{

class DX12RootSignature : public IRootSignature
{
private:
	D3D_ROOT_SIGNATURE_VERSION m_rootSignatureVersion;
	DX12Context* m_context;
	RootSignatureCreateInfo m_createInfo;
	ComPtr<ID3D12RootSignature> m_rootSignature;

	std::vector<CD3DX12_ROOT_PARAMETER1> m_rootParameters;
	std::vector<CD3DX12_ROOT_PARAMETER1> m_rootConstants;
	std::vector<CD3DX12_DESCRIPTOR_RANGE1> m_descriptorRanges;

	std::unordered_set<D3D12_SHADER_VISIBILITY> m_descriptorRangesShaderVisibilities;
public:
	DX12RootSignature(DX12Context* context, const RootSignatureCreateInfo& createInfo);
	inline ID3D12RootSignature* GetRootSignature() const { return m_rootSignature.Get(); }
	~DX12RootSignature() override;
protected:
	void AddResourceBindingInternal(const ResourceBinding& binding) override;
	void AddRootConstantInternal(const RootConstantBinding& rootConstant) override;
	void CreateInternal() override;

};

}