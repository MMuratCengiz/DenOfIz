/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/Interface/IRootSignature.h"

#define ROOT_SIGNATURE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::IRootSignature, handle )

extern "C"
{

    void DenOfIz_RootSignature_Destroy( DenOfIz_RootSignature rootSignature )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( rootSignature ) )
        {
            return;
        }
        DenOfIz::DeferredDestroyQueue::Destroy( ROOT_SIGNATURE_IMPL( rootSignature ) );
    }
}
