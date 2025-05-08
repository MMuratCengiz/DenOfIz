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

#include <DenOfIzGraphics/Assets/Serde/Texture/TextureAsset.h>
#include <DenOfIzGraphics/Backends/Interface/CommonData.h>
#include <DenOfIzGraphics/Utilities/Common.h>
#include <functional>

#define DZ_USE_DDS
#define DZ_USE_STB_IMAGE

#ifdef DZ_USE_DDS
#include "dds.h"
#endif

namespace DenOfIz
{

    enum class TextureExtension
    {
        DDS,
        PNG,
        JPG,
        BMP,
        TGA,
        HDR,
        GIF,
        PIC,
    };

    typedef std::function<void( TextureMip mipData )> MipStreamCallback;
    class Texture
    {
        std::string             m_path;
        std::vector<TextureMip> m_mipData;
        dds::Header             m_ddsHeader{ };
        Byte                   *m_contentData{ };

    public:
        DZ_API uint32_t          Width{ };
        DZ_API uint32_t          Height{ };
        DZ_API uint32_t          Depth{ };
        DZ_API uint32_t          MipLevels = 1;
        DZ_API uint32_t          ArraySize = 1;
        DZ_API uint32_t          BitsPerPixel{ };
        DZ_API uint32_t          BlockSize{ };
        DZ_API uint32_t          RowPitch{ };
        DZ_API uint32_t          NumRows{ };
        DZ_API uint32_t          SlicePitch{ };
        DZ_API Format            Format    = Format::Undefined;
        DZ_API TextureDimension  Dimension = TextureDimension::Undefined;
        DZ_API TextureExtension  Extension = TextureExtension::DDS;
        DZ_API std::vector<Byte> Data{ };
        DZ_API explicit Texture( const std::string &path );
        DZ_API explicit Texture( const InteropArray<Byte> &data, TextureExtension extension = TextureExtension::DDS );
        DZ_API static TextureExtension IdentifyTextureFormat( const InteropArray<Byte> &data );
        DZ_API void StreamMipData( const MipStreamCallback &callback ) const;

    private:
        void LoadTextureSTB( );
        void LoadTextureDDS( );
        void StreamMipDataDDS( const MipStreamCallback &callback ) const;
        void StreamMipDataSTB( const MipStreamCallback &callback ) const;

        void LoadTextureFromMemory( const Byte *data, size_t dataNumBytes );
        void LoadTextureDDSFromMemory( const Byte *data, size_t dataNumBytes );
        void LoadTextureSTBFromMemory( const Byte *data, size_t dataNumBytes );
#ifdef DZ_USE_DDS
        static enum Format GetFormatFromDDS( const dds::DXGI_FORMAT &format );
#endif
    };
} // namespace DenOfIz
