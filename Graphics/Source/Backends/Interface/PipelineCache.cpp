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

#include "DenOfIzGraphicsInternal/Backends/Interface/IPipelineCache.h"

#define PIPELINE_CACHE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::IPipelineCache, handle )

extern "C"
{

    void DenOfIz_PipelineCache_GetDataNumBytes( DenOfIz_PipelineCache cache, size_t *outNumBytes )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( cache ) || outNumBytes == NULL )
        {
            return;
        }
        *outNumBytes = PIPELINE_CACHE_IMPL( cache )->GetDataNumBytes( );
    }

    void DenOfIz_PipelineCache_GetData( DenOfIz_PipelineCache cache, DenOfIz_ByteArray *data )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( cache ) || data == NULL )
        {
            return;
        }
        PIPELINE_CACHE_IMPL( cache )->GetData( *data );
    }

    void DenOfIz_PipelineCache_Destroy( DenOfIz_PipelineCache cache )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( cache ) )
        {
            return;
        }
        delete PIPELINE_CACHE_IMPL( cache );
    }
}
