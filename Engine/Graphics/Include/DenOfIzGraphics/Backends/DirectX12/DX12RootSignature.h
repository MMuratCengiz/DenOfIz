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

namespace DenOfIz
{

class DX12RootSignature : public IRootSignature
{
private:
	DX12Context* m_context;
	RootSignatureCreateInfo m_createInfo;
	ComPtr<ID3D12RootSignature> m_rootSignature;

	std::vector<D3D12_ROOT_PARAMETER> m_rootParameters;
public:
	DX12RootSignature(DX12Context* context, const RootSignatureCreateInfo& createInfo);
	~DX12RootSignature() override;
protected:
	void AddResourceBindingInternal(const ResourceBinding& binding) override;
	void AddRootConstantInternal(const RootConstantBinding& rootConstant) override;
	void CreateInternal() override;
};

}