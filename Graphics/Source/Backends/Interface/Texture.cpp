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

#include "DenOfIzGraphicsInternal/Backends/Interface/ITexture.h"

#define TEXTURE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, handle )
#define SAMPLER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ISampler, handle )

extern "C"
{

    void DenOfIz_TextureResource_GetFormat( DenOfIz_Texture texture, DenOfIz_Format *outFormat )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) || outFormat == NULL )
        {
            return;
        }
        *outFormat = TEXTURE_IMPL( texture )->GetFormat( );
    }

    void DenOfIz_TextureResource_Destroy( DenOfIz_Texture texture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return;
        }
        delete TEXTURE_IMPL( texture );
    }

    void DenOfIz_Sampler_Destroy( DenOfIz_Sampler sampler )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( sampler ) )
        {
            return;
        }
        delete SAMPLER_IMPL( sampler );
    }
}
