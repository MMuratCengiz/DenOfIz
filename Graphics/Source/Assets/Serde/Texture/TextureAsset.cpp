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

#include "DenOfIzGraphics/Assets/Serde/Texture/TextureAsset.h"

#include <string>
#include <vector>

#define TEXTURE_ASSET_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::TextureAsset, handle )

namespace DenOfIz
{
    class TextureAsset
    {
    public:
        std::string                     m_name;
        std::string                     m_sourcePath;
        std::string                     m_path;
        std::vector<DenOfIz_TextureMip> m_mips;

        uint64_t                 m_magic        = 0x445A544558;
        uint32_t                 m_version      = 1;
        uint64_t                 m_numBytes     = 0;
        uint32_t                 m_width        = 0;
        uint32_t                 m_height       = 0;
        uint32_t                 m_depth        = 1;
        DenOfIz_Format           m_format       = DENOFIZ_FORMAT_UNDEFINED;
        DenOfIz_TextureDimension m_dimension    = DENOFIZ_TEXTURE_DIMENSION_TEXTURE2D;
        uint32_t                 m_mipLevels    = 1;
        uint32_t                 m_arraySize    = 1;
        uint32_t                 m_bitsPerPixel = 0;
        uint32_t                 m_blockSize    = 1;
        uint32_t                 m_rowPitch     = 0;
        uint32_t                 m_numRows      = 0;
        uint32_t                 m_slicePitch   = 0;
        DenOfIz_AssetDataStream  m_data         = { };

        static constexpr uint32_t Latest = 1;

        TextureAsset( )  = default;
        ~TextureAsset( ) = default;

        uint64_t Magic( ) const
        {
            return m_magic;
        }

        uint32_t Version( ) const
        {
            return m_version;
        }

        uint64_t NumBytes( ) const
        {
            return m_numBytes;
        }

        DenOfIz_StringView Path( ) const
        {
            return DenOfIz_StringView{ m_path.c_str( ), static_cast<uint32_t>( m_path.size( ) ) };
        }

        DenOfIz_StringView Name( ) const
        {
            return DenOfIz_StringView{ m_name.c_str( ), static_cast<uint32_t>( m_name.size( ) ) };
        }

        DenOfIz_StringView SourcePath( ) const
        {
            return DenOfIz_StringView{ m_sourcePath.c_str( ), static_cast<uint32_t>( m_sourcePath.size( ) ) };
        }

        uint32_t Width( ) const
        {
            return m_width;
        }

        uint32_t Height( ) const
        {
            return m_height;
        }

        uint32_t Depth( ) const
        {
            return m_depth;
        }

        DenOfIz_Format GetFormat( ) const
        {
            return m_format;
        }

        DenOfIz_TextureDimension GetDimension( ) const
        {
            return m_dimension;
        }

        uint32_t MipLevels( ) const
        {
            return m_mipLevels;
        }

        uint32_t ArraySize( ) const
        {
            return m_arraySize;
        }

        uint32_t BitsPerPixel( ) const
        {
            return m_bitsPerPixel;
        }

        uint32_t BlockSize( ) const
        {
            return m_blockSize;
        }

        uint32_t RowPitch( ) const
        {
            return m_rowPitch;
        }

        uint32_t NumRows( ) const
        {
            return m_numRows;
        }

        uint32_t SlicePitch( ) const
        {
            return m_slicePitch;
        }

        DenOfIz_TextureMipArray Mips( ) const
        {
            return DenOfIz_TextureMipArray{ const_cast<DenOfIz_TextureMip *>( m_mips.data( ) ), m_mips.size( ) };
        }

        size_t NumMips( ) const
        {
            return m_mips.size( );
        }

        DenOfIz_AssetDataStream Data( ) const
        {
            return m_data;
        }

        void SetVersion( const uint32_t version )
        {
            m_version = version;
        }

        void SetNumBytes( const uint64_t numBytes )
        {
            m_numBytes = numBytes;
        }

        void SetPath( const DenOfIz_StringView path )
        {
            m_path = std::string( path.Chars, path.NumChars );
        }

        void SetName( const DenOfIz_StringView name )
        {
            m_name = std::string( name.Chars, name.NumChars );
        }

        void SetSourcePath( const DenOfIz_StringView sourcePath )
        {
            m_sourcePath = std::string( sourcePath.Chars, sourcePath.NumChars );
        }

        void SetWidth( const uint32_t width )
        {
            m_width = width;
        }

        void SetHeight( const uint32_t height )
        {
            m_height = height;
        }

        void SetDepth( const uint32_t depth )
        {
            m_depth = depth;
        }

        void SetFormat( const DenOfIz_Format format )
        {
            m_format = format;
        }

        void SetDimension( const DenOfIz_TextureDimension dimension )
        {
            m_dimension = dimension;
        }

        void SetMipLevels( const uint32_t mipLevels )
        {
            m_mipLevels = mipLevels;
        }

        void SetArraySize( const uint32_t arraySize )
        {
            m_arraySize = arraySize;
        }

        void SetBitsPerPixel( const uint32_t bitsPerPixel )
        {
            m_bitsPerPixel = bitsPerPixel;
        }

        void SetBlockSize( const uint32_t blockSize )
        {
            m_blockSize = blockSize;
        }

        void SetRowPitch( const uint32_t rowPitch )
        {
            m_rowPitch = rowPitch;
        }

        void SetNumRows( const uint32_t numRows )
        {
            m_numRows = numRows;
        }

        void SetSlicePitch( const uint32_t slicePitch )
        {
            m_slicePitch = slicePitch;
        }

        void SetData( const DenOfIz_AssetDataStream data )
        {
            m_data = data;
        }

        void AddMip( const DenOfIz_TextureMip &mip )
        {
            m_mips.push_back( mip );
        }

        void SetMips( const DenOfIz_TextureMip *mips, const size_t count )
        {
            m_mips.assign( mips, mips + count );
        }

        void ReserveMips( const size_t capacity )
        {
            m_mips.reserve( capacity );
        }

        void ClearMips( )
        {
            m_mips.clear( );
        }

        static DenOfIz_StringView Extension( )
        {
            return DENOFIZ_STRING( "dztex" );
        }
    };
} // namespace DenOfIz

extern "C"
{

    DenOfIz_TextureAsset DenOfIz_TextureAsset_Create( )
    {
        auto *textureAsset = new DenOfIz::TextureAsset( );
        return DENOFIZ_TO_HANDLE( textureAsset );
    }

    void DenOfIz_TextureAsset_Destroy( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        delete TEXTURE_ASSET_IMPL( textureAsset );
    }

    uint64_t DenOfIz_TextureAsset_Magic( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return 0;
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->Magic( );
    }

    uint32_t DenOfIz_TextureAsset_Version( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return 0;
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->Version( );
    }

    uint64_t DenOfIz_TextureAsset_NumBytes( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return 0;
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->NumBytes( );
    }

    DenOfIz_StringView DenOfIz_TextureAsset_Path( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return { NULL, 0 };
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->Path( );
    }

    DenOfIz_StringView DenOfIz_TextureAsset_Name( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return { NULL, 0 };
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->Name( );
    }

    DenOfIz_StringView DenOfIz_TextureAsset_SourcePath( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return { NULL, 0 };
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->SourcePath( );
    }

    uint32_t DenOfIz_TextureAsset_Width( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return 0;
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->Width( );
    }

    uint32_t DenOfIz_TextureAsset_Height( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return 0;
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->Height( );
    }

    uint32_t DenOfIz_TextureAsset_Depth( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return 0;
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->Depth( );
    }

    DenOfIz_Format DenOfIz_TextureAsset_GetFormat( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return DENOFIZ_FORMAT_UNDEFINED;
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->GetFormat( );
    }

    DenOfIz_TextureDimension DenOfIz_TextureAsset_GetDimension( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return DENOFIZ_TEXTURE_DIMENSION_UNDEFINED;
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->GetDimension( );
    }

    uint32_t DenOfIz_TextureAsset_MipLevels( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return 0;
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->MipLevels( );
    }

    uint32_t DenOfIz_TextureAsset_ArraySize( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return 0;
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->ArraySize( );
    }

    uint32_t DenOfIz_TextureAsset_BitsPerPixel( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return 0;
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->BitsPerPixel( );
    }

    uint32_t DenOfIz_TextureAsset_BlockSize( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return 0;
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->BlockSize( );
    }

    uint32_t DenOfIz_TextureAsset_RowPitch( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return 0;
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->RowPitch( );
    }

    uint32_t DenOfIz_TextureAsset_NumRows( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return 0;
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->NumRows( );
    }

    uint32_t DenOfIz_TextureAsset_SlicePitch( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return 0;
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->SlicePitch( );
    }

    DenOfIz_TextureMipArray DenOfIz_TextureAsset_Mips( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return { NULL, 0 };
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->Mips( );
    }

    size_t DenOfIz_TextureAsset_NumMips( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return 0;
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->NumMips( );
    }

    DenOfIz_AssetDataStream DenOfIz_TextureAsset_Data( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return { 0, 0 };
        }
        return TEXTURE_ASSET_IMPL( textureAsset )->Data( );
    }

    void DenOfIz_TextureAsset_SetVersion( DenOfIz_TextureAsset textureAsset, uint32_t version )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->SetVersion( version );
    }

    void DenOfIz_TextureAsset_SetNumBytes( DenOfIz_TextureAsset textureAsset, uint64_t numBytes )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->SetNumBytes( numBytes );
    }

    void DenOfIz_TextureAsset_SetPath( DenOfIz_TextureAsset textureAsset, DenOfIz_StringView path )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->SetPath( path );
    }

    void DenOfIz_TextureAsset_SetName( DenOfIz_TextureAsset textureAsset, DenOfIz_StringView name )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->SetName( name );
    }

    void DenOfIz_TextureAsset_SetSourcePath( DenOfIz_TextureAsset textureAsset, DenOfIz_StringView sourcePath )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->SetSourcePath( sourcePath );
    }

    void DenOfIz_TextureAsset_SetWidth( DenOfIz_TextureAsset textureAsset, uint32_t width )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->SetWidth( width );
    }

    void DenOfIz_TextureAsset_SetHeight( DenOfIz_TextureAsset textureAsset, uint32_t height )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->SetHeight( height );
    }

    void DenOfIz_TextureAsset_SetDepth( DenOfIz_TextureAsset textureAsset, uint32_t depth )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->SetDepth( depth );
    }

    void DenOfIz_TextureAsset_SetFormat( DenOfIz_TextureAsset textureAsset, DenOfIz_Format format )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->SetFormat( format );
    }

    void DenOfIz_TextureAsset_SetDimension( DenOfIz_TextureAsset textureAsset, DenOfIz_TextureDimension dimension )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->SetDimension( dimension );
    }

    void DenOfIz_TextureAsset_SetMipLevels( DenOfIz_TextureAsset textureAsset, uint32_t mipLevels )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->SetMipLevels( mipLevels );
    }

    void DenOfIz_TextureAsset_SetArraySize( DenOfIz_TextureAsset textureAsset, uint32_t arraySize )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->SetArraySize( arraySize );
    }

    void DenOfIz_TextureAsset_SetBitsPerPixel( DenOfIz_TextureAsset textureAsset, uint32_t bitsPerPixel )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->SetBitsPerPixel( bitsPerPixel );
    }

    void DenOfIz_TextureAsset_SetBlockSize( DenOfIz_TextureAsset textureAsset, uint32_t blockSize )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->SetBlockSize( blockSize );
    }

    void DenOfIz_TextureAsset_SetRowPitch( DenOfIz_TextureAsset textureAsset, uint32_t rowPitch )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->SetRowPitch( rowPitch );
    }

    void DenOfIz_TextureAsset_SetNumRows( DenOfIz_TextureAsset textureAsset, uint32_t numRows )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->SetNumRows( numRows );
    }

    void DenOfIz_TextureAsset_SetSlicePitch( DenOfIz_TextureAsset textureAsset, uint32_t slicePitch )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->SetSlicePitch( slicePitch );
    }

    void DenOfIz_TextureAsset_SetData( DenOfIz_TextureAsset textureAsset, DenOfIz_AssetDataStream data )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->SetData( data );
    }

    void DenOfIz_TextureAsset_AddMip( DenOfIz_TextureAsset textureAsset, const DenOfIz_TextureMip *mip )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) || mip == NULL )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->AddMip( *mip );
    }

    void DenOfIz_TextureAsset_SetMips( DenOfIz_TextureAsset textureAsset, const DenOfIz_TextureMip *mips, size_t count )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) || ( mips == NULL && count > 0 ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->SetMips( mips, count );
    }

    void DenOfIz_TextureAsset_ReserveMips( DenOfIz_TextureAsset textureAsset, size_t capacity )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->ReserveMips( capacity );
    }

    void DenOfIz_TextureAsset_ClearMips( DenOfIz_TextureAsset textureAsset )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( textureAsset ) )
        {
            return;
        }
        TEXTURE_ASSET_IMPL( textureAsset )->ClearMips( );
    }

    DenOfIz_StringView DenOfIz_TextureAsset_Extension( )
    {
        return DenOfIz::TextureAsset::Extension( );
    }
}
