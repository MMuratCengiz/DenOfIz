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

#include <DenOfIzGraphics/Backends/Common/ShaderCompiler.h>
#include <mutex>

namespace DenOfIz
{

enum class APIPreferenceWindows
{
	DirectX12,
	Vulkan
};

enum class APIPreferenceOSX
{
	Metal,
	Vulkan
};

enum class APIPreferenceLinux
{
	Vulkan
};

struct APIPreference
{
	APIPreferenceWindows Windows = APIPreferenceWindows::DirectX12;
	APIPreferenceOSX OSX = APIPreferenceOSX::Metal;
	APIPreferenceLinux Linux = APIPreferenceLinux::Vulkan;
};

class GfxGlobal
{
private:
	static std::unique_ptr<GfxGlobal> s_instance;
	static std::mutex s_mutex;
	ShaderCompiler m_shaderCompiler;
	APIPreference m_apiPreference;
public:
	static GfxGlobal* GetInstance();
	static void Destroy();

	const ShaderCompiler& GetShaderCompiler() const
	{
		return m_shaderCompiler;
	}

	void SetAPIPreference(APIPreference preference)
	{
		m_apiPreference = preference;
	}

	const APIPreference& GetAPIPreference() const
	{
		return m_apiPreference;
	}
};

}