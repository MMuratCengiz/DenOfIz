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

#include "DenOfIzGraphics/Data/TextureData.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <cstring>
#include <filesystem>
#include <fstream>
#include <memory>
#include <vector>
#include "dds.h"

#define TEXTURE_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::TextureDataImpl, handle )

namespace DenOfIz
{
    struct DDSHeaderDeleter
    {
        void operator( )( const dds::Header *ptr ) const
        {
            delete ptr;
        }
    };

    class TextureDataImpl
    {
    public:
        std::string                                    m_path;
        mutable std::vector<DenOfIz_TextureMip>        m_mipData;
        std::unique_ptr<dds::Header, DDSHeaderDeleter> m_ddsHeader;
        Byte                                          *m_contentData{ };

        uint32_t                 m_width{ };
        uint32_t                 m_height{ };
        uint32_t                 m_depth{ };
        uint32_t                 m_mipLevels = 1;
        uint32_t                 m_arraySize = 1;
        uint32_t                 m_bitsPerPixel{ };
        uint32_t                 m_blockSize{ };
        uint32_t                 m_rowPitch{ };
        uint32_t                 m_numRows{ };
        uint32_t                 m_slicePitch{ };
        DenOfIz_Format           m_format    = DENOFIZ_FORMAT_UNDEFINED;
        DenOfIz_TextureDimension m_dimension = DENOFIZ_TEXTURE_DIMENSION_UNDEFINED;
        DenOfIz_TextureExtension m_extension = DENOFIZ_TEXTURE_EXTENSION_DDS;
        std::vector<Byte>        m_data{ };

        explicit TextureDataImpl( const char *path );
        explicit TextureDataImpl( const DenOfIz_StringView &path );
        explicit TextureDataImpl( const DenOfIz_ByteArrayView &data, DenOfIz_TextureExtension extension = DENOFIZ_TEXTURE_EXTENSION_DDS );
        ~TextureDataImpl( ) = default;

        static DenOfIz_TextureExtension IdentifyTextureFormat( const DenOfIz_ByteArrayView &data );
        DenOfIz_TextureMipArray         ReadMipData( ) const;

        uint32_t                 GetWidth( ) const;
        uint32_t                 GetHeight( ) const;
        uint32_t                 GetDepth( ) const;
        uint32_t                 GetMipLevels( ) const;
        uint32_t                 GetArraySize( ) const;
        uint32_t                 GetBitsPerPixel( ) const;
        uint32_t                 GetBlockSize( ) const;
        uint32_t                 GetRowPitch( ) const;
        uint32_t                 GetNumRows( ) const;
        uint32_t                 GetSlicePitch( ) const;
        DenOfIz_Format           GetFormat( ) const;
        DenOfIz_TextureDimension GetDimension( ) const;
        DenOfIz_TextureExtension GetExtension( ) const;
        DenOfIz_ByteArrayView    GetData( ) const;

    private:
        void LoadTextureSTB( );
        void LoadTextureDDS( );

        void LoadTextureFromMemory( const Byte *data, size_t dataNumBytes );
        void LoadTextureDDSFromMemory( const Byte *data, size_t dataNumBytes );
        void LoadTextureSTBFromMemory( const Byte *data, size_t dataNumBytes );
    };
} // namespace DenOfIz

using namespace DenOfIz;

TextureDataImpl::TextureDataImpl( const char *path ) : TextureDataImpl( DENOFIZ_STRING( path ) )
{
}

TextureDataImpl::TextureDataImpl( const DenOfIz_StringView &path ) : m_path( Utilities::AppPath( std::string( path.Chars, path.NumChars ) ) )
{
    if ( !std::filesystem::exists( m_path ) )
    {
        spdlog::error( "Texture file does not exist: {}", m_path );
        return;
    }
    if ( const std::filesystem::path &extension = std::filesystem::path( m_path ).extension( ); extension == ".dds" )
    {
        m_extension = DENOFIZ_TEXTURE_EXTENSION_DDS;
    }
    else if ( extension == ".png" )
    {
        m_extension = DENOFIZ_TEXTURE_EXTENSION_PNG;
    }
    else if ( extension == ".jpg" )
    {
        m_extension = DENOFIZ_TEXTURE_EXTENSION_JPG;
    }
    else if ( extension == ".bmp" )
    {
        m_extension = DENOFIZ_TEXTURE_EXTENSION_BMP;
    }
    else if ( extension == ".tga" )
    {
        m_extension = DENOFIZ_TEXTURE_EXTENSION_TGA;
    }
    else if ( extension == ".hdr" )
    {
        m_extension = DENOFIZ_TEXTURE_EXTENSION_HDR;
    }
    else if ( extension == ".gif" )
    {
        m_extension = DENOFIZ_TEXTURE_EXTENSION_GIF;
    }
    else if ( extension == ".pic" )
    {
        m_extension = DENOFIZ_TEXTURE_EXTENSION_PIC;
    }

    switch ( m_extension )
    {
    case DENOFIZ_TEXTURE_EXTENSION_DDS:
        LoadTextureDDS( );
        break;
    default:
        LoadTextureSTB( );
        break;
    }
}

TextureDataImpl::TextureDataImpl( const DenOfIz_ByteArrayView &data, const DenOfIz_TextureExtension extension )
{
    m_extension = extension;
    LoadTextureFromMemory( data.Elements, data.NumElements );
}

DenOfIz_TextureExtension TextureDataImpl::IdentifyTextureFormat( const DenOfIz_ByteArrayView &data )
{
    const size_t dataNumBytes = data.NumElements;
    if ( dataNumBytes == 0 )
    {
        spdlog::error( "Data array is empty" );
        return DENOFIZ_TEXTURE_EXTENSION_DDS;
    }

    const auto *bytes = data.Elements;
    if ( dataNumBytes >= 4 && bytes[ 0 ] == 'D' && bytes[ 1 ] == 'D' && bytes[ 2 ] == 'S' && bytes[ 3 ] == ' ' )
    {
        return DENOFIZ_TEXTURE_EXTENSION_DDS;
    }
    if ( dataNumBytes >= 8 && bytes[ 0 ] == 0x89 && bytes[ 1 ] == 'P' && bytes[ 2 ] == 'N' && bytes[ 3 ] == 'G' && bytes[ 4 ] == 0x0D && bytes[ 5 ] == 0x0A && bytes[ 6 ] == 0x1A &&
         bytes[ 7 ] == 0x0A )
    {
        return DENOFIZ_TEXTURE_EXTENSION_PNG;
    }
    if ( dataNumBytes >= 3 && bytes[ 0 ] == 0xFF && bytes[ 1 ] == 0xD8 && bytes[ 2 ] == 0xFF )
    {
        return DENOFIZ_TEXTURE_EXTENSION_JPG;
    }
    return DENOFIZ_TEXTURE_EXTENSION_DDS;
}

void TextureDataImpl::LoadTextureSTB( )
{
    int width, height, channels;

    const stbi_uc *contents = stbi_load( m_path.c_str( ), &width, &height, &channels, STBI_rgb_alpha );

    if ( contents == nullptr )
    {
        spdlog::warn( "Error loading texture: {} , reason:{}", m_path, stbi_failure_reason( ) );
        return;
    }
    m_width        = static_cast<uint32_t>( std::max<int>( 1, width ) );
    m_height       = static_cast<uint32_t>( std::max<int>( 1, height ) );
    m_depth        = 1;
    m_format       = DENOFIZ_FORMAT_R8G8B8A8_UNORM;
    m_dimension    = DENOFIZ_TEXTURE_DIMENSION_TEXTURE2D;
    m_arraySize    = 1;
    m_mipLevels    = 1;
    m_bitsPerPixel = 32;
    m_blockSize    = 1;
    m_rowPitch     = m_width * 4;
    m_numRows      = m_height;
    m_slicePitch   = m_rowPitch * m_numRows;
    m_data.resize( m_slicePitch );
    std::memcpy( m_data.data( ), contents, m_slicePitch );

    stbi_image_free( const_cast<stbi_uc *>( contents ) );
}

DenOfIz_Format GetFormatFromDDS( const dds::DXGI_FORMAT &format )
{
    switch ( format )
    {
    case dds::DXGI_FORMAT_UNKNOWN:
        return DENOFIZ_FORMAT_UNDEFINED;
    case dds::DXGI_FORMAT_R32G32B32A32_TYPELESS:
        return DENOFIZ_FORMAT_R32G32B32A32_TYPELESS;
    case dds::DXGI_FORMAT_R32G32B32A32_FLOAT:
        return DENOFIZ_FORMAT_R32G32B32A32_FLOAT;
    case dds::DXGI_FORMAT_R32G32B32A32_UINT:
        return DENOFIZ_FORMAT_R32G32B32A32_UINT;
    case dds::DXGI_FORMAT_R32G32B32A32_SINT:
        return DENOFIZ_FORMAT_R32G32B32A32_SINT;
    case dds::DXGI_FORMAT_R32G32B32_FLOAT:
        return DENOFIZ_FORMAT_R32G32B32_FLOAT;
    case dds::DXGI_FORMAT_R32G32B32_UINT:
        return DENOFIZ_FORMAT_R32G32B32_UINT;
    case dds::DXGI_FORMAT_R32G32B32_SINT:
        return DENOFIZ_FORMAT_R32G32B32_SINT;
    case dds::DXGI_FORMAT_R16G16B16A16_TYPELESS:
        return DENOFIZ_FORMAT_R16G16B16A16_TYPELESS;
    case dds::DXGI_FORMAT_R16G16B16A16_FLOAT:
        return DENOFIZ_FORMAT_R16G16B16A16_FLOAT;
    case dds::DXGI_FORMAT_R16G16B16A16_UNORM:
        return DENOFIZ_FORMAT_R16G16B16A16_UNORM;
    case dds::DXGI_FORMAT_R16G16B16A16_UINT:
        return DENOFIZ_FORMAT_R16G16B16A16_UINT;
    case dds::DXGI_FORMAT_R16G16B16A16_SNORM:
        return DENOFIZ_FORMAT_R16G16B16A16_SNORM;
    case dds::DXGI_FORMAT_R16G16B16A16_SINT:
        return DENOFIZ_FORMAT_R16G16B16A16_SINT;
    case dds::DXGI_FORMAT_R32G32_TYPELESS:
        return DENOFIZ_FORMAT_R32G32_TYPELESS;
    case dds::DXGI_FORMAT_R32G32_FLOAT:
        return DENOFIZ_FORMAT_R32G32_FLOAT;
    case dds::DXGI_FORMAT_R32G32_UINT:
        return DENOFIZ_FORMAT_R32G32_UINT;
    case dds::DXGI_FORMAT_R32G32_SINT:
        return DENOFIZ_FORMAT_R32G32_SINT;
    case dds::DXGI_FORMAT_R10G10B10A2_TYPELESS:
        return DENOFIZ_FORMAT_R10G10B10A2_TYPELESS;
    case dds::DXGI_FORMAT_R10G10B10A2_UNORM:
        return DENOFIZ_FORMAT_R10G10B10A2_UNORM;
    case dds::DXGI_FORMAT_R10G10B10A2_UINT:
        return DENOFIZ_FORMAT_R10G10B10A2_UINT;
    case dds::DXGI_FORMAT_R8G8B8A8_TYPELESS:
        return DENOFIZ_FORMAT_R8G8B8A8_TYPELESS;
    case dds::DXGI_FORMAT_R8G8B8A8_UNORM:
        return DENOFIZ_FORMAT_R8G8B8A8_UNORM;
    case dds::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        return DENOFIZ_FORMAT_R8G8B8A8_UNORM_SRGB;
    case dds::DXGI_FORMAT_R8G8B8A8_UINT:
        return DENOFIZ_FORMAT_R8G8B8A8_UINT;
    case dds::DXGI_FORMAT_R8G8B8A8_SNORM:
        return DENOFIZ_FORMAT_R8G8B8A8_SNORM;
    case dds::DXGI_FORMAT_R8G8B8A8_SINT:
        return DENOFIZ_FORMAT_R8G8B8A8_SINT;
    case dds::DXGI_FORMAT_R16G16_TYPELESS:
        return DENOFIZ_FORMAT_R16G16_TYPELESS;
    case dds::DXGI_FORMAT_R16G16_FLOAT:
        return DENOFIZ_FORMAT_R16G16_FLOAT;
    case dds::DXGI_FORMAT_R16G16_UNORM:
        return DENOFIZ_FORMAT_R16G16_UNORM;
    case dds::DXGI_FORMAT_R16G16_UINT:
        return DENOFIZ_FORMAT_R16G16_UINT;
    case dds::DXGI_FORMAT_R16G16_SNORM:
        return DENOFIZ_FORMAT_R16G16_SNORM;
    case dds::DXGI_FORMAT_R16G16_SINT:
        return DENOFIZ_FORMAT_R16G16_SINT;
    case dds::DXGI_FORMAT_R32_TYPELESS:
        return DENOFIZ_FORMAT_R32_TYPELESS;
    case dds::DXGI_FORMAT_D32_FLOAT:
        return DENOFIZ_FORMAT_D32_FLOAT;
    case dds::DXGI_FORMAT_R32_FLOAT:
        return DENOFIZ_FORMAT_R32_FLOAT;
    case dds::DXGI_FORMAT_R32_UINT:
        return DENOFIZ_FORMAT_R32_UINT;
    case dds::DXGI_FORMAT_R32_SINT:
        return DENOFIZ_FORMAT_R32_SINT;
    case dds::DXGI_FORMAT_R8G8_TYPELESS:
        return DENOFIZ_FORMAT_R8G8_TYPELESS;
    case dds::DXGI_FORMAT_R8G8_UNORM:
        return DENOFIZ_FORMAT_R8G8_UNORM;
    case dds::DXGI_FORMAT_R8G8_UINT:
        return DENOFIZ_FORMAT_R8G8_UINT;
    case dds::DXGI_FORMAT_R8G8_SNORM:
        return DENOFIZ_FORMAT_R8G8_SNORM;
    case dds::DXGI_FORMAT_R8G8_SINT:
        return DENOFIZ_FORMAT_R8G8_SINT;
    case dds::DXGI_FORMAT_R16_TYPELESS:
        return DENOFIZ_FORMAT_R16_TYPELESS;
    case dds::DXGI_FORMAT_R16_FLOAT:
        return DENOFIZ_FORMAT_R16_FLOAT;
    case dds::DXGI_FORMAT_D16_UNORM:
        return DENOFIZ_FORMAT_D16_UNORM;
    case dds::DXGI_FORMAT_R16_UNORM:
        return DENOFIZ_FORMAT_R16_UNORM;
    case dds::DXGI_FORMAT_R16_UINT:
        return DENOFIZ_FORMAT_R16_UINT;
    case dds::DXGI_FORMAT_R16_SNORM:
        return DENOFIZ_FORMAT_R16_SNORM;
    case dds::DXGI_FORMAT_R16_SINT:
        return DENOFIZ_FORMAT_R16_SINT;
    case dds::DXGI_FORMAT_R8_TYPELESS:
        return DENOFIZ_FORMAT_R8_TYPELESS;
    case dds::DXGI_FORMAT_R8_UNORM:
        return DENOFIZ_FORMAT_R8_UNORM;
    case dds::DXGI_FORMAT_R8_UINT:
        return DENOFIZ_FORMAT_R8_UINT;
    case dds::DXGI_FORMAT_R8_SNORM:
        return DENOFIZ_FORMAT_R8_SNORM;
    case dds::DXGI_FORMAT_R8_SINT:
        return DENOFIZ_FORMAT_R8_SINT;
    case dds::DXGI_FORMAT_BC2_UNORM:
        return DENOFIZ_FORMAT_BC2_UNORM;
    case dds::DXGI_FORMAT_BC3_UNORM:
        return DENOFIZ_FORMAT_BC3_UNORM;
    case dds::DXGI_FORMAT_BC4_UNORM:
        return DENOFIZ_FORMAT_BC4_UNORM;
    case dds::DXGI_FORMAT_BC4_SNORM:
        return DENOFIZ_FORMAT_BC4_SNORM;
    case dds::DXGI_FORMAT_BC5_UNORM:
        return DENOFIZ_FORMAT_BC5_UNORM;
    case dds::DXGI_FORMAT_BC5_SNORM:
        return DENOFIZ_FORMAT_BC5_SNORM;
    case dds::DXGI_FORMAT_B8G8R8A8_UNORM:
        return DENOFIZ_FORMAT_B8G8R8A8_UNORM;
    case dds::DXGI_FORMAT_BC7_UNORM:
        return DENOFIZ_FORMAT_BC7_UNORM;
    case dds::DXGI_FORMAT_BC7_TYPELESS:
    case dds::DXGI_FORMAT_BC7_UNORM_SRGB:
        return DENOFIZ_FORMAT_BC7_UNORM_SRGB;
    case dds::DXGI_FORMAT_BC1_TYPELESS:
        return DENOFIZ_FORMAT_BC1_UNORM;
    case dds::DXGI_FORMAT_BC2_TYPELESS:
        return DENOFIZ_FORMAT_BC2_UNORM;
    case dds::DXGI_FORMAT_BC3_TYPELESS:
        return DENOFIZ_FORMAT_BC3_UNORM;
    case dds::DXGI_FORMAT_BC4_TYPELESS:
        return DENOFIZ_FORMAT_BC4_UNORM;
    case dds::DXGI_FORMAT_BC5_TYPELESS:
        return DENOFIZ_FORMAT_BC5_UNORM;
    case dds::DXGI_FORMAT_BC1_UNORM:
        return DENOFIZ_FORMAT_BC1_UNORM;
    case dds::DXGI_FORMAT_BC1_UNORM_SRGB:
        return DENOFIZ_FORMAT_BC1_UNORM_SRGB;
    case dds::DXGI_FORMAT_BC2_UNORM_SRGB:
        return DENOFIZ_FORMAT_BC2_UNORM_SRGB;
    case dds::DXGI_FORMAT_R11G11B10_FLOAT:
    case dds::DXGI_FORMAT_R32G8X24_TYPELESS:
    case dds::DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS:
    case dds::DXGI_FORMAT_X32_TYPELESS_G8X24_UINT:
    case dds::DXGI_FORMAT_R24G8_TYPELESS:
    case dds::DXGI_FORMAT_D24_UNORM_S8_UINT:
    case dds::DXGI_FORMAT_R24_UNORM_X8_TYPELESS:
    case dds::DXGI_FORMAT_X24_TYPELESS_G8_UINT:
    case dds::DXGI_FORMAT_A8_UNORM:
    case dds::DXGI_FORMAT_R1_UNORM:
    case dds::DXGI_FORMAT_R9G9B9E5_SHAREDEXP:
    case dds::DXGI_FORMAT_R8G8_B8G8_UNORM:
    case dds::DXGI_FORMAT_G8R8_G8B8_UNORM:
    case dds::DXGI_FORMAT_B5G6R5_UNORM:
    case dds::DXGI_FORMAT_B5G5R5A1_UNORM:
    case dds::DXGI_FORMAT_B8G8R8X8_TYPELESS:
    case dds::DXGI_FORMAT_B8G8R8X8_UNORM:
    case dds::DXGI_FORMAT_R10G10B10_XR_BIAS_A2_UNORM:
    case dds::DXGI_FORMAT_B8G8R8A8_TYPELESS:
    case dds::DXGI_FORMAT_B8G8R8A8_UNORM_SRGB:
    case dds::DXGI_FORMAT_BC3_UNORM_SRGB:
    case dds::DXGI_FORMAT_B8G8R8X8_UNORM_SRGB:
    case dds::DXGI_FORMAT_BC6H_TYPELESS:
    case dds::DXGI_FORMAT_BC6H_UF16:
    case dds::DXGI_FORMAT_BC6H_SF16:
    case dds::DXGI_FORMAT_AYUV:
    case dds::DXGI_FORMAT_Y410:
    case dds::DXGI_FORMAT_Y416:
    case dds::DXGI_FORMAT_NV12:
    case dds::DXGI_FORMAT_P010:
    case dds::DXGI_FORMAT_P016:
    case dds::DXGI_FORMAT_420_OPAQUE:
    case dds::DXGI_FORMAT_YUY2:
    case dds::DXGI_FORMAT_Y210:
    case dds::DXGI_FORMAT_Y216:
    case dds::DXGI_FORMAT_NV11:
    case dds::DXGI_FORMAT_AI44:
    case dds::DXGI_FORMAT_IA44:
    case dds::DXGI_FORMAT_P8:
    case dds::DXGI_FORMAT_A8P8:
    case dds::DXGI_FORMAT_B4G4R4A4_UNORM:
    case dds::DXGI_FORMAT_SAMPLER_FEEDBACK_MIN_MIP_OPAQUE:
    case dds::DXGI_FORMAT_SAMPLER_FEEDBACK_MIP_REGION_USED_OPAQUE:
    case dds::DXGI_FORMAT_P208:
    case dds::DXGI_FORMAT_V208:
    case dds::DXGI_FORMAT_V408:
    case dds::D3DFMT_R8G8B8:
    case dds::DXGI_FORMAT_R32G32B32_TYPELESS:
    case dds::DXGI_FORMAT_D32_FLOAT_S8X24_UINT:
    case dds::DXGI_FORMAT_FORCE_DWORD:
        return DENOFIZ_FORMAT_UNDEFINED;
    }
    return DENOFIZ_FORMAT_UNDEFINED;
}

void TextureDataImpl::LoadTextureDDS( )
{
    std::ifstream file( m_path, std::ios::binary | std::ios::ate );
    DZ_RETURN_IF( !file.is_open( ) );
    if ( !file.is_open( ) )
    {
        spdlog::warn( "Error loading texture: {} , reason: File not found", m_path );
        return;
    }

    const std::streamsize size = file.tellg( );
    file.seekg( 0, std::ios::beg );

    std::vector<Byte> fileDataHeap( size );
    Byte             *fileData = fileDataHeap.data( );

    if ( !file.read( reinterpret_cast<char *>( fileData ), size ) )
    {
        spdlog::warn( "Error loading texture: {} , reason: File read error", m_path );
        return;
    }

    const dds::Header header = dds::read_header( fileData, size );
    m_ddsHeader              = std::unique_ptr<dds::Header, DDSHeaderDeleter>( new dds::Header( header ) );
    if ( !m_ddsHeader->is_valid( ) )
    {
        spdlog::warn( "Error loading texture: {} , reason: Invalid DDS header", m_path );
        return;
    }

    m_arraySize    = 1;
    m_width        = m_ddsHeader->width( );
    m_height       = m_ddsHeader->height( );
    m_depth        = m_ddsHeader->depth( );
    m_mipLevels    = m_ddsHeader->mip_levels( );
    m_arraySize    = m_ddsHeader->array_size( );
    m_format       = GetFormatFromDDS( m_ddsHeader->format( ) );
    m_bitsPerPixel = m_ddsHeader->bits_per_element( );
    m_blockSize    = m_ddsHeader->block_size( );

    m_rowPitch   = m_ddsHeader->row_pitch( 0 );
    m_numRows    = DenOfIz_Format_IsBC( m_format ) ? std::max( 1U, ( m_height + m_blockSize - 1 ) / m_blockSize ) : m_height;
    m_slicePitch = m_ddsHeader->slice_pitch( 0 );

    m_data.resize( m_ddsHeader->data_size( ) );
    std::memcpy( m_data.data( ), fileData + m_ddsHeader->data_offset( ), m_ddsHeader->data_size( ) );

    if ( m_ddsHeader->is_1d( ) )
    {
        m_dimension = DENOFIZ_TEXTURE_DIMENSION_TEXTURE1D;
    }
    else if ( m_ddsHeader->is_3d( ) )
    {
        m_dimension = DENOFIZ_TEXTURE_DIMENSION_TEXTURE3D;
    }
    else
    {
        m_dimension = DENOFIZ_TEXTURE_DIMENSION_TEXTURE2D;
    }
    if ( m_ddsHeader->is_cubemap( ) )
    {
        m_dimension = DENOFIZ_TEXTURE_DIMENSION_TEXTURE_CUBE;
    }
}

uint32_t TextureDataImpl::GetWidth( ) const
{
    return m_width;
}

uint32_t TextureDataImpl::GetHeight( ) const
{
    return m_height;
}

uint32_t TextureDataImpl::GetDepth( ) const
{
    return m_depth;
}

uint32_t TextureDataImpl::GetMipLevels( ) const
{
    return m_mipLevels;
}

uint32_t TextureDataImpl::GetArraySize( ) const
{
    return m_arraySize;
}

uint32_t TextureDataImpl::GetBitsPerPixel( ) const
{
    return m_bitsPerPixel;
}

uint32_t TextureDataImpl::GetBlockSize( ) const
{
    return m_blockSize;
}

uint32_t TextureDataImpl::GetRowPitch( ) const
{
    return m_rowPitch;
}

uint32_t TextureDataImpl::GetNumRows( ) const
{
    return m_numRows;
}

uint32_t TextureDataImpl::GetSlicePitch( ) const
{
    return m_slicePitch;
}

DenOfIz_Format TextureDataImpl::GetFormat( ) const
{
    return m_format;
}

DenOfIz_TextureDimension TextureDataImpl::GetDimension( ) const
{
    return m_dimension;
}

DenOfIz_TextureExtension TextureDataImpl::GetExtension( ) const
{
    return m_extension;
}

DenOfIz_ByteArrayView TextureDataImpl::GetData( ) const
{
    return { m_data.data( ), m_data.size( ) };
}

DenOfIz_TextureMipArray TextureDataImpl::ReadMipData( ) const
{
    DenOfIz_TextureMipArray mipData;

    switch ( m_extension )
    {
    case DENOFIZ_TEXTURE_EXTENSION_DDS:
        {
            const uint32_t totalMips = m_arraySize * m_mipLevels;
            m_mipData.clear( );
            m_mipData.resize( totalMips );

            uint32_t mipIndex = 0;
            for ( uint32_t array = 0; array < m_arraySize; ++array )
            {
                for ( uint32_t mip = 0; mip < m_mipLevels; ++mip )
                {
                    const auto externalOffset = m_ddsHeader->mip_offset( mip, array ) - m_ddsHeader->data_offset( );

                    DenOfIz_TextureMip &mipInfo = m_mipData[ mipIndex++ ];
                    mipInfo.Width               = std::max( 1U, m_ddsHeader->width( ) >> mip );
                    mipInfo.Height              = std::max( 1U, m_ddsHeader->height( ) >> mip );
                    mipInfo.MipIndex            = mip;
                    mipInfo.ArrayIndex          = array;
                    mipInfo.RowPitch            = m_ddsHeader->row_pitch( mip );
                    if ( DenOfIz_Format_IsBC( m_format ) )
                    {
                        const uint32_t blockSize = DenOfIz_Format_BlockSize( m_format );
                        mipInfo.NumRows          = std::max( 1U, ( mipInfo.Height + blockSize - 1 ) / blockSize );
                    }
                    else
                    {
                        mipInfo.NumRows = mipInfo.Height;
                    }
                    mipInfo.SlicePitch = m_ddsHeader->slice_pitch( mip );
                    mipInfo.DataOffset = externalOffset;
                }
            }
        }
        break;
    default:
        m_mipData.clear( );
        m_mipData.resize( 1 );

        DenOfIz_TextureMip &mipInfo = m_mipData[ 0 ];
        mipInfo.Width               = m_width;
        mipInfo.Height              = m_height;
        mipInfo.MipIndex            = 0;
        mipInfo.ArrayIndex          = 0;
        mipInfo.RowPitch            = m_rowPitch;
        mipInfo.NumRows             = m_numRows;
        mipInfo.SlicePitch          = m_slicePitch;
        mipInfo.DataOffset          = 0;
        break;
    }

    mipData.Elements    = m_mipData.empty( ) ? nullptr : m_mipData.data( );
    mipData.NumElements = static_cast<uint32_t>( m_mipData.size( ) );

    return mipData;
}

void TextureDataImpl::LoadTextureFromMemory( const Byte *data, const size_t dataNumBytes )
{
    switch ( m_extension )
    {
    case DENOFIZ_TEXTURE_EXTENSION_DDS:
        LoadTextureDDSFromMemory( data, dataNumBytes );
        break;
    default:
        LoadTextureSTBFromMemory( data, dataNumBytes );
        break;
    }
}

void TextureDataImpl::LoadTextureDDSFromMemory( const Byte *data, const size_t dataNumBytes )
{
    if ( data == nullptr || dataNumBytes < sizeof( dds::Header ) )
    {
        spdlog::warn( "Invalid DDS data provided" );
        return;
    }

    const dds::Header header = dds::read_header( data, dataNumBytes );
    m_ddsHeader              = std::unique_ptr<dds::Header, DDSHeaderDeleter>( new dds::Header( header ) );
    if ( !m_ddsHeader->is_valid( ) )
    {
        spdlog::warn( "Error loading texture from memory: Invalid DDS header" );
        return;
    }

    m_arraySize    = 1;
    m_width        = m_ddsHeader->width( );
    m_height       = m_ddsHeader->height( );
    m_depth        = m_ddsHeader->depth( );
    m_mipLevels    = m_ddsHeader->mip_levels( );
    m_arraySize    = m_ddsHeader->array_size( );
    m_format       = GetFormatFromDDS( m_ddsHeader->format( ) );
    m_bitsPerPixel = m_ddsHeader->bits_per_element( );
    m_blockSize    = m_ddsHeader->block_size( );

    m_rowPitch   = m_ddsHeader->row_pitch( 0 );
    m_numRows    = DenOfIz_Format_IsBC( m_format ) ? std::max( 1U, ( m_height + m_blockSize - 1 ) / m_blockSize ) : m_height;
    m_slicePitch = m_ddsHeader->slice_pitch( 0 );

    m_data.resize( m_ddsHeader->data_size( ) );
    const uint8_t *srcData = data + m_ddsHeader->data_offset( );
    std::memcpy( m_data.data( ), srcData, m_ddsHeader->data_size( ) );

    if ( m_ddsHeader->is_1d( ) )
    {
        m_dimension = DENOFIZ_TEXTURE_DIMENSION_TEXTURE1D;
    }
    else if ( m_ddsHeader->is_3d( ) )
    {
        m_dimension = DENOFIZ_TEXTURE_DIMENSION_TEXTURE3D;
    }
    else
    {
        m_dimension = DENOFIZ_TEXTURE_DIMENSION_TEXTURE2D;
    }
    if ( m_ddsHeader->is_cubemap( ) )
    {
        m_dimension = DENOFIZ_TEXTURE_DIMENSION_TEXTURE_CUBE;
    }
}

void TextureDataImpl::LoadTextureSTBFromMemory( const Byte *data, const size_t dataNumBytes )
{
    int width, height, channels;

    stbi_uc *contents = stbi_load_from_memory( static_cast<const stbi_uc *>( data ), static_cast<int>( dataNumBytes ), &width, &height, &channels, STBI_rgb_alpha );

    if ( contents == nullptr )
    {
        spdlog::warn( "Error loading texture from memory with STB, reason: {}", stbi_failure_reason( ) );
        return;
    }

    m_width        = static_cast<uint32_t>( std::max<int>( 1, width ) );
    m_height       = static_cast<uint32_t>( std::max<int>( 1, height ) );
    m_depth        = 1;
    m_format       = DENOFIZ_FORMAT_R8G8B8A8_UNORM;
    m_dimension    = DENOFIZ_TEXTURE_DIMENSION_TEXTURE2D;
    m_arraySize    = 1;
    m_mipLevels    = 1;
    m_bitsPerPixel = 32;
    m_blockSize    = 1;
    m_rowPitch     = m_width * 4;
    m_numRows      = m_height;
    m_slicePitch   = m_rowPitch * m_numRows;
    m_data.resize( m_slicePitch );
    std::memcpy( m_data.data( ), contents, m_slicePitch );

    stbi_image_free( contents );
}

extern "C"
{
    DenOfIz_TextureData DenOfIz_TextureData_CreateFromPath( const DenOfIz_TextureCreateFromPathDesc *desc )
    {
        if ( desc == NULL )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        auto *texture = new DenOfIz::TextureDataImpl( desc->Path );
        return DENOFIZ_TO_HANDLE( texture );
    }

    DenOfIz_TextureData DenOfIz_TextureData_CreateFromData( const DenOfIz_TextureCreateFromDataDesc *desc )
    {
        if ( desc == NULL )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        auto *texture = new DenOfIz::TextureDataImpl( desc->Data, desc->Extension );
        return DENOFIZ_TO_HANDLE( texture );
    }

    DenOfIz_TextureExtension DenOfIz_TextureData_IdentifyTextureFormat( const DenOfIz_ByteArrayView *data )
    {
        if ( data == NULL )
        {
            return DENOFIZ_TEXTURE_EXTENSION_DDS;
        }
        return DenOfIz::TextureDataImpl::IdentifyTextureFormat( *data );
    }

    DenOfIz_TextureMipArray DenOfIz_TextureData_ReadMipData( DenOfIz_TextureData texture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return DenOfIz_TextureMipArray{ };
        }
        return TEXTURE_IMPL( texture )->ReadMipData( );
    }

    uint32_t DenOfIz_TextureData_GetWidth( DenOfIz_TextureData texture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return 0;
        }
        return TEXTURE_IMPL( texture )->GetWidth( );
    }

    uint32_t DenOfIz_TextureData_GetHeight( DenOfIz_TextureData texture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return 0;
        }
        return TEXTURE_IMPL( texture )->GetHeight( );
    }

    uint32_t DenOfIz_TextureData_GetDepth( DenOfIz_TextureData texture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return 0;
        }
        return TEXTURE_IMPL( texture )->GetDepth( );
    }

    uint32_t DenOfIz_TextureData_GetMipLevels( DenOfIz_TextureData texture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return 0;
        }
        return TEXTURE_IMPL( texture )->GetMipLevels( );
    }

    uint32_t DenOfIz_TextureData_GetArraySize( DenOfIz_TextureData texture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return 0;
        }
        return TEXTURE_IMPL( texture )->GetArraySize( );
    }

    uint32_t DenOfIz_TextureData_GetBitsPerPixel( DenOfIz_TextureData texture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return 0;
        }
        return TEXTURE_IMPL( texture )->GetBitsPerPixel( );
    }

    uint32_t DenOfIz_TextureData_GetBlockSize( DenOfIz_TextureData texture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return 0;
        }
        return TEXTURE_IMPL( texture )->GetBlockSize( );
    }

    uint32_t DenOfIz_TextureData_GetRowPitch( DenOfIz_TextureData texture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return 0;
        }
        return TEXTURE_IMPL( texture )->GetRowPitch( );
    }

    uint32_t DenOfIz_TextureData_GetNumRows( DenOfIz_TextureData texture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return 0;
        }
        return TEXTURE_IMPL( texture )->GetNumRows( );
    }

    uint32_t DenOfIz_TextureData_GetSlicePitch( DenOfIz_TextureData texture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return 0;
        }
        return TEXTURE_IMPL( texture )->GetSlicePitch( );
    }

    DenOfIz_Format DenOfIz_TextureData_GetFormat( DenOfIz_TextureData texture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return DENOFIZ_FORMAT_UNDEFINED;
        }
        return TEXTURE_IMPL( texture )->GetFormat( );
    }

    DenOfIz_TextureDimension DenOfIz_TextureData_GetDimension( DenOfIz_TextureData texture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return DENOFIZ_TEXTURE_DIMENSION_UNDEFINED;
        }
        return TEXTURE_IMPL( texture )->GetDimension( );
    }

    DenOfIz_TextureExtension DenOfIz_TextureData_GetExtension( DenOfIz_TextureData texture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return DENOFIZ_TEXTURE_EXTENSION_DDS;
        }
        return TEXTURE_IMPL( texture )->GetExtension( );
    }

    DenOfIz_ByteArrayView DenOfIz_TextureData_GetData( DenOfIz_TextureData texture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return DenOfIz_ByteArrayView{ };
        }
        return TEXTURE_IMPL( texture )->GetData( );
    }

    void DenOfIz_TextureData_Destroy( DenOfIz_TextureData texture )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( texture ) )
        {
            return;
        }
        delete TEXTURE_IMPL( texture );
    }
}
