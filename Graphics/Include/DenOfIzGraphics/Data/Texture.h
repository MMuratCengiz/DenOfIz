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

        uint32_t                m_width{ };
        uint32_t                m_height{ };
        uint32_t                m_depth{ };
        uint32_t                m_mipLevels = 1;
        uint32_t                m_arraySize = 1;
        uint32_t                m_bitsPerPixel{ };
        uint32_t                m_blockSize{ };
        uint32_t                m_rowPitch{ };
        uint32_t                m_numRows{ };
        uint32_t                m_slicePitch{ };
        Format                  m_format = Format::Undefined;
        TextureDimension        m_dimension = TextureDimension::Undefined;
        TextureExtension        m_extension = TextureExtension::DDS;
        InteropArray<Byte>      m_data{ };

    public:
        DZ_API uint32_t         GetWidth() const { return m_width; }
        DZ_API uint32_t         GetHeight() const { return m_height; }
        DZ_API uint32_t         GetDepth() const { return m_depth; }
        DZ_API uint32_t         GetMipLevels() const { return m_mipLevels; }
        DZ_API uint32_t         GetArraySize() const { return m_arraySize; }
        DZ_API uint32_t         GetBitsPerPixel() const { return m_bitsPerPixel; }
        DZ_API uint32_t         GetBlockSize() const { return m_blockSize; }
        DZ_API uint32_t         GetRowPitch() const { return m_rowPitch; }
        DZ_API uint32_t         GetNumRows() const { return m_numRows; }
        DZ_API uint32_t         GetSlicePitch() const { return m_slicePitch; }
        DZ_API Format           GetFormat() const { return m_format; }
        DZ_API TextureDimension GetDimension() const { return m_dimension; }
        DZ_API TextureExtension GetExtension() const { return m_extension; }
        DZ_API const InteropArray<Byte>& GetData() const { return m_data; }
        DZ_API explicit Texture( const InteropString &path );
        DZ_API explicit Texture( const InteropArray<Byte> &data, TextureExtension extension = TextureExtension::DDS );
        DZ_API static TextureExtension                IdentifyTextureFormat( const InteropArray<Byte> &data );
        DZ_API [[nodiscard]] InteropArray<TextureMip> ReadMipData( ) const;
        void                                          StreamMipData( const MipStreamCallback &callback ) const;

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
