/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
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

extern const uint8_t g_ImGuiShadersCompressed[];
extern const size_t  g_ImGuiShadersCompressedSize;

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

DenOfIz_ShaderProgram EmbeddedImGuiShaders::GetImGuiShaderProgram( )
{
    static DenOfIz_ShaderProgram cachedProgram = DENOFIZ_NULL_HANDLE;
    static std::mutex            cacheMutex;

    std::lock_guard lock( cacheMutex );
    if ( DENOFIZ_HANDLE_IS_VALID( cachedProgram ) )
    {
        return cachedProgram;
    }

    cachedProgram = LoadShaderFromCompressed( g_ImGuiShadersCompressed, g_ImGuiShadersCompressedSize );
    return cachedProgram;
}

#include "Embedded/ImGuiShadersCompressed.inl"
