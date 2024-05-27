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

#include <DenOfIzCore/Common.h>
#include "IResource.h"
#include "IShader.h"
#include <boost/optional.hpp>

namespace DenOfIz
{

enum class RootSignatureType
{
	Graphics,
	Compute
};

// Static = 0th set, Dynamic = 1, PerDraw = 2
enum class ResourceUpdateFrequency : uint32_t
{
	Static = 0,
	Dynamic = 1,
	PerDraw = 2
};

struct RootSignatureCreateInfo
{
	// Todo
};

struct ResourceBinding
{
	std::string Name;
	uint32_t Binding;
	ResourceBindingType Type;
	ResourceUpdateFrequency Frequency;
	std::vector<ShaderStage> Stages;
	int ArraySize = 1;

	uint32_t FrequencyUInt() const
	{
		return static_cast<uint32_t>(Frequency);
	}
};

struct RootConstantBinding
{
	std::string Name;
	uint32_t Order;
	int Size;
	std::vector<ShaderStage> Stages;
};

class IRootSignature
{
protected:
	uint32_t m_resourceCount = 0;
	std::vector<uint32_t> m_resourceCountPerFrequency = { 0, 0, 0 };
	std::unordered_map<std::string, ResourceBinding> m_resourceBindingMap;
	std::unordered_map<std::string, RootConstantBinding> m_rootConstantMap;
	bool m_created = false;
public:
	virtual ~IRootSignature() = default;

	void AddResourceBinding(const ResourceBinding& binding) {
		ValidateNotCreated();
		m_resourceBindingMap[binding.Name] = binding;
		m_resourceCount++;
		uint32_t frequency = binding.FrequencyUInt();
		m_resourceCountPerFrequency[frequency] = std::max(m_resourceCountPerFrequency[frequency], binding.Binding + 1);
		AddResourceBindingInternal(binding);
	}

	void AddRootConstant(const RootConstantBinding& rootConstant) {
		ValidateNotCreated();
		m_rootConstantMap[rootConstant.Name] = rootConstant;
		AddRootConstantInternal(rootConstant);
	}

	void Create() {
		ValidateNotCreated();
		m_created = true;
		CreateInternal();
	}

	inline uint32_t GetResourceCount() const { return m_resourceCount; }
	inline uint32_t GetResourceCount(ResourceUpdateFrequency frequency) const { return m_resourceCountPerFrequency[static_cast<uint32_t>(frequency)]; }

	inline boost::optional<ResourceBinding> GetResourceBinding(std::string name) const
	{
		if (m_resourceBindingMap.find(name) == m_resourceBindingMap.end())
		{
			return boost::none;
		}

		return boost::make_optional(m_resourceBindingMap.at(name));
	}

	inline boost::optional<RootConstantBinding> GetRootConstantBinding(std::string name)
	{
		if (m_rootConstantMap.find(name) == m_rootConstantMap.end())
		{
			return boost::none;
		}

		return boost::make_optional(m_rootConstantMap.at(name));
	}
protected:
	virtual void AddResourceBindingInternal(const ResourceBinding& binding) = 0;
	virtual void AddRootConstantInternal(const RootConstantBinding& rootConstant) = 0;

	virtual void CreateInternal() = 0;
private:
	inline void ValidateNotCreated()
	{
		assertm(!m_created, "Root signature is already created. Changing the root signature after creation could cause undefined behavior.");
	}
};

}