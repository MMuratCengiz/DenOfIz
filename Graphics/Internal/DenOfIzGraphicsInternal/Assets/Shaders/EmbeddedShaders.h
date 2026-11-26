/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

Licensed under the MIT License. See LICENSE in the project root.
*/

#pragma once

#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>
#include "DenOfIzGraphics/Backends/Common/ShaderProgram.h"

namespace DenOfIz
{
    class EmbeddedShaders
    {
    public:
        struct ShaderData
        {
            const uint8_t *compressedData;
            size_t         compressedSize;
        };

        struct DecompressedShaderData
        {
            std::vector<Byte> data;
            bool              isDecompressed = false;
        };

        static std::unordered_map<std::string, ShaderData>             &GetShaderRegistry( );
        static std::unordered_map<std::string, DecompressedShaderData> &GetDecompressedShaderCache( );
        static std::mutex                                              &GetDecompressionMutex( );
        static const std::vector<Byte>                                 &GetShaderData( const std::string &shaderName );
        static DenOfIz_ShaderProgram                                    CreateShaderProgram( const std::string &shaderName );
        static DenOfIz_ShaderProgram                                    GetEmbeddedShaderProgram( const std::string &shaderName );

        static DenOfIz_ShaderProgram GetTextRendererShaderProgram( );
        static DenOfIz_ShaderProgram GetUIShaderProgram( );
    };
} // namespace DenOfIz
