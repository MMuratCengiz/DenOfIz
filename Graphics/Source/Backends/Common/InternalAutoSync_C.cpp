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

#include "DenOfIzGraphicsInternal/Backends/Common/InternalAutoSync_C.h"
#include <DenOfIzGraphics/Handle.h>
#include <DenOfIzGraphics/Utilities/Array.h>
#include <DenOfIzGraphics/Utilities/HashMap.h>
#include <stb_ds.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#elif __APPLE__ || __linux__
#include <pthread.h>
#endif

typedef struct DenOfIz_SwapChainTextureMapEntry
{
    DenOfIz_SwapChain key;
    DenOfIz_Texture   value;
} DenOfIz_SwapChainTextureMapEntry;

struct DenOfIz_InternalAutoSync_T
{
    bool                              autoSync;
    DenOfIz_Texture                  *swapChainTextures;
    DenOfIz_SwapChainTextureMapEntry *swapChainCurrentTextures;
    DenOfIz_SwapChain                *activeSwapChains;
#ifdef _WIN32
    CRITICAL_SECTION mutex;
#else
    pthread_mutex_t mutex;
#endif
};

DenOfIz_InternalAutoSync DenOfIz_InternalAutoSync_Create( bool autoSync )
{
    DenOfIz_InternalAutoSync sync = (DenOfIz_InternalAutoSync)malloc( sizeof( struct DenOfIz_InternalAutoSync_T ) );
    if ( !sync )
    {
        return NULL;
    }

    sync->autoSync                 = autoSync;
    sync->swapChainTextures        = NULL;
    sync->swapChainCurrentTextures = NULL;
    sync->activeSwapChains         = NULL;

#ifdef _WIN32
    InitializeCriticalSection( &sync->mutex );
#else
    pthread_mutex_init( &sync->mutex, NULL );
#endif

    return sync;
}

void DenOfIz_InternalAutoSync_Destroy( DenOfIz_InternalAutoSync autoSync )
{
    if ( !autoSync )
    {
        return;
    }

    if ( autoSync->swapChainTextures )
    {
        DenOfIz_Array_Free( autoSync->swapChainTextures );
        autoSync->swapChainTextures = NULL;
    }

    if ( autoSync->swapChainCurrentTextures )
    {
        DenOfIz_HashMap_Free( autoSync->swapChainCurrentTextures );
        autoSync->swapChainCurrentTextures = NULL;
    }

    if ( autoSync->activeSwapChains )
    {
        DenOfIz_Array_Free( autoSync->activeSwapChains );
        autoSync->activeSwapChains = NULL;
    }

#ifdef _WIN32
    DeleteCriticalSection( &autoSync->mutex );
#else
    pthread_mutex_destroy( &autoSync->mutex );
#endif

    free( autoSync );
}

void DenOfIz_InternalAutoSync_NewTextureResource( DenOfIz_InternalAutoSync autoSync, DenOfIz_Texture texture, bool isSwapChainTexture )
{
    if ( !autoSync || !DENOFIZ_HANDLE_IS_VALID( texture ) )
    {
        return;
    }

#ifdef _WIN32
    EnterCriticalSection( &autoSync->mutex );
#else
    pthread_mutex_lock( &autoSync->mutex );
#endif

    if ( isSwapChainTexture )
    {
        bool found = false;
        for ( int i = 0; i < DenOfIz_Array_Length( autoSync->swapChainTextures ); i++ )
        {
            if ( autoSync->swapChainTextures[ i ] == texture )
            {
                found = true;
                break;
            }
        }
        if ( !found )
        {
            DenOfIz_Array_Push( autoSync->swapChainTextures, texture );
        }
    }

#ifdef _WIN32
    LeaveCriticalSection( &autoSync->mutex );
#else
    pthread_mutex_unlock( &autoSync->mutex );
#endif
}

void DenOfIz_InternalAutoSync_RemoveTextureResource( DenOfIz_InternalAutoSync autoSync, DenOfIz_Texture texture )
{
    if ( !autoSync || !DENOFIZ_HANDLE_IS_VALID( texture ) )
    {
        return;
    }

#ifdef _WIN32
    EnterCriticalSection( &autoSync->mutex );
#else
    pthread_mutex_lock( &autoSync->mutex );
#endif

    for ( int i = 0; i < DenOfIz_Array_Length( autoSync->swapChainTextures ); i++ )
    {
        if ( autoSync->swapChainTextures[ i ] == texture )
        {
            DenOfIz_Array_Delete( autoSync->swapChainTextures, i );
            break;
        }
    }

    for ( int i = 0; i < DenOfIz_HashMap_Length( autoSync->swapChainCurrentTextures ); i++ )
    {
        if ( autoSync->swapChainCurrentTextures[ i ].value == texture )
        {
            DenOfIz_HashMap_Delete( autoSync->swapChainCurrentTextures, autoSync->swapChainCurrentTextures[ i ].key );
            break;
        }
    }

#ifdef _WIN32
    LeaveCriticalSection( &autoSync->mutex );
#else
    pthread_mutex_unlock( &autoSync->mutex );
#endif
}

void DenOfIz_InternalAutoSync_AcquireImage( DenOfIz_InternalAutoSync autoSync, DenOfIz_SwapChain swapChain, DenOfIz_Texture texture )
{
    if ( !autoSync || !DENOFIZ_HANDLE_IS_VALID( swapChain ) || !DENOFIZ_HANDLE_IS_VALID( texture ) )
    {
        return;
    }

#ifdef _WIN32
    EnterCriticalSection( &autoSync->mutex );
#else
    pthread_mutex_lock( &autoSync->mutex );
#endif

    DenOfIz_HashMap_Put( autoSync->swapChainCurrentTextures, swapChain, texture );

    bool found = false;
    for ( int i = 0; i < DenOfIz_Array_Length( autoSync->swapChainTextures ); i++ )
    {
        if ( autoSync->swapChainTextures[ i ] == texture )
        {
            found = true;
            break;
        }
    }
    if ( !found )
    {
        DenOfIz_Array_Push( autoSync->swapChainTextures, texture );
    }

    found = false;
    for ( int i = 0; i < DenOfIz_Array_Length( autoSync->activeSwapChains ); i++ )
    {
        if ( autoSync->activeSwapChains[ i ] == swapChain )
        {
            found = true;
            break;
        }
    }
    if ( !found )
    {
        DenOfIz_Array_Push( autoSync->activeSwapChains, swapChain );
    }

#ifdef _WIN32
    LeaveCriticalSection( &autoSync->mutex );
#else
    pthread_mutex_unlock( &autoSync->mutex );
#endif
}

void DenOfIz_InternalAutoSync_BeginRendering( DenOfIz_InternalAutoSync autoSync, DenOfIz_Texture renderTarget )
{
    if ( !autoSync || !DENOFIZ_HANDLE_IS_VALID( renderTarget ) )
    {
        return;
    }

#ifdef _WIN32
    EnterCriticalSection( &autoSync->mutex );
#else
    pthread_mutex_lock( &autoSync->mutex );
#endif

    bool isSwapChainTexture = false;
    for ( int i = 0; i < DenOfIz_Array_Length( autoSync->swapChainTextures ); i++ )
    {
        if ( autoSync->swapChainTextures[ i ] == renderTarget )
        {
            isSwapChainTexture = true;
            break;
        }
    }

    if ( isSwapChainTexture )
    {
        for ( int i = 0; i < DenOfIz_HashMap_Length( autoSync->swapChainCurrentTextures ); i++ )
        {
            if ( autoSync->swapChainCurrentTextures[ i ].value == renderTarget )
            {
                DenOfIz_SwapChain swapChain = autoSync->swapChainCurrentTextures[ i ].key;

                bool found = false;
                for ( int j = 0; j < DenOfIz_Array_Length( autoSync->activeSwapChains ); j++ )
                {
                    if ( autoSync->activeSwapChains[ j ] == swapChain )
                    {
                        found = true;
                        break;
                    }
                }
                if ( !found )
                {
                    DenOfIz_Array_Push( autoSync->activeSwapChains, swapChain );
                }

#ifdef _WIN32
                LeaveCriticalSection( &autoSync->mutex );
#else
                pthread_mutex_unlock( &autoSync->mutex );
#endif
                return;
            }
        }
    }

#ifdef _WIN32
    LeaveCriticalSection( &autoSync->mutex );
#else
    pthread_mutex_unlock( &autoSync->mutex );
#endif
}

bool DenOfIz_InternalAutoSync_NeedsSwapChainSync( DenOfIz_InternalAutoSync autoSync, DenOfIz_SwapChain *outSwapChain )
{
    if ( !autoSync || !outSwapChain )
    {
        return false;
    }

#ifdef _WIN32
    EnterCriticalSection( &autoSync->mutex );
#else
    pthread_mutex_lock( &autoSync->mutex );
#endif

    if ( DenOfIz_Array_Length( autoSync->activeSwapChains ) > 0 )
    {
        *outSwapChain = autoSync->activeSwapChains[ 0 ];

#ifdef _WIN32
        LeaveCriticalSection( &autoSync->mutex );
#else
        pthread_mutex_unlock( &autoSync->mutex );
#endif
        return true;
    }

    *outSwapChain = DENOFIZ_NULL_HANDLE;

#ifdef _WIN32
    LeaveCriticalSection( &autoSync->mutex );
#else
    pthread_mutex_unlock( &autoSync->mutex );
#endif

    return false;
}

void DenOfIz_InternalAutoSync_ClearCommandListSync( DenOfIz_InternalAutoSync autoSync )
{
    if ( !autoSync )
    {
        return;
    }

#ifdef _WIN32
    EnterCriticalSection( &autoSync->mutex );
#else
    pthread_mutex_lock( &autoSync->mutex );
#endif

    if ( autoSync->activeSwapChains )
    {
        DenOfIz_Array_SetLength( autoSync->activeSwapChains, 0 );
    }

#ifdef _WIN32
    LeaveCriticalSection( &autoSync->mutex );
#else
    pthread_mutex_unlock( &autoSync->mutex );
#endif
}
