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

#include "DenOfIzExamples/NullTexture.h"

using namespace DenOfIz;

NullTexture::NullTexture( DenOfIz_LogicalDevice device )
{
    DenOfIz_TextureDesc desc{ };
    desc.Width     = 1;
    desc.Height    = 1;
    desc.Depth     = 1;
    desc.ArraySize = 1;
    desc.MipLevels = 1;
    desc.Format    = DENOFIZ_FORMAT_R8G8B8A8_UNORM;
    desc.DebugName = DENOFIZ_STRING( "NullTexture" );
    desc.Usage     = DENOFIZ_TEXTURE_USAGE_TEXTURE_BINDING_BIT;
    DenOfIz_LogicalDevice_CreateTexture( device, &desc, &m_texture );
}

NullTexture::~NullTexture( )
{
    if ( DENOFIZ_HANDLE_IS_VALID( m_texture ) )
    {
        DenOfIz_TextureResource_Destroy( m_texture );
    }
}

DenOfIz_Texture NullTexture::Texture( ) const
{
    return m_texture;
}
