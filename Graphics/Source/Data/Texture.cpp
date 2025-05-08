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

#include <DenOfIzGraphics/Data/Texture.h>

#if not defined( STB_IMAGE_IMPLEMENTATION ) and defined( DZ_USE_STB_IMAGE )
// #define STB_IMAGE_IMPLEMENTATION
// #define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#endif

#include <DenOfIzGraphics/Utilities/Utilities.h>
#include <filesystem>
#include <fstream>

using namespace DenOfIz;

Texture::Texture( const InteropString &path ) : m_path( Utilities::AppPath( path.Get( ) ) )
{
    if ( !std::filesystem::exists( m_path ) )
    {
        LOG( ERROR ) << "Texture file does not exist: " << m_path;
        return;
    }

    if ( const std::filesystem::path &extension = std::filesystem::path( m_path ).extension( ); extension == ".dds" )
    {
        Extension = TextureExtension::DDS;
    }
    else if ( extension == ".png" )
    {
        Extension = TextureExtension::PNG;
    }
    else if ( extension == ".jpg" )
    {
        Extension = TextureExtension::JPG;
    }
    else if ( extension == ".bmp" )
    {
        Extension = TextureExtension::BMP;
    }
    else if ( extension == ".tga" )
    {
        Extension = TextureExtension::TGA;
    }
    else if ( extension == ".hdr" )
    {
        Extension = TextureExtension::HDR;
    }
    else if ( extension == ".gif" )
    {
        Extension = TextureExtension::GIF;
    }
    else if ( extension == ".pic" )
    {
        Extension = TextureExtension::PIC;
    }

    switch ( Extension )
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
    Extension = extension;
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

    stbi_uc *contents = stbi_load( m_path.c_str( ), &width, &height, &channels, STBI_rgb_alpha );

    if ( contents == nullptr )
    {
        LOG( WARNING ) << "Error loading texture: " << m_path << ", reason:" << stbi_failure_reason( );
        return;
    }
    Width        = static_cast<uint32_t>( std::max<int>( 1, width ) );
    Height       = static_cast<uint32_t>( std::max<int>( 1, height ) );
    Depth        = 1;
    Format       = Format::R8G8B8A8Unorm;
    Dimension    = TextureDimension::Texture2D;
    ArraySize    = 1;
    MipLevels    = 1;
    BitsPerPixel = 32;
    BlockSize    = 1;
    RowPitch     = Width * 4;
    NumRows      = Height;
    SlicePitch   = RowPitch * NumRows;
    Data.Resize( SlicePitch );
    Data.MemCpy( contents, SlicePitch );
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

    m_ddsHeader = dds::read_header( fileData, size );
    if ( !m_ddsHeader.is_valid( ) )
    {
        LOG( WARNING ) << "Error loading texture: " << m_path << ", reason: Invalid DDS header";
        return;
    }

    ArraySize    = 1;
    Width        = m_ddsHeader.width( );
    Height       = m_ddsHeader.height( );
    Depth        = m_ddsHeader.depth( );
    MipLevels    = m_ddsHeader.mip_levels( );
    ArraySize    = m_ddsHeader.array_size( );
    Format       = GetFormatFromDDS( m_ddsHeader.format( ) );
    BitsPerPixel = m_ddsHeader.bits_per_element( );
    BlockSize    = m_ddsHeader.block_size( );
    RowPitch     = std::max( 1U, ( Width + ( BlockSize - 1 ) ) / BlockSize ) * BitsPerPixel >> 3;
    NumRows      = std::max( 1U, ( Height + ( BlockSize - 1 ) ) / BlockSize );
    SlicePitch   = RowPitch * NumRows;

    Data.Resize( m_ddsHeader.data_size( ) );
    Data.MemCpy( fileData + m_ddsHeader.data_offset( ), m_ddsHeader.data_size( ) );

    if ( m_ddsHeader.is_1d( ) )
    {
        Dimension = TextureDimension::Texture1D;
    }
    else if ( m_ddsHeader.is_3d( ) )
    {
        Dimension = TextureDimension::Texture3D;
    }
    else
    {
        Dimension = TextureDimension::Texture2D;
    }
    if ( m_ddsHeader.is_cubemap( ) )
    {
        Dimension = TextureDimension::TextureCube;
    }

    if ( IsFormatBC( Format ) )
    {
        Width  = Utilities::Align( Width, FormatBlockSize( Format ) );
        Height = Utilities::Align( Height, FormatBlockSize( Format ) );
    }
}

Format Texture::GetFormatFromDDS( const dds::DXGI_FORMAT &format )
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

void Texture::StreamMipData( const MipStreamCallback &callback ) const
{
    switch ( Extension )
    {
    case TextureExtension::DDS:
        StreamMipDataDDS( callback );
        break;
    default:
        StreamMipDataSTB( callback );
        break;
    }
}

InteropArray<TextureMip> Texture::ReadMipData( ) const
{
    InteropArray<TextureMip> mipData;
    StreamMipData( [ & ]( const TextureMip &mip ) { mipData.AddElement( mip ); } );
    return mipData;
}

void Texture::StreamMipDataDDS( const MipStreamCallback &callback ) const
{
    for ( uint32_t array = 0; array < ArraySize; ++array )
    {
        for ( uint32_t mip = 0; mip < MipLevels; ++mip )
        {
            // this.Data already skips the data_offset() but mip_offset() includes it
            const auto externalOffset = m_ddsHeader.mip_offset( mip, array ) - m_ddsHeader.data_offset( );

            TextureMip mipData{ };
            mipData.Width      = m_ddsHeader.width( ) >> mip;
            mipData.Height     = m_ddsHeader.height( ) >> mip;
            mipData.MipIndex   = mip;
            mipData.ArrayIndex = array;
            mipData.RowPitch   = m_ddsHeader.row_pitch( mip );
            mipData.NumRows    = NumRows >> mip;
            mipData.SlicePitch = m_ddsHeader.slice_pitch( mip );
            mipData.DataOffset = externalOffset;

            callback( mipData );
        }
    }
}

void Texture::StreamMipDataSTB( const MipStreamCallback &callback ) const
{
    TextureMip mipData{ };
    mipData.Width      = Width;
    mipData.Height     = Height;
    mipData.MipIndex   = 0;
    mipData.ArrayIndex = 0;
    mipData.RowPitch   = RowPitch;
    mipData.NumRows    = NumRows;
    mipData.SlicePitch = SlicePitch;
    mipData.DataOffset = 0;

    callback( mipData );
}

void Texture::LoadTextureFromMemory( const Byte *data, const size_t dataNumBytes )
{
    switch ( Extension )
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

    m_ddsHeader = dds::read_header( data, dataNumBytes );
    if ( !m_ddsHeader.is_valid( ) )
    {
        LOG( WARNING ) << "Error loading texture from memory: Invalid DDS header";
        return;
    }

    ArraySize    = 1;
    Width        = m_ddsHeader.width( );
    Height       = m_ddsHeader.height( );
    Depth        = m_ddsHeader.depth( );
    MipLevels    = m_ddsHeader.mip_levels( );
    ArraySize    = m_ddsHeader.array_size( );
    Format       = GetFormatFromDDS( m_ddsHeader.format( ) );
    BitsPerPixel = m_ddsHeader.bits_per_element( );
    BlockSize    = m_ddsHeader.block_size( );
    RowPitch     = std::max( 1U, ( Width + ( BlockSize - 1 ) ) / BlockSize ) * BitsPerPixel >> 3;
    NumRows      = std::max( 1U, ( Height + ( BlockSize - 1 ) ) / BlockSize );
    SlicePitch   = RowPitch * NumRows;

    Data.Resize( m_ddsHeader.data_size( ) );
    const uint8_t *srcData = data + m_ddsHeader.data_offset( );
    Data.MemCpy( srcData, m_ddsHeader.data_size( ) );

    if ( m_ddsHeader.is_1d( ) )
    {
        Dimension = TextureDimension::Texture1D;
    }
    else if ( m_ddsHeader.is_3d( ) )
    {
        Dimension = TextureDimension::Texture3D;
    }
    else
    {
        Dimension = TextureDimension::Texture2D;
    }
    if ( m_ddsHeader.is_cubemap( ) )
    {
        Dimension = TextureDimension::TextureCube;
    }

    if ( IsFormatBC( Format ) )
    {
        Width  = Utilities::Align( Width, FormatBlockSize( Format ) );
        Height = Utilities::Align( Height, FormatBlockSize( Format ) );
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

    Width        = static_cast<uint32_t>( std::max<int>( 1, width ) );
    Height       = static_cast<uint32_t>( std::max<int>( 1, height ) );
    Depth        = 1;
    Format       = Format::R8G8B8A8Unorm;
    Dimension    = TextureDimension::Texture2D;
    ArraySize    = 1;
    MipLevels    = 1;
    BitsPerPixel = 32;
    BlockSize    = 1;
    RowPitch     = Width * 4;
    NumRows      = Height;
    SlicePitch   = RowPitch * NumRows;
    Data.Resize( SlicePitch );
    Data.MemCpy( contents, SlicePitch );

    stbi_image_free( contents );
}
