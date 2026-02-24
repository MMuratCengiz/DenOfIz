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
