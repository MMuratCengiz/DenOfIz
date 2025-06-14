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
        BlendIndices,
        BlendWeight,
        TextureCoordinate,
    };

    struct DZ_API InputLayoutElementDesc
    {
        InteropString Semantic;
        uint32_t      SemanticIndex;
        Format        Format;
    };
    template class DZ_API InteropArray<InputLayoutElementDesc>;

    struct DZ_API InputLayoutElementDescArray
    {
        InputLayoutElementDesc* Elements;
        uint32_t                NumElements;
    };

    /**
     * @brief Describes a group of input elements that are bound to a single vertex buffer.
     */
    struct DZ_API InputGroupDesc
    {
        StepRate                       StepRate = StepRate::PerVertex;
        InputLayoutElementDescArray    Elements;
    };
    template class DZ_API InteropArray<InputGroupDesc>;

    struct DZ_API InputGroupDescArray
    {
        InputGroupDesc* Elements;
        uint32_t        NumElements;
    };

    /**
     * @brief Describes the input layout of a the input assembler stage. The order the groups are added determines the buffer binding.
     */
    struct DZ_API InputLayoutDesc
    {
        InputGroupDescArray InputGroups;
    };

    class DZ_API IInputLayout
    {
    public:
        virtual ~IInputLayout( ) = default;
    };

} // namespace DenOfIz
