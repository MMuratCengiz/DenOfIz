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

#include <DenOfIzGraphics/Backends/Interface/IInputLayout.h>

using namespace DenOfIz;

Semantic DenOfIz::SemanticFromString( const std::string &semantic )
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
