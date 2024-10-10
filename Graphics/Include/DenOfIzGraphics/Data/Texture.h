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

#include <DenOfIzGraphics/Utilities/Common.h>
#include <DenOfIzGraphics/Utilities/Utilities.h>
#include <DenOfIzGraphics/Backends/Interface/CommonData.h>
#include <functional>

#define DZ_USE_DDS
#define DZ_USE_STB_IMAGE

#ifdef DZ_USE_DDS
#include "dds.h"
#endif

namespace DenOfIz
{
    enum class TextureDimension
    {
        Undefined,
        Texture1D,
        Texture2D,
        Texture3D,
        TextureCube,
    };

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

    struct MipData
    {
        uint32_t Width;
        uint32_t Height;
        uint32_t MipIndex;
        uint32_t ArrayIndex;
        uint32_t RowPitch;
        uint32_t NumRows;
        uint32_t SlicePitch;
        uint32_t DataOffset;
    };

    typedef std::function<void( MipData mipData )> MipStreamCallback;
    class Texture
    {
    private:
        std::string          m_path;
        std::vector<MipData> m_mipData;
        dds::Header          m_ddsHeader{ };
        Byte                *m_contentData{ };

    public:
        uint32_t          Width{ };
        uint32_t          Height{ };
        uint32_t          Depth{ };
        uint32_t          MipLevels = 1;
        uint32_t          ArraySize = 1;
        uint32_t          BitsPerPixel{ };
        uint32_t          BlockSize{ };
        uint32_t          RowPitch{ };
        uint32_t          NumRows{ };
        uint32_t          SlicePitch{ };
        Format            Format    = Format::Undefined;
        TextureDimension  Dimension = TextureDimension::Undefined;
        TextureExtension  Extension = TextureExtension::DDS;
        std::vector<Byte> Data{ };
        explicit          Texture( const std::string &path );

        void StreamMipData( const MipStreamCallback &callback ) const;

    private:
        void LoadTextureSTB( );
        void LoadTextureDDS( );
        void StreamMipDataDDS( const MipStreamCallback &callback ) const;
        void StreamMipDataSTB( const MipStreamCallback &callback ) const;
#ifdef DZ_USE_DDS
        static enum Format GetFormatFromDDS( const dds::DXGI_FORMAT &format );
#endif
    };
} // namespace DenOfIz
