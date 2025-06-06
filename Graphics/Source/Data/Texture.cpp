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

#include "DenOfIzGraphics/Data/Texture.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

#ifdef STB_IMPLEMENTATION
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#include "stb_image.h"

#include "dds.h"
#include <filesystem>
#include <fstream>

using namespace DenOfIz;

void DDSHeaderDeleter::operator( )( const dds::Header *ptr ) const
{
    delete ptr;
}

Texture::Texture( const InteropString &path ) : m_path( Utilities::AppPath( path.Get( ) ) )
{
    if ( !std::filesystem::exists( m_path ) )
    {
        LOG( ERROR ) << "Texture file does not exist: " << m_path;
        return;
    }
    if ( const std::filesystem::path &extension = std::filesystem::path( m_path ).extension( ); extension == ".dds" )
    {
        m_extension = TextureExtension::DDS;
    }
    else if ( extension == ".png" )
    {
        m_extension = TextureExtension::PNG;
    }
    else if ( extension == ".jpg" )
    {
        m_extension = TextureExtension::JPG;
    }
    else if ( extension == ".bmp" )
    {
        m_extension = TextureExtension::BMP;
    }
    else if ( extension == ".tga" )
    {
        m_extension = TextureExtension::TGA;
    }
    else if ( extension == ".hdr" )
    {
        m_extension = TextureExtension::HDR;
    }
    else if ( extension == ".gif" )
    {
        m_extension = TextureExtension::GIF;
    }
    else if ( extension == ".pic" )
    {
        m_extension = TextureExtension::PIC;
    }

    switch ( m_extension )
    {
    case TextureExtension::DDS:
        LoadTextureDDS( );
        break;
    default:
        LoadTextureSTB( );
        break;
    }
}

Texture::Texture( const InteropArray<Byte> &data, const TextureExtension extension )
{
    m_extension = extension;
    LoadTextureFromMemory( data.Data( ), data.NumElements( ) );
}

TextureExtension Texture::IdentifyTextureFormat( const InteropArray<Byte> &data )
{
    const size_t dataNumBytes = data.NumElements( );
    if ( dataNumBytes == 0 )
    {
        LOG( ERROR ) << "Data array is empty";
        return TextureExtension::DDS;
    }

    const auto *bytes = data.Data( );
    if ( dataNumBytes >= 4 && bytes[ 0 ] == 'D' && bytes[ 1 ] == 'D' && bytes[ 2 ] == 'S' && bytes[ 3 ] == ' ' )
    {
        return TextureExtension::DDS;
    }
    if ( dataNumBytes >= 8 && bytes[ 0 ] == 0x89 && bytes[ 1 ] == 'P' && bytes[ 2 ] == 'N' && bytes[ 3 ] == 'G' && bytes[ 4 ] == 0x0D && bytes[ 5 ] == 0x0A && bytes[ 6 ] == 0x1A &&
         bytes[ 7 ] == 0x0A )
    {
        return TextureExtension::PNG;
    }
    if ( dataNumBytes >= 3 && bytes[ 0 ] == 0xFF && bytes[ 1 ] == 0xD8 && bytes[ 2 ] == 0xFF )
    {
        return TextureExtension::JPG;
    }
    return TextureExtension::DDS;
}

void Texture::LoadTextureSTB( )
{
    int width, height, channels;

    const stbi_uc *contents = stbi_load( m_path.c_str( ), &width, &height, &channels, STBI_rgb_alpha );

    if ( contents == nullptr )
    {
        LOG( WARNING ) << "Error loading texture: " << m_path << ", reason:" << stbi_failure_reason( );
        return;
    }
    m_width        = static_cast<uint32_t>( std::max<int>( 1, width ) );
    m_height       = static_cast<uint32_t>( std::max<int>( 1, height ) );
    m_depth        = 1;
    m_format       = Format::R8G8B8A8Unorm;
    m_dimension    = TextureDimension::Texture2D;
    m_arraySize    = 1;
    m_mipLevels    = 1;
    m_bitsPerPixel = 32;
    m_blockSize    = 1;
    m_rowPitch     = m_width * 4;
    m_numRows      = m_height;
    m_slicePitch   = m_rowPitch * m_numRows;
    m_data.Resize( m_slicePitch );
    m_data.MemCpy( contents, m_slicePitch );
}

Format GetFormatFromDDS( const dds::DXGI_FORMAT &format )
{
    switch ( format )
    {
    case dds::DXGI_FORMAT_UNKNOWN:
        return Format::Undefined;
    case dds::DXGI_FORMAT_R32G32B32A32_TYPELESS:
        return Format::R32G32B32A32Typeless;
    case dds::DXGI_FORMAT_R32G32B32A32_FLOAT:
        return Format::R32G32B32A32Float;
    case dds::DXGI_FORMAT_R32G32B32A32_UINT:
        return Format::R32G32B32A32Uint;
    case dds::DXGI_FORMAT_R32G32B32A32_SINT:
        return Format::R32G32B32A32Sint;
    case dds::DXGI_FORMAT_R32G32B32_FLOAT:
        return Format::R32G32B32Float;
    case dds::DXGI_FORMAT_R32G32B32_UINT:
        return Format::R32G32B32Uint;
    case dds::DXGI_FORMAT_R32G32B32_SINT:
        return Format::R32G32B32Sint;
    case dds::DXGI_FORMAT_R16G16B16A16_TYPELESS:
        return Format::R16G16B16A16Typeless;
    case dds::DXGI_FORMAT_R16G16B16A16_FLOAT:
        return Format::R16G16B16A16Float;
    case dds::DXGI_FORMAT_R16G16B16A16_UNORM:
        return Format::R16G16B16A16Unorm;
    case dds::DXGI_FORMAT_R16G16B16A16_UINT:
        return Format::R16G16B16A16Uint;
    case dds::DXGI_FORMAT_R16G16B16A16_SNORM:
        return Format::R16G16B16A16Snorm;
    case dds::DXGI_FORMAT_R16G16B16A16_SINT:
        return Format::R16G16B16A16Sint;
    case dds::DXGI_FORMAT_R32G32_TYPELESS:
        return Format::R32G32Typeless;
    case dds::DXGI_FORMAT_R32G32_FLOAT:
        return Format::R32G32Float;
    case dds::DXGI_FORMAT_R32G32_UINT:
        return Format::R32G32Uint;
    case dds::DXGI_FORMAT_R32G32_SINT:
        return Format::R32G32Sint;
    case dds::DXGI_FORMAT_R10G10B10A2_TYPELESS:
        return Format::R10G10B10A2Typeless;
    case dds::DXGI_FORMAT_R10G10B10A2_UNORM:
        return Format::R10G10B10A2Unorm;
    case dds::DXGI_FORMAT_R10G10B10A2_UINT:
        return Format::R10G10B10A2Uint;
    case dds::DXGI_FORMAT_R8G8B8A8_TYPELESS:
        return Format::R8G8B8A8Typeless;
    case dds::DXGI_FORMAT_R8G8B8A8_UNORM:
        return Format::R8G8B8A8Unorm;
    case dds::DXGI_FORMAT_R8G8B8A8_UNORM_SRGB:
        return Format::R8G8B8A8UnormSrgb;
    case dds::DXGI_FORMAT_R8G8B8A8_UINT:
        return Format::R8G8B8A8Uint;
    case dds::DXGI_FORMAT_R8G8B8A8_SNORM:
        return Format::R8G8B8A8Snorm;
    case dds::DXGI_FORMAT_R8G8B8A8_SINT:
        return Format::R8G8B8A8Sint;
    case dds::DXGI_FORMAT_R16G16_TYPELESS:
        return Format::R16G16Typeless;
    case dds::DXGI_FORMAT_R16G16_FLOAT:
        return Format::R16G16Float;
    case dds::DXGI_FORMAT_R16G16_UNORM:
        return Format::R16G16Unorm;
    case dds::DXGI_FORMAT_R16G16_UINT:
        return Format::R16G16Uint;
    case dds::DXGI_FORMAT_R16G16_SNORM:
        return Format::R16G16Snorm;
    case dds::DXGI_FORMAT_R16G16_SINT:
        return Format::R16G16Sint;
    case dds::DXGI_FORMAT_R32_TYPELESS:
        return Format::R32Typeless;
    case dds::DXGI_FORMAT_D32_FLOAT:
        return Format::D32Float;
    case dds::DXGI_FORMAT_R32_FLOAT:
        return Format::R32Float;
    case dds::DXGI_FORMAT_R32_UINT:
        return Format::R32Uint;
    case dds::DXGI_FORMAT_R32_SINT:
        return Format::R32Sint;
    case dds::DXGI_FORMAT_R8G8_TYPELESS:
        return Format::R8G8Typeless;
    case dds::DXGI_FORMAT_R8G8_UNORM:
        return Format::R8G8Unorm;
    case dds::DXGI_FORMAT_R8G8_UINT:
        return Format::R8G8Uint;
    case dds::DXGI_FORMAT_R8G8_SNORM:
        return Format::R8G8Snorm;
    case dds::DXGI_FORMAT_R8G8_SINT:
        return Format::R8G8Sint;
    case dds::DXGI_FORMAT_R16_TYPELESS:
        return Format::R16Typeless;
    case dds::DXGI_FORMAT_R16_FLOAT:
        return Format::R16Float;
    case dds::DXGI_FORMAT_D16_UNORM:
        return Format::D16Unorm;
    case dds::DXGI_FORMAT_R16_UNORM:
        return Format::R16Unorm;
    case dds::DXGI_FORMAT_R16_UINT:
        return Format::R16Uint;
    case dds::DXGI_FORMAT_R16_SNORM:
        return Format::R16Snorm;
    case dds::DXGI_FORMAT_R16_SINT:
        return Format::R16Sint;
    case dds::DXGI_FORMAT_R8_TYPELESS:
        return Format::R8Typeless;
    case dds::DXGI_FORMAT_R8_UNORM:
        return Format::R8Unorm;
    case dds::DXGI_FORMAT_R8_UINT:
        return Format::R8Uint;
    case dds::DXGI_FORMAT_R8_SNORM:
        return Format::R8Snorm;
    case dds::DXGI_FORMAT_R8_SINT:
        return Format::R8Sint;
    case dds::DXGI_FORMAT_BC2_UNORM:
        return Format::BC2Unorm;
    case dds::DXGI_FORMAT_BC3_UNORM:
        return Format::BC3Unorm;
    case dds::DXGI_FORMAT_BC4_UNORM:
        return Format::BC4Unorm;
    case dds::DXGI_FORMAT_BC4_SNORM:
        return Format::BC4Snorm;
    case dds::DXGI_FORMAT_BC5_UNORM:
        return Format::BC5Unorm;
    case dds::DXGI_FORMAT_BC5_SNORM:
        return Format::BC5Snorm;
    case dds::DXGI_FORMAT_B8G8R8A8_UNORM:
        return Format::B8G8R8A8Unorm;
    case dds::DXGI_FORMAT_BC7_UNORM:
        return Format::BC7Unorm;
    // Is Typeless to Unorm fine?
    case dds::DXGI_FORMAT_BC1_TYPELESS:
        return Format::BC1Unorm;
    case dds::DXGI_FORMAT_BC2_TYPELESS:
        return Format::BC2Unorm;
    case dds::DXGI_FORMAT_BC3_TYPELESS:
        return Format::BC3Unorm;
    case dds::DXGI_FORMAT_BC4_TYPELESS:
        return Format::BC4Unorm;
    case dds::DXGI_FORMAT_BC5_TYPELESS:
        return Format::BC5Unorm;
    case dds::DXGI_FORMAT_BC1_UNORM:
        return Format::BC1Unorm;
    case dds::DXGI_FORMAT_BC1_UNORM_SRGB:
        return Format::BC1UnormSrgb;
    case dds::DXGI_FORMAT_BC2_UNORM_SRGB:
        return Format::BC2UnormSrgb;
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
    case dds::DXGI_FORMAT_BC7_TYPELESS:
    case dds::DXGI_FORMAT_BC7_UNORM_SRGB:
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
        return Format::Undefined;
    }
    return Format::Undefined;
}

void Texture::LoadTextureDDS( )
{
    // Step 1: Read the DDS file
    std::ifstream file( m_path, std::ios::binary | std::ios::ate );
    DZ_RETURN_IF( !file.is_open( ) );
    if ( !file.is_open( ) )
    {
        LOG( WARNING ) << "Error loading texture: " << m_path << ", reason: File not found";
        return;
    }

    const std::streamsize size = file.tellg( );
    file.seekg( 0, std::ios::beg );

    std::vector<Byte> fileDataHeap( size );
    Byte             *fileData = fileDataHeap.data( );

    if ( !file.read( reinterpret_cast<char *>( fileData ), size ) )
    {
        LOG( WARNING ) << "Error loading texture: " << m_path << ", reason: File read error";
        return;
    }

    const dds::Header header = dds::read_header( fileData, size );
    m_ddsHeader              = std::unique_ptr<dds::Header, DDSHeaderDeleter>( new dds::Header( header ) );
    if ( !m_ddsHeader->is_valid( ) )
    {
        LOG( WARNING ) << "Error loading texture: " << m_path << ", reason: Invalid DDS header";
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
    m_rowPitch     = std::max( 1U, ( m_width + ( m_blockSize - 1 ) ) / m_blockSize ) * m_bitsPerPixel >> 3;
    m_numRows      = std::max( 1U, ( m_height + ( m_blockSize - 1 ) ) / m_blockSize );
    m_slicePitch   = m_rowPitch * m_numRows;

    m_data.Resize( m_ddsHeader->data_size( ) );
    m_data.MemCpy( fileData + m_ddsHeader->data_offset( ), m_ddsHeader->data_size( ) );

    if ( m_ddsHeader->is_1d( ) )
    {
        m_dimension = TextureDimension::Texture1D;
    }
    else if ( m_ddsHeader->is_3d( ) )
    {
        m_dimension = TextureDimension::Texture3D;
    }
    else
    {
        m_dimension = TextureDimension::Texture2D;
    }
    if ( m_ddsHeader->is_cubemap( ) )
    {
        m_dimension = TextureDimension::TextureCube;
    }

    if ( IsFormatBC( m_format ) )
    {
        m_width  = Utilities::Align( m_width, FormatBlockSize( m_format ) );
        m_height = Utilities::Align( m_height, FormatBlockSize( m_format ) );
    }
}

void Texture::StreamMipData( const MipStreamCallback &callback ) const
{
    switch ( m_extension )
    {
    case TextureExtension::DDS:
        StreamMipDataDDS( callback );
        break;
    default:
        StreamMipDataSTB( callback );
        break;
    }
}

uint32_t Texture::GetWidth( ) const
{
    return m_width;
}

uint32_t Texture::GetHeight( ) const
{
    return m_height;
}

uint32_t Texture::GetDepth( ) const
{
    return m_depth;
}

uint32_t Texture::GetMipLevels( ) const
{
    return m_mipLevels;
}

uint32_t Texture::GetArraySize( ) const
{
    return m_arraySize;
}

uint32_t Texture::GetBitsPerPixel( ) const
{
    return m_bitsPerPixel;
}

uint32_t Texture::GetBlockSize( ) const
{
    return m_blockSize;
}

uint32_t Texture::GetRowPitch( ) const
{
    return m_rowPitch;
}

uint32_t Texture::GetNumRows( ) const
{
    return m_numRows;
}

uint32_t Texture::GetSlicePitch( ) const
{
    return m_slicePitch;
}

Format Texture::GetFormat( ) const
{
    return m_format;
}

TextureDimension Texture::GetDimension( ) const
{
    return m_dimension;
}

TextureExtension Texture::GetExtension( ) const
{
    return m_extension;
}

const InteropArray<Byte> &Texture::GetData( ) const
{
    return m_data;
}

InteropArray<TextureMip> Texture::ReadMipData( ) const
{
    InteropArray<TextureMip> mipData;
    StreamMipData( [ & ]( const TextureMip &mip ) { mipData.AddElement( mip ); } );
    return mipData;
}

void Texture::StreamMipDataDDS( const MipStreamCallback &callback ) const
{
    for ( uint32_t array = 0; array < m_arraySize; ++array )
    {
        for ( uint32_t mip = 0; mip < m_mipLevels; ++mip )
        {
            // this.Data already skips the data_offset() but mip_offset() includes it
            const auto externalOffset = m_ddsHeader->mip_offset( mip, array ) - m_ddsHeader->data_offset( );

            TextureMip mipData{ };
            mipData.Width      = m_ddsHeader->width( ) >> mip;
            mipData.Height     = m_ddsHeader->height( ) >> mip;
            mipData.MipIndex   = mip;
            mipData.ArrayIndex = array;
            mipData.RowPitch   = m_ddsHeader->row_pitch( mip );
            mipData.NumRows    = m_numRows >> mip;
            mipData.SlicePitch = m_ddsHeader->slice_pitch( mip );
            mipData.DataOffset = externalOffset;

            callback( mipData );
        }
    }
}

void Texture::StreamMipDataSTB( const MipStreamCallback &callback ) const
{
    TextureMip mipData{ };
    mipData.Width      = m_width;
    mipData.Height     = m_height;
    mipData.MipIndex   = 0;
    mipData.ArrayIndex = 0;
    mipData.RowPitch   = m_rowPitch;
    mipData.NumRows    = m_numRows;
    mipData.SlicePitch = m_slicePitch;
    mipData.DataOffset = 0;

    callback( mipData );
}

void Texture::LoadTextureFromMemory( const Byte *data, const size_t dataNumBytes )
{
    switch ( m_extension )
    {
    case TextureExtension::DDS:
        LoadTextureDDSFromMemory( data, dataNumBytes );
        break;
    default:
        LoadTextureSTBFromMemory( data, dataNumBytes );
        break;
    }
}

void Texture::LoadTextureDDSFromMemory( const Byte *data, const size_t dataNumBytes )
{
    if ( data == nullptr || dataNumBytes < sizeof( dds::Header ) )
    {
        LOG( WARNING ) << "Invalid DDS data provided";
        return;
    }

    const dds::Header header = dds::read_header( data, dataNumBytes );
    m_ddsHeader              = std::unique_ptr<dds::Header, DDSHeaderDeleter>( new dds::Header( header ) );
    if ( !m_ddsHeader->is_valid( ) )
    {
        LOG( WARNING ) << "Error loading texture from memory: Invalid DDS header";
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
    m_rowPitch     = std::max( 1U, ( m_width + ( m_blockSize - 1 ) ) / m_blockSize ) * m_bitsPerPixel >> 3;
    m_numRows      = std::max( 1U, ( m_height + ( m_blockSize - 1 ) ) / m_blockSize );
    m_slicePitch   = m_rowPitch * m_numRows;

    m_data.Resize( m_ddsHeader->data_size( ) );
    const uint8_t *srcData = data + m_ddsHeader->data_offset( );
    m_data.MemCpy( srcData, m_ddsHeader->data_size( ) );

    if ( m_ddsHeader->is_1d( ) )
    {
        m_dimension = TextureDimension::Texture1D;
    }
    else if ( m_ddsHeader->is_3d( ) )
    {
        m_dimension = TextureDimension::Texture3D;
    }
    else
    {
        m_dimension = TextureDimension::Texture2D;
    }
    if ( m_ddsHeader->is_cubemap( ) )
    {
        m_dimension = TextureDimension::TextureCube;
    }

    if ( IsFormatBC( m_format ) )
    {
        m_width  = Utilities::Align( m_width, FormatBlockSize( m_format ) );
        m_height = Utilities::Align( m_height, FormatBlockSize( m_format ) );
    }
}

void Texture::LoadTextureSTBFromMemory( const Byte *data, const size_t dataNumBytes )
{
    int width, height, channels;

    stbi_uc *contents = stbi_load_from_memory( static_cast<const stbi_uc *>( data ), static_cast<int>( dataNumBytes ), &width, &height, &channels, STBI_rgb_alpha );

    if ( contents == nullptr )
    {
        LOG( WARNING ) << "Error loading texture from memory with STB, reason:" << stbi_failure_reason( );
        return;
    }

    m_width        = static_cast<uint32_t>( std::max<int>( 1, width ) );
    m_height       = static_cast<uint32_t>( std::max<int>( 1, height ) );
    m_depth        = 1;
    m_format       = Format::R8G8B8A8Unorm;
    m_dimension    = TextureDimension::Texture2D;
    m_arraySize    = 1;
    m_mipLevels    = 1;
    m_bitsPerPixel = 32;
    m_blockSize    = 1;
    m_rowPitch     = m_width * 4;
    m_numRows      = m_height;
    m_slicePitch   = m_rowPitch * m_numRows;
    m_data.Resize( m_slicePitch );
    m_data.MemCpy( contents, m_slicePitch );

    stbi_image_free( contents );
}
