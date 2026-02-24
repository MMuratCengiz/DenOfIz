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
