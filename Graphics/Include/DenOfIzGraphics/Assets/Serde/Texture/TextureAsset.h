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

#include "DenOfIzGraphics/Assets/Serde/Asset.h"
#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphics/Data/TextureData.h"
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"

#ifdef __cplusplus
extern "C"
{
#endif

    DENOFIZ_DEFINE_HANDLE( DenOfIz_TextureAsset )

    DZ_API DenOfIz_TextureAsset DenOfIz_TextureAsset_Create( );
    DZ_API void                 DenOfIz_TextureAsset_Destroy( DenOfIz_TextureAsset textureAsset );

    DZ_API uint64_t                 DenOfIz_TextureAsset_Magic( DenOfIz_TextureAsset textureAsset );
    DZ_API uint32_t                 DenOfIz_TextureAsset_Version( DenOfIz_TextureAsset textureAsset );
    DZ_API uint64_t                 DenOfIz_TextureAsset_NumBytes( DenOfIz_TextureAsset textureAsset );
    DZ_API DenOfIz_StringView       DenOfIz_TextureAsset_Path( DenOfIz_TextureAsset textureAsset );
    DZ_API DenOfIz_StringView       DenOfIz_TextureAsset_Name( DenOfIz_TextureAsset textureAsset );
    DZ_API DenOfIz_StringView       DenOfIz_TextureAsset_SourcePath( DenOfIz_TextureAsset textureAsset );
    DZ_API uint32_t                 DenOfIz_TextureAsset_Width( DenOfIz_TextureAsset textureAsset );
    DZ_API uint32_t                 DenOfIz_TextureAsset_Height( DenOfIz_TextureAsset textureAsset );
    DZ_API uint32_t                 DenOfIz_TextureAsset_Depth( DenOfIz_TextureAsset textureAsset );
    DZ_API DenOfIz_Format           DenOfIz_TextureAsset_GetFormat( DenOfIz_TextureAsset textureAsset );
    DZ_API DenOfIz_TextureDimension DenOfIz_TextureAsset_GetDimension( DenOfIz_TextureAsset textureAsset );
    DZ_API uint32_t                 DenOfIz_TextureAsset_MipLevels( DenOfIz_TextureAsset textureAsset );
    DZ_API uint32_t                 DenOfIz_TextureAsset_ArraySize( DenOfIz_TextureAsset textureAsset );
    DZ_API uint32_t                 DenOfIz_TextureAsset_BitsPerPixel( DenOfIz_TextureAsset textureAsset );
    DZ_API uint32_t                 DenOfIz_TextureAsset_BlockSize( DenOfIz_TextureAsset textureAsset );
    DZ_API uint32_t                 DenOfIz_TextureAsset_RowPitch( DenOfIz_TextureAsset textureAsset );
    DZ_API uint32_t                 DenOfIz_TextureAsset_NumRows( DenOfIz_TextureAsset textureAsset );
    DZ_API uint32_t                 DenOfIz_TextureAsset_SlicePitch( DenOfIz_TextureAsset textureAsset );
    DZ_API DenOfIz_TextureMipArray  DenOfIz_TextureAsset_Mips( DenOfIz_TextureAsset textureAsset );
    DZ_API size_t                   DenOfIz_TextureAsset_NumMips( DenOfIz_TextureAsset textureAsset );
    DZ_API DenOfIz_AssetDataStream  DenOfIz_TextureAsset_Data( DenOfIz_TextureAsset textureAsset );

    DZ_API void DenOfIz_TextureAsset_SetVersion( DenOfIz_TextureAsset textureAsset, uint32_t version );
    DZ_API void DenOfIz_TextureAsset_SetNumBytes( DenOfIz_TextureAsset textureAsset, uint64_t numBytes );
    DZ_API void DenOfIz_TextureAsset_SetPath( DenOfIz_TextureAsset textureAsset, DenOfIz_StringView path );
    DZ_API void DenOfIz_TextureAsset_SetName( DenOfIz_TextureAsset textureAsset, DenOfIz_StringView name );
    DZ_API void DenOfIz_TextureAsset_SetSourcePath( DenOfIz_TextureAsset textureAsset, DenOfIz_StringView sourcePath );
    DZ_API void DenOfIz_TextureAsset_SetWidth( DenOfIz_TextureAsset textureAsset, uint32_t width );
    DZ_API void DenOfIz_TextureAsset_SetHeight( DenOfIz_TextureAsset textureAsset, uint32_t height );
    DZ_API void DenOfIz_TextureAsset_SetDepth( DenOfIz_TextureAsset textureAsset, uint32_t depth );
    DZ_API void DenOfIz_TextureAsset_SetFormat( DenOfIz_TextureAsset textureAsset, DenOfIz_Format format );
    DZ_API void DenOfIz_TextureAsset_SetDimension( DenOfIz_TextureAsset textureAsset, DenOfIz_TextureDimension dimension );
    DZ_API void DenOfIz_TextureAsset_SetMipLevels( DenOfIz_TextureAsset textureAsset, uint32_t mipLevels );
    DZ_API void DenOfIz_TextureAsset_SetArraySize( DenOfIz_TextureAsset textureAsset, uint32_t arraySize );
    DZ_API void DenOfIz_TextureAsset_SetBitsPerPixel( DenOfIz_TextureAsset textureAsset, uint32_t bitsPerPixel );
    DZ_API void DenOfIz_TextureAsset_SetBlockSize( DenOfIz_TextureAsset textureAsset, uint32_t blockSize );
    DZ_API void DenOfIz_TextureAsset_SetRowPitch( DenOfIz_TextureAsset textureAsset, uint32_t rowPitch );
    DZ_API void DenOfIz_TextureAsset_SetNumRows( DenOfIz_TextureAsset textureAsset, uint32_t numRows );
    DZ_API void DenOfIz_TextureAsset_SetSlicePitch( DenOfIz_TextureAsset textureAsset, uint32_t slicePitch );
    DZ_API void DenOfIz_TextureAsset_SetData( DenOfIz_TextureAsset textureAsset, DenOfIz_AssetDataStream data );

    DZ_API void DenOfIz_TextureAsset_AddMip( DenOfIz_TextureAsset textureAsset, const DenOfIz_TextureMip *mip );
    DZ_API void DenOfIz_TextureAsset_SetMips( DenOfIz_TextureAsset textureAsset, const DenOfIz_TextureMip *mips, size_t count );
    DZ_API void DenOfIz_TextureAsset_ReserveMips( DenOfIz_TextureAsset textureAsset, size_t capacity );
    DZ_API void DenOfIz_TextureAsset_ClearMips( DenOfIz_TextureAsset textureAsset );

    DZ_API DenOfIz_StringView DenOfIz_TextureAsset_Extension( );

#ifdef __cplusplus
}
#endif
