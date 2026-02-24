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
        delete ROOT_SIGNATURE_IMPL( rootSignature );
    }
}
