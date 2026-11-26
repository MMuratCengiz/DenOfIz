/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphics/Assets/Stream/BinaryContainer.h"
#include "DenOfIzGraphicsInternal/Assets/Stream/BinaryContainerImpl.h"

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
