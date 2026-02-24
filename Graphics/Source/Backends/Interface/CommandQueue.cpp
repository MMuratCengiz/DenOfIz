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

#include "DenOfIzGraphicsInternal/Backends/Interface/ICommandQueue.h"

#define COMMAND_QUEUE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ICommandQueue, handle )

extern "C"
{

    void DenOfIz_CommandQueue_WaitIdle( DenOfIz_CommandQueue queue )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( queue ) )
        {
            return;
        }
        COMMAND_QUEUE_IMPL( queue )->WaitIdle( );
    }

    void DenOfIz_CommandQueue_ExecuteCommandLists( DenOfIz_CommandQueue queue, const DenOfIz_ExecuteCommandListsDesc *executeCommandListsDesc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( queue ) || executeCommandListsDesc == NULL )
        {
            return;
        }
        COMMAND_QUEUE_IMPL( queue )->ExecuteCommandLists( *executeCommandListsDesc );
    }

    void DenOfIz_CommandQueue_Destroy( DenOfIz_CommandQueue queue )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( queue ) )
        {
            return;
        }
        delete COMMAND_QUEUE_IMPL( queue );
    }
}
