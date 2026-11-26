/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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
