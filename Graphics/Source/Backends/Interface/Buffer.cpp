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

#include "DenOfIzGraphicsInternal/Backends/Interface/IBuffer.h"

#define BUFFER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::IBuffer, handle )

extern "C"
{

    void DenOfIz_Buffer_MapMemory( DenOfIz_Buffer buffer, void **outMappedMemory )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( buffer ) || outMappedMemory == NULL )
        {
            return;
        }
        *outMappedMemory = BUFFER_IMPL( buffer )->MapMemory( );
    }

    void DenOfIz_Buffer_UnmapMemory( DenOfIz_Buffer buffer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( buffer ) )
        {
            return;
        }
        BUFFER_IMPL( buffer )->UnmapMemory( );
    }

    void DenOfIz_Buffer_NumBytes( DenOfIz_Buffer buffer, size_t *outNumBytes )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( buffer ) || outNumBytes == NULL )
        {
            return;
        }
        *outNumBytes = BUFFER_IMPL( buffer )->NumBytes( );
    }

    void DenOfIz_Buffer_Data( DenOfIz_Buffer buffer, const void **outData )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( buffer ) || outData == NULL )
        {
            return;
        }
        *outData = BUFFER_IMPL( buffer )->Data( );
    }

    void DenOfIz_Buffer_GetData( DenOfIz_Buffer buffer, DenOfIz_ByteArray *outData )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( buffer ) || outData == NULL )
        {
            return;
        }
        *outData = BUFFER_IMPL( buffer )->GetData( );
    }

    void DenOfIz_Buffer_SetData( DenOfIz_Buffer buffer, const DenOfIz_ByteArrayView *data, bool keepMapped )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( buffer ) || data == NULL )
        {
            return;
        }
        BUFFER_IMPL( buffer )->SetData( *data, keepMapped );
    }

    void DenOfIz_Buffer_WriteData( DenOfIz_Buffer buffer, const DenOfIz_ByteArrayView *data, uint32_t bufferOffset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( buffer ) || data == NULL )
        {
            return;
        }
        BUFFER_IMPL( buffer )->WriteData( *data, bufferOffset );
    }

    void DenOfIz_Buffer_Destroy( DenOfIz_Buffer buffer )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( buffer ) )
        {
            return;
        }
        delete BUFFER_IMPL( buffer );
    }
}
