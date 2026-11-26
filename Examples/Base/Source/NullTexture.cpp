/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
        DenOfIz_Texture_Destroy( m_texture );
    }
}

DenOfIz_Texture NullTexture::Texture( ) const
{
    return m_texture;
}
