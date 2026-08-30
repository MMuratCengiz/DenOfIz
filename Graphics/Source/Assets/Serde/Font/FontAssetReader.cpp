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

#include "DenOfIzGraphics/Assets/Serde/Font/FontAssetReader.h"
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"
#include "DenOfIzGraphicsInternal/Assets/Serde/Common/AssetReaderHelpers.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"
#include "DenOfIzGraphicsInternal/Utilities/Utilities.h"

#define FONT_ASSET_READER_IMPL( handle ) DENOFIZ_FROM_HANDLE( DenOfIz::FontAssetReader, handle )

namespace DenOfIz
{
    class FontAssetReader
    {
    public:
        DenOfIz_BinaryReader m_reader;
        DenOfIz_FontAsset    m_fontAsset         = DENOFIZ_NULL_HANDLE;
        bool                 m_assetRead         = false;
        uint64_t             m_streamStartOffset = 0;

        explicit FontAssetReader( const DenOfIz_FontAssetReaderDesc &desc );
        ~FontAssetReader( );

        DenOfIz_FontAsset Read( );
    };
} // namespace DenOfIz

using namespace DenOfIz;

FontAssetReader::FontAssetReader( const DenOfIz_FontAssetReaderDesc &desc ) : m_reader( desc.Reader )
{
    DZ_NOT_NULL( m_reader );
}

FontAssetReader::~FontAssetReader( ) = default;

DenOfIz_FontAsset FontAssetReader::Read( )
{
    if ( m_assetRead )
    {
        return m_fontAsset;
    }
    m_fontAsset         = DenOfIz_FontAsset_Create( );
    m_streamStartOffset = DenOfIz_BinaryReader_Position( m_reader );

    const uint64_t magic = DenOfIz_BinaryReader_ReadUInt64( m_reader );
    if ( magic != DenOfIz_FontAsset_Magic( m_fontAsset ) )
    {
        spdlog::error( "Invalid font asset magic word" );
        return m_fontAsset;
    }

    const uint32_t version = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    if ( version > 2 )
    {
        spdlog::warn( "FontAsset version mismatch" );
    }

    DenOfIz_FontAsset_SetVersion( m_fontAsset, version );
    DenOfIz_FontAsset_SetNumBytes( m_fontAsset, DenOfIz_BinaryReader_ReadUInt64( m_reader ) );
    DenOfIz_FontAsset_SetPath( m_fontAsset, DenOfIz_StringView( DenOfIz_BinaryReader_ReadString( m_reader ) ) );

    const uint64_t          dataNumBytes = DenOfIz_BinaryReader_ReadUInt64( m_reader );
    const DenOfIz_ByteArray data         = DenOfIz_BinaryReader_ReadBytes( m_reader, dataNumBytes );
    DenOfIz_FontAsset_SetData( m_fontAsset, data.Elements, data.NumElements );

    DenOfIz_FontAsset_SetInitialFontSize( m_fontAsset, DenOfIz_BinaryReader_ReadUInt32( m_reader ) );
    DenOfIz_FontAsset_SetAtlasWidth( m_fontAsset, DenOfIz_BinaryReader_ReadUInt32( m_reader ) );
    DenOfIz_FontAsset_SetAtlasHeight( m_fontAsset, DenOfIz_BinaryReader_ReadUInt32( m_reader ) );

    DenOfIz_FontMetrics metrics;
    metrics.Ascent             = DenOfIz_BinaryReader_ReadFloat( m_reader );
    metrics.Descent            = DenOfIz_BinaryReader_ReadFloat( m_reader );
    metrics.LineGap            = DenOfIz_BinaryReader_ReadFloat( m_reader );
    metrics.LineHeight         = DenOfIz_BinaryReader_ReadFloat( m_reader );
    metrics.UnderlinePos       = DenOfIz_BinaryReader_ReadFloat( m_reader );
    metrics.UnderlineThickness = DenOfIz_BinaryReader_ReadFloat( m_reader );
    DenOfIz_FontAsset_SetMetrics( m_fontAsset, &metrics );

    const uint32_t numGlyphs = DenOfIz_BinaryReader_ReadUInt32( m_reader );
    DenOfIz_FontAsset_ReserveGlyphs( m_fontAsset, numGlyphs );

    for ( uint32_t i = 0; i < numGlyphs; ++i )
    {
        DenOfIz_FontGlyph glyph;
        glyph.CodePoint   = DenOfIz_BinaryReader_ReadUInt32( m_reader );
        glyph.Bounds.XMin = DenOfIz_BinaryReader_ReadDouble( m_reader );
        glyph.Bounds.YMin = DenOfIz_BinaryReader_ReadDouble( m_reader );
        glyph.Bounds.XMax = DenOfIz_BinaryReader_ReadDouble( m_reader );
        glyph.Bounds.YMax = DenOfIz_BinaryReader_ReadDouble( m_reader );
        glyph.Width       = DenOfIz_BinaryReader_ReadUInt32( m_reader );
        glyph.Height      = DenOfIz_BinaryReader_ReadUInt32( m_reader );
        glyph.BearingX    = DenOfIz_BinaryReader_ReadFloat( m_reader );
        glyph.BearingY    = DenOfIz_BinaryReader_ReadFloat( m_reader );
        glyph.XAdvance    = DenOfIz_BinaryReader_ReadFloat( m_reader );
        glyph.YAdvance    = DenOfIz_BinaryReader_ReadFloat( m_reader );
        glyph.AtlasX      = DenOfIz_BinaryReader_ReadUInt32( m_reader );
        glyph.AtlasY      = DenOfIz_BinaryReader_ReadUInt32( m_reader );
        DenOfIz_FontAsset_AddGlyph( m_fontAsset, &glyph );
    }

    const uint64_t          atlasDataNumBytes = DenOfIz_BinaryReader_ReadUInt64( m_reader );
    const DenOfIz_ByteArray atlasData         = DenOfIz_BinaryReader_ReadBytes( m_reader, atlasDataNumBytes );
    DenOfIz_FontAsset_SetAtlasData( m_fontAsset, atlasData.Elements, atlasData.NumElements );

    m_assetRead = true;
    return m_fontAsset;
}

extern "C"
{

    DenOfIz_FontAssetReader DenOfIz_FontAssetReader_Create( const DenOfIz_FontAssetReaderDesc *desc )
    {
        if ( desc == NULL )
        {
            return DENOFIZ_NULL_HANDLE;
        }

        auto *reader = new FontAssetReader( *desc );
        return DENOFIZ_TO_HANDLE( reader );
    }

    void DenOfIz_FontAssetReader_Destroy( DenOfIz_FontAssetReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return;
        }
        delete FONT_ASSET_READER_IMPL( reader );
    }

    DenOfIz_FontAsset DenOfIz_FontAssetReader_Read( DenOfIz_FontAssetReader reader )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( reader ) )
        {
            return DENOFIZ_NULL_HANDLE;
        }
        return FONT_ASSET_READER_IMPL( reader )->Read( );
    }

    void DenOfIz_FontAssetReader_LoadAtlasIntoGpuTexture( DenOfIz_FontAsset fontAsset, const DenOfIz_LoadAtlasIntoGpuTextureDesc *desc )
    {
        if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) || desc == NULL )
        {
            return;
        }

        if ( !DENOFIZ_HANDLE_IS_VALID( desc->CommandList ) || !DENOFIZ_HANDLE_IS_VALID( desc->Texture ) )
        {
            spdlog::critical( "CommandList and Texture are required for LoadIntoGpuTexture" );
            return;
        }

        const auto stagingBuffer = desc->StagingBuffer;
        Byte      *mappedMemory;
        DenOfIz_Buffer_MapMemory( desc->StagingBuffer, (void **)&mappedMemory );
        const uint32_t         atlasWidth  = DenOfIz_FontAsset_AtlasWidth( fontAsset );
        const uint32_t         atlasHeight = DenOfIz_FontAsset_AtlasHeight( fontAsset );
        const uint32_t         rowPitch    = atlasWidth * DENOFIZ_FONT_ASSET_NUM_CHANNELS;
        DenOfIz_PhysicalDevice deviceInfo;
        DenOfIz_LogicalDevice_DeviceInfo( desc->Device, &deviceInfo );
        const uint32_t alignedRowPitch = Utilities::Align( atlasWidth * DENOFIZ_FONT_ASSET_NUM_CHANNELS, deviceInfo.Constants.BufferTextureRowAlignment );

        const DenOfIz_ByteArray atlasData = DenOfIz_FontAsset_AtlasData( fontAsset );
        const Byte             *pSrcData  = atlasData.Elements;
        for ( uint32_t y = 0; y < atlasHeight; ++y )
        {
            memcpy( mappedMemory + alignedRowPitch * y, pSrcData + rowPitch * y, rowPitch );
        }
        DenOfIz_Buffer_UnmapMemory( stagingBuffer );

        DenOfIz_CopyBufferToTextureDesc copyDesc{ };
        copyDesc.DstTexture = desc->Texture;
        copyDesc.SrcBuffer  = stagingBuffer;
        DenOfIz_TextureResource_GetFormat( desc->Texture, &copyDesc.Format );
        copyDesc.MipLevel   = 0;
        copyDesc.ArrayLayer = 0;
        copyDesc.RowPitch   = atlasWidth * DENOFIZ_FONT_ASSET_NUM_CHANNELS;
        copyDesc.NumRows    = atlasHeight;
        DenOfIz_CommandList_CopyBufferToTexture( desc->CommandList, &copyDesc );
    }
}
