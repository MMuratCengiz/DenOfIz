/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
