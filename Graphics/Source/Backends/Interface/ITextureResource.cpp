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

#include "DenOfIzGraphics/Backends/Interface/ITextureResource.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

void DenOfIz::ValidateTextureDesc( TextureDesc &desc )
{
    if ( desc.Descriptor.IsSet( ResourceDescriptor::RWTexture ) && desc.MSAASampleCount != MSAASampleCount::_0 )
    {
        spdlog::warn("MSAA textures cannot be used as UAVs. Resetting MSAASampleCount to 0.");
        desc.MSAASampleCount = MSAASampleCount::_0;
    }

    if ( desc.MSAASampleCount != MSAASampleCount::_0 && desc.MipLevels > 1 )
    {
        spdlog::warn("Mip mapped textures cannot be sampled. Resetting MSAASampleCount to 0.");
        desc.MSAASampleCount = MSAASampleCount::_0;
    }

    if ( desc.ArraySize > 1 && desc.Depth > 1 )
    {
        spdlog::warn("Array textures cannot have depth. Resetting depth to 1.");
        desc.Depth = 1;
    }

    if ( !desc.Descriptor.IsSet( ResourceDescriptor::RWTexture ) && !desc.Descriptor.IsSet( ResourceDescriptor::Texture ) &&
         !desc.Descriptor.IsSet( ResourceDescriptor::TextureCube ) )
    {
        spdlog::warn("Descriptor does not specify a texture: [ResourceDescriptor::(RWTexture/Texture/TextureCube)].");
    }

    if ( desc.Descriptor.IsSet( ResourceDescriptor::TextureCube ) && desc.ArraySize != 6 )
    {
        spdlog::warn("TextureCube does not have an array size of 6. ");
    }

    if ( desc.Descriptor.IsSet( ResourceDescriptor::TextureCube ) && desc.Height != desc.Width )
    {
        spdlog::warn("TextureCube does not have equal width and height.");
    }
}
