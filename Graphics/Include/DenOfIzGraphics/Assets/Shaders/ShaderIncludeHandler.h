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

#pragma once

#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_ShaderIncludeHandler )

    DZ_API DenOfIz_ShaderIncludeHandler DenOfIz_ShaderIncludeHandler_Create( );
    DZ_API void DenOfIz_ShaderIncludeHandler_AddFile( DenOfIz_ShaderIncludeHandler handler, const DenOfIz_StringView *path, const DenOfIz_ByteArrayView *data );
    DZ_API void DenOfIz_ShaderIncludeHandler_Destroy( DenOfIz_ShaderIncludeHandler handler );

#ifdef __cplusplus
}
#endif
