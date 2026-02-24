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

#include "DenOfIzGraphicsInternal/Assets/Shaders/EmbeddedShaders.h"
#include <memory>
#include <miniz/miniz.h>
#include <mutex>
#include <vector>
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetReader.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryReader.h"
#include "DenOfIzGraphicsInternal/Utilities/Logging.h"

using namespace DenOfIz;

extern const uint8_t g_TextRendererShadersCompressed[];
extern const size_t  g_TextRendererShadersCompressedSize;
extern const uint8_t g_UIArrayShadersCompressed[];
extern const size_t  g_UIArrayShadersCompressedSize;
extern const uint8_t g_UISingleShadersCompressed[];
extern const size_t  g_UISingleShadersCompressedSize;

std::unordered_map<std::string, EmbeddedShaders::ShaderData> &EmbeddedShaders::GetShaderRegistry( )
{
    static std::unordered_map<std::string, ShaderData> shaderRegistry = []( )
    {
        std::unordered_map<std::string, ShaderData> registry;
        registry[ "TextRenderer" ] = ShaderData{ g_TextRendererShadersCompressed, g_TextRendererShadersCompressedSize };
        registry[ "UIArray" ]      = ShaderData{ g_UIArrayShadersCompressed, g_UIArrayShadersCompressedSize };
        registry[ "UISingle" ]     = ShaderData{ g_UISingleShadersCompressed, g_UISingleShadersCompressedSize };
        return registry;
    }( );
    return shaderRegistry;
}

std::unordered_map<std::string, EmbeddedShaders::DecompressedShaderData> &EmbeddedShaders::GetDecompressedShaderCache( )
{
    static std::unordered_map<std::string, DecompressedShaderData> cache;
    return cache;
}

std::mutex &EmbeddedShaders::GetDecompressionMutex( )
{
    static std::mutex mutex;
    return mutex;
}

const std::vector<Byte> &EmbeddedShaders::GetShaderData( const std::string &shaderName )
{
    auto      &registry = GetShaderRegistry( );
    const auto it       = registry.find( shaderName );

    if ( it == registry.end( ) )
    {
        spdlog::error( "Shader '{}' not found in embedded shaders registry", shaderName );
        static std::vector<Byte> emptyData;
        return emptyData;
    }

    const auto &shaderData = it->second;
    auto       &cache      = GetDecompressedShaderCache( );
    auto       &mutex      = GetDecompressionMutex( );

    {
        std::lock_guard lock( mutex );
        const auto      cacheIt = cache.find( shaderName );
        if ( cacheIt != cache.end( ) && cacheIt->second.isDecompressed )
        {
            return cacheIt->second.data;
        }
    }

    std::lock_guard lock( mutex );
    auto           &decompressedData = cache[ shaderName ];

    if ( !decompressedData.isDecompressed )
    {
        uint32_t uncompressedSize = 0;
        memcpy( &uncompressedSize, shaderData.compressedData, sizeof( uint32_t ) );

        std::vector<Byte> decompressedBuffer( uncompressedSize );
        mz_ulong          destLen = uncompressedSize;
        const int result = mz_uncompress( decompressedBuffer.data( ), &destLen, shaderData.compressedData + sizeof( uint32_t ), shaderData.compressedSize - sizeof( uint32_t ) );

        if ( result != MZ_OK || destLen != uncompressedSize )
        {
            spdlog::error( "Failed to decompress embedded shader data for shader '{}'", shaderName );
            return decompressedData.data;
        }

        decompressedData.data.resize( uncompressedSize );
        std::memcpy( decompressedData.data.data( ), decompressedBuffer.data( ), uncompressedSize );
        decompressedData.isDecompressed = true;
    }
    return decompressedData.data;
}

DenOfIz_ShaderProgram EmbeddedShaders::CreateShaderProgram( const std::string &shaderName )
{
    const auto &shaderData = GetShaderData( shaderName );

    if ( shaderData.empty( ) )
    {
        return DENOFIZ_NULL_HANDLE;
    }

    DenOfIz_ByteArrayView data{ };
    data.Elements    = shaderData.data( );
    data.NumElements = shaderData.size( );
    DenOfIz_BinaryReaderDesc binaryReaderDesc{ 0 };
    DenOfIz_BinaryReader     binaryReader = DenOfIz_BinaryReader_CreateFromData( data, &binaryReaderDesc );

    DenOfIz_ShaderAssetReaderDesc readerDesc{ };
    readerDesc.Reader                 = binaryReader;
    DenOfIz_ShaderAssetReader reader  = DenOfIz_ShaderAssetReader_Create( &readerDesc );
    DenOfIz_ShaderProgram     program = DenOfIz_ShaderProgram_CreateFromAsset( reader );
    DenOfIz_ShaderAssetReader_Destroy( reader );
    DenOfIz_BinaryReader_Destroy( binaryReader );
    return program;
}

DenOfIz_ShaderProgram EmbeddedShaders::GetEmbeddedShaderProgram( const std::string &shaderName )
{
    static std::unordered_map<std::string, DenOfIz_ShaderProgram> shaderCache;
    static std::mutex                                             cacheMutex;

    {
        std::lock_guard lock( cacheMutex );
        const auto      it = shaderCache.find( shaderName );
        if ( it != shaderCache.end( ) )
        {
            return it->second;
        }
    }

    DenOfIz_ShaderProgram shaderProgram = CreateShaderProgram( shaderName );
    if ( !DENOFIZ_HANDLE_IS_VALID( shaderProgram ) )
    {
        spdlog::error( "Failed to create shader program for shader '{}'", shaderName );
        return DENOFIZ_NULL_HANDLE;
    }

    {
        std::lock_guard lock( cacheMutex );
        const auto      it = shaderCache.find( shaderName );
        if ( it != shaderCache.end( ) )
        {
            return it->second;
        }
        shaderCache[ shaderName ] = shaderProgram;
    }
    return shaderProgram;
}

DenOfIz_ShaderProgram EmbeddedShaders::GetTextRendererShaderProgram( )
{
    return GetEmbeddedShaderProgram( "TextRenderer" );
}

DenOfIz_ShaderProgram EmbeddedShaders::GetUIArrayShaderProgram( )
{
    return GetEmbeddedShaderProgram( "UIArray" );
}

DenOfIz_ShaderProgram EmbeddedShaders::GetUISingleShaderProgram( )
{
    return GetEmbeddedShaderProgram( "UISingle" );
}

#include "DenOfIzGraphicsInternal/Assets/Shaders/Embedded/TextRendererShadersCompressed.inl"
#include "DenOfIzGraphicsInternal/Assets/Shaders/Embedded/UIArrayShadersCompressed.inl"
#include "DenOfIzGraphicsInternal/Assets/Shaders/Embedded/UISingleShadersCompressed.inl"
