/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/Interface/IInputLayout.h"

#define INPUT_LAYOUT_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::IInputLayout, handle )

extern "C"
{

    void DenOfIz_InputLayout_Destroy( DenOfIz_InputLayout inputLayout )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( inputLayout ) )
        {
            return;
        }
        DenOfIz::DeferredDestroyQueue::Destroy( INPUT_LAYOUT_IMPL( inputLayout ) );
    }
}
