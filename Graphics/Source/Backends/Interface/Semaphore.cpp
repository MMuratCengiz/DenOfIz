/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/Interface/ISemaphore.h"

#define SEMAPHORE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ISemaphore, handle )

extern "C"
{

    void DenOfIz_Semaphore_Notify( DenOfIz_Semaphore semaphore )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( semaphore ) )
        {
            return;
        }
        SEMAPHORE_IMPL( semaphore )->Notify( );
    }

    void DenOfIz_Semaphore_IsCompleted( DenOfIz_Semaphore semaphore, bool *outCompleted )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( semaphore ) || outCompleted == NULL )
        {
            return;
        }
        *outCompleted = SEMAPHORE_IMPL( semaphore )->IsCompleted( );
    }

    void DenOfIz_Semaphore_Destroy( DenOfIz_Semaphore semaphore )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( semaphore ) )
        {
            return;
        }
        delete SEMAPHORE_IMPL( semaphore );
    }
}
