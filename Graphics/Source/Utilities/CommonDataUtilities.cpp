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

#include "DenOfIzGraphicsInternal/Utilities/CommonDataUtilities.h"

using namespace DenOfIz;

std::string DenOfIz::ResourceBindingSlotToString( const DenOfIz_ResourceBindingSlot &slot )
{
    std::string typeString;
    switch ( slot.Type )
    {
    case DENOFIZ_RESOURCE_BINDING_TYPE_CONSTANT_BUFFER:
        typeString = "b";
        break;
    case DENOFIZ_RESOURCE_BINDING_TYPE_SHADER_RESOURCE:
        typeString = "t";
        break;
    case DENOFIZ_RESOURCE_BINDING_TYPE_UNORDERED_ACCESS:
        typeString = "u";
        break;
    case DENOFIZ_RESOURCE_BINDING_TYPE_SAMPLER:
        typeString = "s";
        break;
    }

    return "(" + typeString + std::to_string( slot.Binding ) + ", space" + std::to_string( slot.RegisterSpace ) + ")";
}
