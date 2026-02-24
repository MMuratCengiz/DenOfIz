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
