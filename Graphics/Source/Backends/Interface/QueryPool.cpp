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

#include "DenOfIzGraphicsInternal/Backends/Interface/IQueryPool.h"

#define QUERY_POOL_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::IQueryPool, handle )

extern "C"
{

    void DenOfIz_QueryPool_GetType( DenOfIz_QueryPool queryPool, DenOfIz_QueryType *outType )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( queryPool ) || outType == NULL )
        {
            return;
        }
        *outType = QUERY_POOL_IMPL( queryPool )->GetType( );
    }

    void DenOfIz_QueryPool_GetNumQueries( DenOfIz_QueryPool queryPool, uint32_t *outNumQueries )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( queryPool ) || outNumQueries == NULL )
        {
            return;
        }
        *outNumQueries = QUERY_POOL_IMPL( queryPool )->GetNumQueries( );
    }

    void DenOfIz_QueryPool_GetTimestampFrequency( DenOfIz_QueryPool queryPool, double *outFrequency )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( queryPool ) || outFrequency == NULL )
        {
            return;
        }
        *outFrequency = QUERY_POOL_IMPL( queryPool )->GetTimestampFrequency( );
    }

    void DenOfIz_QueryPool_GetQueryData( DenOfIz_QueryPool queryPool, uint32_t queryIndex, DenOfIz_QueryData *outData )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( queryPool ) || outData == NULL )
        {
            return;
        }
        *outData = QUERY_POOL_IMPL( queryPool )->GetQueryData( queryIndex );
    }

    void DenOfIz_QueryPool_Destroy( DenOfIz_QueryPool queryPool )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( queryPool ) )
        {
            return;
        }
        delete QUERY_POOL_IMPL( queryPool );
    }
}
