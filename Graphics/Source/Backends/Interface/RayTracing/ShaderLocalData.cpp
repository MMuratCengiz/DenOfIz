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

#include "DenOfIzGraphicsInternal/Backends/Interface/IBuffer.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/ITexture.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/RayTracing/IShaderLocalData.h"

#define SHADER_LOCAL_DATA_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::IShaderLocalData, handle )
#define BUFFER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, handle )
#define TEXTURE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ITexture, handle )
#define SAMPLER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ISampler, handle )

extern "C"
{

    void DenOfIz_ShaderLocalData_Begin( DenOfIz_ShaderLocalData localData )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( localData ) )
        {
            return;
        }
        SHADER_LOCAL_DATA_IMPL( localData )->Begin( );
    }

    void DenOfIz_ShaderLocalData_CbvBuffer( DenOfIz_ShaderLocalData localData, uint32_t binding, DenOfIz_Buffer bufferResource )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( localData ) || !DENOFIZ_HANDLE_IS_VALID( bufferResource ) )
        {
            return;
        }
        SHADER_LOCAL_DATA_IMPL( localData )->Cbv( binding, BUFFER_IMPL( bufferResource ) );
    }

    void DenOfIz_ShaderLocalData_CbvData( DenOfIz_ShaderLocalData localData, uint32_t binding, const DenOfIz_ByteArrayView *data )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( localData ) || data == NULL )
        {
            return;
        }
        SHADER_LOCAL_DATA_IMPL( localData )->Cbv( binding, *data );
    }

    void DenOfIz_ShaderLocalData_SrvBuffer( DenOfIz_ShaderLocalData localData, uint32_t binding, DenOfIz_Buffer bufferResource )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( localData ) || !DENOFIZ_HANDLE_IS_VALID( bufferResource ) )
        {
            return;
        }
        SHADER_LOCAL_DATA_IMPL( localData )->Srv( binding, BUFFER_IMPL( bufferResource ) );
    }

    void DenOfIz_ShaderLocalData_SrvTexture( DenOfIz_ShaderLocalData localData, uint32_t binding, DenOfIz_Texture textureResource )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( localData ) || !DENOFIZ_HANDLE_IS_VALID( textureResource ) )
        {
            return;
        }
        SHADER_LOCAL_DATA_IMPL( localData )->Srv( binding, TEXTURE_IMPL( textureResource ) );
    }

    void DenOfIz_ShaderLocalData_UavBuffer( DenOfIz_ShaderLocalData localData, uint32_t binding, DenOfIz_Buffer bufferResource )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( localData ) || !DENOFIZ_HANDLE_IS_VALID( bufferResource ) )
        {
            return;
        }
        SHADER_LOCAL_DATA_IMPL( localData )->Uav( binding, BUFFER_IMPL( bufferResource ) );
    }

    void DenOfIz_ShaderLocalData_UavTexture( DenOfIz_ShaderLocalData localData, uint32_t binding, DenOfIz_Texture textureResource )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( localData ) || !DENOFIZ_HANDLE_IS_VALID( textureResource ) )
        {
            return;
        }
        SHADER_LOCAL_DATA_IMPL( localData )->Uav( binding, TEXTURE_IMPL( textureResource ) );
    }

    void DenOfIz_ShaderLocalData_Sampler( DenOfIz_ShaderLocalData localData, uint32_t binding, DenOfIz_Sampler sampler )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( localData ) || !DENOFIZ_HANDLE_IS_VALID( sampler ) )
        {
            return;
        }
        SHADER_LOCAL_DATA_IMPL( localData )->Sampler( binding, SAMPLER_IMPL( sampler ) );
    }

    void DenOfIz_ShaderLocalData_End( DenOfIz_ShaderLocalData localData )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( localData ) )
        {
            return;
        }
        SHADER_LOCAL_DATA_IMPL( localData )->End( );
    }

    void DenOfIz_ShaderLocalData_Destroy( DenOfIz_ShaderLocalData localData )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( localData ) )
        {
            return;
        }
        delete SHADER_LOCAL_DATA_IMPL( localData );
    }
}
