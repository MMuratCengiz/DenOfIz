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

#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"
#include "DenOfIzGraphicsInternal/Assets/Stream/BinaryContainerImpl.h" // cpp version

#define BINARY_CONTAINER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::BinaryContainerImpl, handle )

extern "C"
{

    DenOfIz_BinaryContainer DenOfIz_BinaryContainer_Create( )
    {
        auto *container = new DenOfIz::BinaryContainerImpl( );
        return DENOFIZ_TO_HANDLE( container );
    }

    void DenOfIz_BinaryContainer_Destroy( DenOfIz_BinaryContainer container )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( container ) )
        {
            return;
        }
        delete BINARY_CONTAINER_IMPL( container );
    }

    DenOfIz_ByteArrayView DenOfIz_BinaryContainer_GetData( DenOfIz_BinaryContainer container )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( container ) )
        {
            return DenOfIz_ByteArrayView{ nullptr, 0 };
        }
        return BINARY_CONTAINER_IMPL( container )->GetData( );
    }
}
