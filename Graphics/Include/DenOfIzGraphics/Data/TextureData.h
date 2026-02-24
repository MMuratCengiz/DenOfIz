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

#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum DenOfIz_TextureExtension
    {
        DENOFIZ_TEXTURE_EXTENSION_DDS,
        DENOFIZ_TEXTURE_EXTENSION_PNG,
        DENOFIZ_TEXTURE_EXTENSION_JPG,
        DENOFIZ_TEXTURE_EXTENSION_BMP,
        DENOFIZ_TEXTURE_EXTENSION_TGA,
        DENOFIZ_TEXTURE_EXTENSION_HDR,
        DENOFIZ_TEXTURE_EXTENSION_GIF,
        DENOFIZ_TEXTURE_EXTENSION_PIC,
    } DenOfIz_TextureExtension;

    typedef enum DenOfIz_TextureDimension
    {
        DENOFIZ_TEXTURE_DIMENSION_UNDEFINED,
        DENOFIZ_TEXTURE_DIMENSION_TEXTURE1D,
        DENOFIZ_TEXTURE_DIMENSION_TEXTURE2D,
        DENOFIZ_TEXTURE_DIMENSION_TEXTURE3D,
        DENOFIZ_TEXTURE_DIMENSION_TEXTURE_CUBE,
    } DenOfIz_TextureDimension;

    typedef struct DenOfIz_TextureMip
    {
        uint32_t Width;
        uint32_t Height;
        uint32_t MipIndex;
        uint32_t ArrayIndex;
        uint32_t RowPitch;
        uint32_t NumRows;
        uint32_t SlicePitch;
        uint32_t DataOffset;
    } DenOfIz_TextureMip;

    typedef struct DenOfIz_TextureMipArray
    {
        DenOfIz_TextureMip *Elements;
        size_t              NumElements;
    } DenOfIz_TextureMipArray;

    typedef struct DenOfIz_TextureCreateFromPathDesc
    {
        DenOfIz_StringView Path;
    } DenOfIz_TextureCreateFromPathDesc;

    typedef struct DenOfIz_TextureCreateFromDataDesc
    {
        DenOfIz_ByteArrayView    Data;
        DenOfIz_TextureExtension Extension;
    } DenOfIz_TextureCreateFromDataDesc;

    DENOFIZ_DEFINE_HANDLE( DenOfIz_TextureData )

    DZ_API DenOfIz_TextureData      DenOfIz_TextureData_CreateFromPath( const DenOfIz_TextureCreateFromPathDesc *desc );
    DZ_API DenOfIz_TextureData      DenOfIz_TextureData_CreateFromData( const DenOfIz_TextureCreateFromDataDesc *desc );
    DZ_API DenOfIz_TextureExtension DenOfIz_TextureData_IdentifyTextureFormat( const DenOfIz_ByteArrayView *data );
    DZ_API DenOfIz_TextureMipArray  DenOfIz_TextureData_ReadMipData( DenOfIz_TextureData texture );
    DZ_API uint32_t                 DenOfIz_TextureData_GetWidth( DenOfIz_TextureData texture );
    DZ_API uint32_t                 DenOfIz_TextureData_GetHeight( DenOfIz_TextureData texture );
    DZ_API uint32_t                 DenOfIz_TextureData_GetDepth( DenOfIz_TextureData texture );
    DZ_API uint32_t                 DenOfIz_TextureData_GetMipLevels( DenOfIz_TextureData texture );
    DZ_API uint32_t                 DenOfIz_TextureData_GetArraySize( DenOfIz_TextureData texture );
    DZ_API uint32_t                 DenOfIz_TextureData_GetBitsPerPixel( DenOfIz_TextureData texture );
    DZ_API uint32_t                 DenOfIz_TextureData_GetBlockSize( DenOfIz_TextureData texture );
    DZ_API uint32_t                 DenOfIz_TextureData_GetRowPitch( DenOfIz_TextureData texture );
    DZ_API uint32_t                 DenOfIz_TextureData_GetNumRows( DenOfIz_TextureData texture );
    DZ_API uint32_t                 DenOfIz_TextureData_GetSlicePitch( DenOfIz_TextureData texture );
    DZ_API DenOfIz_Format           DenOfIz_TextureData_GetFormat( DenOfIz_TextureData texture );
    DZ_API DenOfIz_TextureDimension DenOfIz_TextureData_GetDimension( DenOfIz_TextureData texture );
    DZ_API DenOfIz_TextureExtension DenOfIz_TextureData_GetExtension( DenOfIz_TextureData texture );
    DZ_API DenOfIz_ByteArrayView    DenOfIz_TextureData_GetData( DenOfIz_TextureData texture );
    DZ_API void                     DenOfIz_TextureData_Destroy( DenOfIz_TextureData texture );

#ifdef __cplusplus
}
#endif
