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

#include "DenOfIzGraphicsInternal/Backends/Interface/IBindGroup.h"

#define BIND_GROUP_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::IBindGroup, handle )
#define BUFFER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, handle )
#define TEXTURE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, handle )
#define SAMPLER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ISampler, handle )
#define TLAS_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ITopLevelAS, handle )

extern "C"
{

    void DenOfIz_BindGroup_BeginUpdate( DenOfIz_BindGroup bindGroup )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bindGroup ) )
        {
            return;
        }
        BIND_GROUP_IMPL( bindGroup )->BeginUpdate( );
    }

    void DenOfIz_BindGroup_Cbv( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Buffer resource )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bindGroup ) )
        {
            return;
        }
        BIND_GROUP_IMPL( bindGroup )->Cbv( binding, BUFFER_IMPL( resource ) );
    }

    void DenOfIz_BindGroup_CbvWithDesc( DenOfIz_BindGroup bindGroup, const DenOfIz_BindBufferDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bindGroup ) || desc == NULL )
        {
            return;
        }
        BIND_GROUP_IMPL( bindGroup )->Cbv( desc->Binding, BUFFER_IMPL( desc->Resource ), desc->ResourceOffset );
    }

    void DenOfIz_BindGroup_SrvBuffer( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Buffer resource )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bindGroup ) )
        {
            return;
        }
        BIND_GROUP_IMPL( bindGroup )->Srv( binding, BUFFER_IMPL( resource ) );
    }

    void DenOfIz_BindGroup_SrvBufferWithDesc( DenOfIz_BindGroup bindGroup, const DenOfIz_BindBufferDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bindGroup ) || desc == NULL )
        {
            return;
        }
        BIND_GROUP_IMPL( bindGroup )->Srv( desc->Binding, BUFFER_IMPL( desc->Resource ), desc->ResourceOffset );
    }

    void DenOfIz_BindGroup_SrvTopLevelAS( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_TopLevelAS accelerationStructure )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bindGroup ) )
        {
            return;
        }
        BIND_GROUP_IMPL( bindGroup )->Srv( binding, TLAS_IMPL( accelerationStructure ) );
    }

    void DenOfIz_BindGroup_SrvTexture( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Texture resource )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bindGroup ) )
        {
            return;
        }
        BIND_GROUP_IMPL( bindGroup )->Srv( binding, TEXTURE_IMPL( resource ) );
    }

    void DenOfIz_BindGroup_SrvTextureMip( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Texture resource, uint32_t mipLevel )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bindGroup ) )
        {
            return;
        }
        BIND_GROUP_IMPL( bindGroup )->Srv( binding, TEXTURE_IMPL( resource ), mipLevel );
    }

    void DenOfIz_BindGroup_SrvArray( DenOfIz_BindGroup bindGroup, uint32_t binding, const DenOfIz_TextureArray *resources )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bindGroup ) || resources == NULL )
        {
            return;
        }
        BIND_GROUP_IMPL( bindGroup )->SrvArray( binding, *resources );
    }

    void DenOfIz_BindGroup_SrvArrayIndex( DenOfIz_BindGroup bindGroup, uint32_t binding, uint32_t arrayIndex, DenOfIz_Texture resource, DenOfIz_BindGroup *outBindGroup )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bindGroup ) )
        {
            return;
        }
        BIND_GROUP_IMPL( bindGroup )->SrvArrayIndex( binding, arrayIndex, TEXTURE_IMPL( resource ) );
    }

    void DenOfIz_BindGroup_UavBuffer( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Buffer resource )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bindGroup ) )
        {
            return;
        }
        BIND_GROUP_IMPL( bindGroup )->Uav( binding, BUFFER_IMPL( resource ) );
    }

    void DenOfIz_BindGroup_UavBufferWithDesc( DenOfIz_BindGroup bindGroup, const DenOfIz_BindBufferDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bindGroup ) || desc == NULL )
        {
            return;
        }
        BIND_GROUP_IMPL( bindGroup )->Uav( desc->Binding, BUFFER_IMPL( desc->Resource ), desc->ResourceOffset );
    }

    void DenOfIz_BindGroup_UavTexture( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Texture resource )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bindGroup ) )
        {
            return;
        }
        BIND_GROUP_IMPL( bindGroup )->Uav( binding, TEXTURE_IMPL( resource ) );
    }

    void DenOfIz_BindGroup_UavTextureMip( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Texture resource, uint32_t mipLevel )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bindGroup ) )
        {
            return;
        }
        BIND_GROUP_IMPL( bindGroup )->Uav( binding, TEXTURE_IMPL( resource ), mipLevel );
    }

    void DenOfIz_BindGroup_Sampler( DenOfIz_BindGroup bindGroup, uint32_t binding, DenOfIz_Sampler sampler )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bindGroup ) )
        {
            return;
        }
        BIND_GROUP_IMPL( bindGroup )->Sampler( binding, SAMPLER_IMPL( sampler ) );
    }

    void DenOfIz_BindGroup_EndUpdate( DenOfIz_BindGroup bindGroup )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bindGroup ) )
        {
            return;
        }
        BIND_GROUP_IMPL( bindGroup )->EndUpdate( );
    }

    void DenOfIz_BindGroup_Destroy( DenOfIz_BindGroup bindGroup )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bindGroup ) )
        {
            return;
        }
        delete BIND_GROUP_IMPL( bindGroup );
    }
}
