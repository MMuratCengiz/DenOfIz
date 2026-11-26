/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#include "DenOfIzGraphics/Backends/Interface/Fence.h"
#include "DenOfIzGraphicsInternal/Backends/Interface/IFence.h"

#define FENCE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::IFence, handle )

extern "C"
{

    void DenOfIz_Fence_Wait( DenOfIz_Fence fence )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fence ) )
        {
            return;
        }
        FENCE_IMPL( fence )->Wait( );
    }

    void DenOfIz_Fence_Reset( DenOfIz_Fence fence )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fence ) )
        {
            return;
        }
        FENCE_IMPL( fence )->Reset( );
    }

    void DenOfIz_Fence_Destroy( DenOfIz_Fence fence )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fence ) )
        {
            return;
        }
        delete FENCE_IMPL( fence );
    }
}
