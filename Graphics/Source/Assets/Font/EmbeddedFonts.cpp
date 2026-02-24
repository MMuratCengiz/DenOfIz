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

#include "DenOfIzGraphicsInternal/Assets/Font/EmbeddedFonts.h"
#include <memory>
#include <miniz/miniz.h>
#include <mutex>
#include <vector>
#include "DenOfIzGraphics/Assets/Serde/Font/FontAssetReader.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryReader.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

extern const uint8_t g_JetbrainsMonoFontCompressed[];
extern const size_t  g_JetbrainsMonoFontCompressedSize;

std::unordered_map<std::string, EmbeddedFonts::FontData> &EmbeddedFonts::GetFontRegistry( )
{
    static std::unordered_map<std::string, FontData> fontRegistry = []( )
    {
        std::unordered_map<std::string, FontData> registry;
        registry[ "JetbrainsMono" ] = FontData{ g_JetbrainsMonoFontCompressed, g_JetbrainsMonoFontCompressedSize };
        return registry;
    }( );
    return fontRegistry;
}

std::unordered_map<std::string, EmbeddedFonts::DecompressedFontData> &EmbeddedFonts::GetDecompressedFontCache( )
{
    static std::unordered_map<std::string, DecompressedFontData> cache;
    return cache;
}

std::mutex &EmbeddedFonts::GetDecompressionMutex( )
{
    static std::mutex mutex;
    return mutex;
}

const std::vector<Byte> &EmbeddedFonts::GetFontData( const std::string &fontName )
{
    auto      &registry = GetFontRegistry( );
    const auto it       = registry.find( fontName );

    if ( it == registry.end( ) )
    {
        spdlog::error( "Font '{}' not found in embedded fonts registry", fontName );
        static std::vector<Byte> emptyData;
        return emptyData;
    }

    const auto &fontData = it->second;
    auto       &cache    = GetDecompressedFontCache( );
    auto       &mutex    = GetDecompressionMutex( );

    {
        std::lock_guard lock( mutex );
        const auto      cacheIt = cache.find( fontName );
        if ( cacheIt != cache.end( ) && cacheIt->second.isDecompressed )
        {
            return cacheIt->second.data;
        }
    }

    std::lock_guard lock( mutex );
    auto           &decompressedData = cache[ fontName ];

    if ( !decompressedData.isDecompressed )
    {
        uint32_t uncompressedSize = 0;
        memcpy( &uncompressedSize, fontData.compressedData, sizeof( uint32_t ) );

        std::vector<Byte> decompressedBuffer( uncompressedSize );
        mz_ulong          destLen = uncompressedSize;
        const int result = mz_uncompress( decompressedBuffer.data( ), &destLen, fontData.compressedData + sizeof( uint32_t ), fontData.compressedSize - sizeof( uint32_t ) );

        if ( result != MZ_OK || destLen != uncompressedSize )
        {
            spdlog::error( "Failed to decompress embedded font data for font '{}'", fontName );
            return decompressedData.data;
        }

        decompressedData.data.resize( uncompressedSize );
        std::memcpy( decompressedData.data.data( ), decompressedBuffer.data( ), uncompressedSize );
        decompressedData.isDecompressed = true;
    }
    return decompressedData.data;
}

DenOfIz_FontAsset EmbeddedFonts::CreateFontAsset( const std::string &fontName )
{
    const auto &fontData = GetFontData( fontName );

    if ( fontData.empty( ) )
    {
        return DENOFIZ_NULL_HANDLE;
    }

    DenOfIz_ByteArrayView data{ };
    data.Elements    = fontData.data( );
    data.NumElements = fontData.size( );
    DenOfIz_BinaryReaderDesc binaryReaderDesc{ 0 };
    DenOfIz_BinaryReader     binaryReader = DenOfIz_BinaryReader_CreateFromData( data, &binaryReaderDesc );

    DenOfIz_FontAssetReaderDesc readerDesc{ };
    readerDesc.Reader              = binaryReader;
    DenOfIz_FontAssetReader reader = DenOfIz_FontAssetReader_Create( &readerDesc );
    DenOfIz_FontAsset       asset  = DenOfIz_FontAssetReader_Read( reader );
    DenOfIz_FontAssetReader_Destroy( reader );
    return asset;
}

DenOfIz_FontAsset EmbeddedFonts::GetEmbeddedFont( const std::string &fontName )
{
    static std::unordered_map<std::string, DenOfIz_FontAsset> fontCache;
    static std::mutex                                         cacheMutex;

    {
        std::lock_guard lock( cacheMutex );
        const auto      it = fontCache.find( fontName );
        if ( it != fontCache.end( ) )
        {
            return it->second;
        }
    }

    DenOfIz_FontAsset fontAsset = CreateFontAsset( fontName );
    if ( !DENOFIZ_HANDLE_IS_VALID( fontAsset ) )
    {
        spdlog::error( "Failed to create font asset for font '{}'", fontName );
        return DENOFIZ_NULL_HANDLE;
    }

    {
        std::lock_guard lock( cacheMutex );
        const auto      it = fontCache.find( fontName );
        if ( it != fontCache.end( ) )
        {
            return it->second;
        }
        fontCache[ fontName ] = fontAsset;
    }
    return fontAsset;
}

DenOfIz_FontAsset EmbeddedFonts::GetJetbrainsMono( )
{
    return GetEmbeddedFont( "JetbrainsMono" );
}

#include "DenOfIzGraphicsInternal/Assets/Font/Embedded/JetbrainsMonoCompressed.inl"
