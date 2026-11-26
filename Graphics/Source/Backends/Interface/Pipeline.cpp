/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphicsInternal/Backends/Interface/IPipeline.h"

#define PIPELINE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::IPipeline, handle )

extern "C"
{

    void DenOfIz_Pipeline_Destroy( DenOfIz_Pipeline pipeline )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( pipeline ) )
        {
            return;
        }
        DenOfIz::DeferredDestroyQueue::Destroy( PIPELINE_IMPL( pipeline ) );
    }
}
