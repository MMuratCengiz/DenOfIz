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

#include "DenOfIzGraphics/Assets/Serde/Font/FontAsset.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryWriter.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_FontAssetWriter )

    typedef struct DenOfIz_FontAssetWriterDesc
    {
        DenOfIz_BinaryWriter Writer;
    } DenOfIz_FontAssetWriterDesc;

    DZ_API DenOfIz_FontAssetWriter DenOfIz_FontAssetWriter_Create( const DenOfIz_FontAssetWriterDesc *desc );
    DZ_API void                    DenOfIz_FontAssetWriter_Destroy( DenOfIz_FontAssetWriter writer );
    DZ_API void                    DenOfIz_FontAssetWriter_Write( DenOfIz_FontAssetWriter writer, DenOfIz_FontAsset fontAsset );
    DZ_API void                    DenOfIz_FontAssetWriter_End( DenOfIz_FontAssetWriter writer );

#ifdef __cplusplus
}
#endif
