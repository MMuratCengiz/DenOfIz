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
        delete TLAS_IMPL( topLevelAS );
    }
}
