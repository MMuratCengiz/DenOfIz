/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/Interface/ICommandListPool.h"

#define COMMAND_LIST_POOL_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ICommandListPool, handle )

extern "C"
{
    void DenOfIz_CommandListPool_GetCommandLists( DenOfIz_CommandListPool pool, DenOfIz_CommandListArray *outCommandLists )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( pool ) || outCommandLists == NULL )
        {
            return;
        }
        DenOfIz_CommandListArray cppArray = COMMAND_LIST_POOL_IMPL( pool )->GetCommandLists( );
        outCommandLists->NumElements      = cppArray.NumElements;
        outCommandLists->Elements         = (DenOfIz_CommandList *)malloc( sizeof( DenOfIz_CommandList ) * cppArray.NumElements );
        for ( size_t i = 0; i < cppArray.NumElements; ++i )
        {
            outCommandLists->Elements[ i ] = DENOFIZ_TO_HANDLE( cppArray.Elements[ i ] );
        }
    }

    void DenOfIz_CommandListPool_Destroy( DenOfIz_CommandListPool pool )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( pool ) )
        {
            return;
        }
        delete COMMAND_LIST_POOL_IMPL( pool );
    }
}
