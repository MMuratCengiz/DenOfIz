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

#include "EmbeddedImGuiShaders.h"

#include <cstring>
#include <miniz/miniz.h>
#include <mutex>
#include <spdlog/spdlog.h>
#include <vector>
#include "DenOfIzGraphics/Assets/Serde/Shader/ShaderAssetReader.h"
#include "DenOfIzGraphics/Assets/Stream/BinaryReader.h"

using namespace DenOfIz;

extern const uint8_t g_ImGuiArrayShadersCompressed[];
extern const size_t  g_ImGuiArrayShadersCompressedSize;
extern const uint8_t g_ImGuiSingleShadersCompressed[];
extern const size_t  g_ImGuiSingleShadersCompressedSize;

namespace
{
    DenOfIz_ShaderProgram LoadShaderFromCompressed( const uint8_t *compressedData, size_t compressedSize )
    {
        uint32_t uncompressedSize = 0;
        memcpy( &uncompressedSize, compressedData, sizeof( uint32_t ) );

        std::vector<Byte> decompressedBuffer( uncompressedSize );
        mz_ulong          destLen = uncompressedSize;
        const int         result  = mz_uncompress( decompressedBuffer.data( ), &destLen, compressedData + sizeof( uint32_t ), compressedSize - sizeof( uint32_t ) );

        if ( result != MZ_OK || destLen != uncompressedSize )
        {
            spdlog::error( "Failed to decompress ImGui shader data" );
            return DENOFIZ_NULL_HANDLE;
        }

        DenOfIz_ByteArrayView data{ };
        data.Elements    = decompressedBuffer.data( );
        data.NumElements = decompressedBuffer.size( );
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
} // namespace

DenOfIz_ShaderProgram EmbeddedImGuiShaders::GetImGuiArrayShaderProgram( )
{
    static DenOfIz_ShaderProgram cachedProgram = DENOFIZ_NULL_HANDLE;
    static std::mutex            cacheMutex;

    std::lock_guard lock( cacheMutex );
    if ( DENOFIZ_HANDLE_IS_VALID( cachedProgram ) )
    {
        return cachedProgram;
    }

    cachedProgram = LoadShaderFromCompressed( g_ImGuiArrayShadersCompressed, g_ImGuiArrayShadersCompressedSize );
    return cachedProgram;
}

DenOfIz_ShaderProgram EmbeddedImGuiShaders::GetImGuiSingleShaderProgram( )
{
    static DenOfIz_ShaderProgram cachedProgram = DENOFIZ_NULL_HANDLE;
    static std::mutex            cacheMutex;

    std::lock_guard lock( cacheMutex );
    if ( DENOFIZ_HANDLE_IS_VALID( cachedProgram ) )
    {
        return cachedProgram;
    }

    cachedProgram = LoadShaderFromCompressed( g_ImGuiSingleShadersCompressed, g_ImGuiSingleShadersCompressedSize );
    return cachedProgram;
}

#include "Embedded/ImGuiArrayShadersCompressed.inl"
#include "Embedded/ImGuiSingleShadersCompressed.inl"
