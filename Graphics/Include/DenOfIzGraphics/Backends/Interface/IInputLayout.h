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

    static Semantic SemanticFromString( const std::string &semantic )
    {
        if ( semantic == "POSITION" )
        {
            return Semantic::Position;
        }
        if ( semantic == "NORMAL" )
        {
            return Semantic::Normal;
        }
        if ( semantic == "COLOR" )
        {
            return Semantic::Color;
        }
        if ( semantic == "TANGENT" )
        {
            return Semantic::Tangent;
        }
        if ( semantic == "BINORMAL" )
        {
            return Semantic::Binormal;
        }
        if ( semantic == "BITANGENT" )
        {
            return Semantic::Bitangent;
        }
        if ( semantic == "BLENDJOINTS" )
        {
            return Semantic::BlendJoints;
        }
        if ( semantic == "BLENDWEIGHTS" )
        {
            return Semantic::BlendWeights;
        }
        if ( semantic.starts_with( "TEXCOORD" ) )
        {
            return Semantic::TextureCoordinate;
        }

        LOG( ERROR ) << "Unknown semantic: " << semantic;
        return Semantic::Position;
    }

    struct InputLayoutElementDesc
    {
        Semantic Semantic;
        uint32_t SemanticIndex;
        Format   Format;
    };

    /**
     * @brief Describes a group of input elements that are bound to a single vertex buffer.
     */
    struct InputGroupDesc
    {
        std::vector<InputLayoutElementDesc> Elements;
        StepRate                            StepRate = StepRate::PerVertex;
    };

    /**
     * @brief Describes the input layout of a the input assembler stage. The order the groups are added determines the buffer binding.
     */
    struct InputLayoutDesc
    {
        std::vector<InputGroupDesc> InputGroups;
    };

    class IInputLayout
    {
    public:
        virtual ~IInputLayout( ) = default;
    };

} // namespace DenOfIz
