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

#include <DenOfIzGraphics/Backends/Interface/IPipeline.h>
#include "DX12Context.h"

namespace DenOfIz
{

class DX12Pipeline : public IPipeline
{
private:
	DX12Context* m_context;
	ComPtr<ID3D12PipelineState> m_pipelineState;
	PipelineCreateInfo m_createInfo;
public:
	DX12Pipeline(DX12Context* context, const PipelineCreateInfo& info);
	~DX12Pipeline() override;
private:
	void SetMSAASampleCount(const PipelineCreateInfo& createInfo, D3D12_GRAPHICS_PIPELINE_STATE_DESC& psoDesc) const;
};

}