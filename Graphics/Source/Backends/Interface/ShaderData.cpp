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

#include "DenOfIzGraphics/Backends/Interface/ShaderData.h"
#include <cstdlib> // for std::realloc

using namespace DenOfIz;

uint32_t ResourceBindingSlot::Key( ) const
{
    return static_cast<uint32_t>( Type ) * 1000 + RegisterSpace * 100 + Binding;
}

InteropString ResourceBindingSlot::ToInteropString( ) const
{
    std::string typeString;
    switch ( Type )
    {
    case ResourceBindingType::ConstantBuffer:
        typeString = "b";
        break;
    case ResourceBindingType::ShaderResource:
        typeString = "t";
        break;
    case ResourceBindingType::UnorderedAccess:
        typeString = "u";
        break;
    case ResourceBindingType::Sampler:
        typeString = "s";
        break;
    }

    return InteropString( )
        .Append( "(" )
        .Append( typeString.c_str( ) )
        .Append( std::to_string( Binding ).c_str( ) )
        .Append( ", space" )
        .Append( std::to_string( RegisterSpace ).c_str( ) )
        .Append( ")" );
}
