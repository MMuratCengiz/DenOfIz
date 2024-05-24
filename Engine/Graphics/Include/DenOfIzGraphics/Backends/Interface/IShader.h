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

#include "IResource.h"
#include <string>

namespace DenOfIz
{

enum class ShaderStage
{
	Vertex,
	TessellationControl,
	TessellationEvaluation,
	Geometry,
	Fragment,
	Compute,
	AllGraphics,
	All,
	Raygen,
	AnyHit,
	ClosestHit,
	Miss,
	Intersection,
	Callable,
	Task,
	Mesh,
};

struct Shader
{
	ShaderStage Stage;
	std::string Path;
};

struct CompiledShader
{
	ShaderStage Stage;
	std::vector<uint32_t> Data;
};

struct VertexInput
{
	uint32_t BoundBufferIndex;
	uint32_t Location;
	uint32_t Size;
	uint32_t Offset;
	uint32_t Stride;
	std::string Name;
	ImageFormat Format;
};

enum class UniformType
{
	Struct,
	Sampler
};

struct ShaderUniformInput
{
	ShaderStage Stage;
	uint32_t BoundDescriptorSet;
	uint32_t Location;
	uint32_t Binding;
	uint32_t ArraySize;
	uint32_t Size;
	UniformType Type;
	ImageFormat Format;
	std::string Name;
};

}
