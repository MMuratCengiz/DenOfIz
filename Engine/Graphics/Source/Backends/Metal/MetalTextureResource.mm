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

#include <DenOfIzGraphics/Backends/Metal/MetalTextureResource.h>

using namespace DenOfIz;

MetalTextureResource::MetalTextureResource( MetalContext *context, const TextureDesc &desc ) : ITextureResource( desc ), m_context( context )
{
    m_context = context;
    m_desc    = desc;

    MTLTextureDescriptor *descriptor = nil;

    MTLPixelFormat format = MTLPixelFormatInvalid;
    switch ( m_desc.Format )
    {

    }
    if ( m_desc.ArraySize > 1 )
    {
    }
    else
    {
        if ( m_depth > 1 )
        {

        }
        else
        {
            descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:MTLPixelFormatR8Unorm width:m_desc.Width height:m_desc.Height mipmapped:m_desc.MipLevels > 1];
        }
    }

    m_texture = [m_context->Device newTextureWithDescriptor:descriptor];
}

MetalTextureResource::MetalTextureResource( MetalContext *context, const TextureDesc &desc, id<MTLTexture> texture ) :
    ITextureResource( m_desc ), m_context( context ), m_texture( texture )
{
    m_context = context;
    m_desc    = desc;
    m_texture = texture;
}
