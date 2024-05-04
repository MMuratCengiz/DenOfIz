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
#include <DenOfIzGraphics/Backends/Interface/IShader.h>
#include <DenOfIzGraphics/Backends/Interface/IResource.h>
#include <DenOfIzGraphics/Backends/Common/ShaderCompiler.h>
#include "spirv_hlsl.hpp"

namespace DenOfIz
{

struct ShaderVarType
{
	ImageFormat Format;
	uint32_t Size;
};

struct SpvDecoration
{
	spirv_cross::SPIRType Type;
	uint32_t Offset; // This offset is mostly meant for struct members
	uint32_t Set;
	uint32_t Location;
	uint32_t Binding;
	uint32_t ArraySize;
	uint32_t Size;
	std::string Name;
	std::vector<SpvDecoration> Children;
};

struct PushConstant
{
	ShaderStage Stage;
	uint32_t Size;
	uint32_t Offset;
	std::string Name;
	std::vector<SpvDecoration> Children;
};

class SpvProgram
{
private:
	std::vector<VertexInput> vertexInputs;
	std::vector<ShaderUniformInput> uniformInputs;
	std::vector<PushConstant> pushConstants;

public:
	std::vector<CompiledShader> Shaders;

	explicit SpvProgram(std::vector<CompiledShader> shaders);

	inline const std::vector<VertexInput> VertexInputs()
	{
		return vertexInputs;
	}

	inline const std::vector<ShaderUniformInput> UniformInputs()
	{
		return uniformInputs;
	}

	inline const std::vector<PushConstant> PushConstants()
	{
		return pushConstants;
	}

private:
	void OnEachShader(const CompiledShader& shaderInfo, const bool& first);
	void CreatePushConstant(const spirv_cross::Compiler& compiler, const spirv_cross::Resource resource, const ShaderStage stage);
	void CreateVertexInput(const uint32_t& offset, const ShaderVarType& type, const SpvDecoration& decoration, const uint32_t& Size);
	void
	CreateUniformInput(const spirv_cross::Compiler& compiler, const UniformType& uniformType, const spirv_cross::Resource resource, const ShaderStage stage);
	static ShaderVarType SpvTypeToCustomType(const spirv_cross::SPIRType& type);
	SpvDecoration GetDecoration(const spirv_cross::Compiler& compiler, const spirv_cross::Resource& resource);
	void AddResourceToInput(const UniformType& uniformType, const ShaderStage& stage, const SpvDecoration& decoration);
	uint32_t GetTypeArraySize(SpvDecoration& decoration) const;
};

}