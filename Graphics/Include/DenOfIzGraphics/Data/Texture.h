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

#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAsset.h"
#include "DenOfIzGraphics/Backends/Interface/CommonData.h"
#include "DenOfIzGraphics/Utilities/Common.h"

namespace dds
{
    struct Header;
}

namespace DenOfIz
{
    struct DDSHeaderDeleter
    {
        void operator( )( const dds::Header *ptr ) const;
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

    class Texture
    {
        mutable DZArena                                m_arena{ 1024 };
        std::string                                    m_path;
        std::vector<TextureMip>                        m_mipData;
        std::unique_ptr<dds::Header, DDSHeaderDeleter> m_ddsHeader;
        Byte                                          *m_contentData{ };

        uint32_t          m_width{ };
        uint32_t          m_height{ };
        uint32_t          m_depth{ };
        uint32_t          m_mipLevels = 1;
        uint32_t          m_arraySize = 1;
        uint32_t          m_bitsPerPixel{ };
        uint32_t          m_blockSize{ };
        uint32_t          m_rowPitch{ };
        uint32_t          m_numRows{ };
        uint32_t          m_slicePitch{ };
        Format            m_format    = Format::Undefined;
        TextureDimension  m_dimension = TextureDimension::Undefined;
        TextureExtension  m_extension = TextureExtension::DDS;
        std::vector<Byte> m_data{ };

    public:
        DZ_API explicit Texture( const InteropString &path );
        DZ_API explicit Texture( const ByteArrayView &data, TextureExtension extension = TextureExtension::DDS );
        DZ_API static TextureExtension       IdentifyTextureFormat( const ByteArrayView &data );
        DZ_API [[nodiscard]] TextureMipArray ReadMipData( ) const;

        DZ_API [[nodiscard]] uint32_t         GetWidth( ) const;
        DZ_API [[nodiscard]] uint32_t         GetHeight( ) const;
        DZ_API [[nodiscard]] uint32_t         GetDepth( ) const;
        DZ_API [[nodiscard]] uint32_t         GetMipLevels( ) const;
        DZ_API [[nodiscard]] uint32_t         GetArraySize( ) const;
        DZ_API [[nodiscard]] uint32_t         GetBitsPerPixel( ) const;
        DZ_API [[nodiscard]] uint32_t         GetBlockSize( ) const;
        DZ_API [[nodiscard]] uint32_t         GetRowPitch( ) const;
        DZ_API [[nodiscard]] uint32_t         GetNumRows( ) const;
        DZ_API [[nodiscard]] uint32_t         GetSlicePitch( ) const;
        DZ_API [[nodiscard]] Format           GetFormat( ) const;
        DZ_API [[nodiscard]] TextureDimension GetDimension( ) const;
        DZ_API [[nodiscard]] TextureExtension GetExtension( ) const;
        DZ_API [[nodiscard]] ByteArrayView    GetData( ) const;

    private:
        void LoadTextureSTB( );
        void LoadTextureDDS( );

        void LoadTextureFromMemory( const Byte *data, size_t dataNumBytes );
        void LoadTextureDDSFromMemory( const Byte *data, size_t dataNumBytes );
        void LoadTextureSTBFromMemory( const Byte *data, size_t dataNumBytes );
    };
} // namespace DenOfIz
