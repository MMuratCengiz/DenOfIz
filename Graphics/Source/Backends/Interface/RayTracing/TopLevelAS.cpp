/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/Interface/RayTracing/ITopLevelAS.h"

#define TLAS_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::ITopLevelAS, handle )

extern "C"
{

    void DenOfIz_TopLevelAS_UpdateInstanceTransforms( DenOfIz_TopLevelAS topLevelAS, const DenOfIz_UpdateTransformsDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( topLevelAS ) || desc == NULL )
        {
            return;
        }
        TLAS_IMPL( topLevelAS )->UpdateInstanceTransforms( *desc );
    }

    void DenOfIz_TopLevelAS_BuildNumBytes( DenOfIz_TopLevelAS topLevelAS, size_t *outNumBytes )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( topLevelAS ) || outNumBytes == NULL )
        {
            return;
        }
        *outNumBytes = TLAS_IMPL( topLevelAS )->BuildNumBytes( );
    }

    void DenOfIz_TopLevelAS_Destroy( DenOfIz_TopLevelAS topLevelAS )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( topLevelAS ) )
        {
            return;
        }
        DenOfIz::DeferredDestroyQueue::Destroy( TLAS_IMPL( topLevelAS ) );
    }
}
