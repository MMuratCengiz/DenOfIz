/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/Interface/RayTracing/ILocalRootSignature.h"

#define LOCAL_ROOT_SIG_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ILocalRootSignature, handle )

extern "C"
{

    void DenOfIz_LocalRootSignature_Destroy( DenOfIz_LocalRootSignature localRootSignature )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( localRootSignature ) )
        {
            return;
        }
        DenOfIz::DeferredDestroyQueue::Destroy( LOCAL_ROOT_SIG_IMPL( localRootSignature ) );
    }
}
