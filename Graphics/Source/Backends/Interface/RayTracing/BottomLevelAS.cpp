/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/Interface/RayTracing/IBottomLevelAS.h"

#define BLAS_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::IBottomLevelAS, handle )

extern "C"
{

    void DenOfIz_BottomLevelAS_BuildNumBytes( DenOfIz_BottomLevelAS bottomLevelAS, size_t *outNumBytes )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bottomLevelAS ) || outNumBytes == NULL )
        {
            return;
        }
        *outNumBytes = BLAS_IMPL( bottomLevelAS )->BuildNumBytes( );
    }

    void DenOfIz_BottomLevelAS_Destroy( DenOfIz_BottomLevelAS bottomLevelAS )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( bottomLevelAS ) )
        {
            return;
        }
        DenOfIz::DeferredDestroyQueue::Destroy( BLAS_IMPL( bottomLevelAS ) );
    }
}
