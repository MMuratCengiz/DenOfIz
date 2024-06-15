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

#include "../TFCommon.h"
#include <DenOfIzCore/Time.h>
#include <memory>

namespace DenOfIz
{

class TriangleRenderer : public TFCommon
{
private:

	std::vector<float> m_triangle{ 1.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f,
								   -1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f,
								   0.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f };
	std::vector<uint32_t> m_indices{ 0, 1, 2 };

	Shader* mp_basicShader;
	RootSignature* mp_rootSignature;
	DescriptorSet* mp_descriptorSet;
	Buffer* mp_deltaTimeBuffer[g_DataBufferCount];
	Buffer* mp_vertexBuffer;
	Buffer* mp_indexBuffer;
	Pipeline* mp_pipeline;

	std::unique_ptr<Time> m_time = std::make_unique<Time>();
public:
	TriangleRenderer() = default;
	void Update(float deltaTime) override;
	void Draw() override;
	bool Init() override;
	void Exit() override;
	bool Load(ReloadDesc* pReloadDesc) override;
	void Unload(ReloadDesc* pReloadDesc) override;
};

}