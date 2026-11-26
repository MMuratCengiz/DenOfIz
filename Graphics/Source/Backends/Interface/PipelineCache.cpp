/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
