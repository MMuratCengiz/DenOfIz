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

#include "CommonData.h"

namespace DenOfIz
{

    enum class StepRate
    {
        PerVertex,
        PerInstance,
    };

    enum class Semantic
    {
        Position,
        Normal,
        Color,
        Tangent,
        Binormal,
        Bitangent,
        BlendJoints,
        BlendWeights,
        TextureCoordinate,
    };

    struct InputLayoutElement
    {
        Semantic Semantic;
        uint32_t SemanticIndex;
        ImageFormat Format;
    };

    /**
     * @brief Describes a group of input elements that are bound to a single vertex buffer.
     */
    struct InputGroup
    {
        std::vector<InputLayoutElement> Elements;
        StepRate StepRate;
    };

    /**
     * @brief Describes the input layout of a the input assembler stage. The order the groups are added determines the buffer binding.
     */
    struct InputLayoutCreateInfo
    {
        std::vector<InputGroup> InputGroups;
    };

    class IInputLayout
    {
    public:
        virtual ~IInputLayout() = default;
    };

} // namespace DenOfIz
