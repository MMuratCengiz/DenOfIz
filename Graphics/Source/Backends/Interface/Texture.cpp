/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/Interface/ITexture.h"

#define TEXTURE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, handle )
#define SAMPLER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ISampler, handle )

extern "C"
{

    void DenOfIz_Texture_GetFormat( DenOfIz_Texture texture, DenOfIz_Format *outFormat )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) || outFormat == NULL )
        {
            return;
        }
        *outFormat = TEXTURE_IMPL( texture )->GetFormat( );
    }

    void DenOfIz_Texture_GetDesc( DenOfIz_Texture texture, DenOfIz_TextureDesc *outDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) || outDesc == NULL )
        {
            return;
        }
        *outDesc = TEXTURE_IMPL( texture )->GetDesc( );
    }

    void DenOfIz_Texture_Destroy( DenOfIz_Texture texture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return;
        }
        DenOfIz::DeferredDestroyQueue::Destroy( TEXTURE_IMPL( texture ) );
    }

    void DenOfIz_Sampler_Destroy( DenOfIz_Sampler sampler )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( sampler ) )
        {
            return;
        }
        DenOfIz::DeferredDestroyQueue::Destroy( SAMPLER_IMPL( sampler ) );
    }
}
