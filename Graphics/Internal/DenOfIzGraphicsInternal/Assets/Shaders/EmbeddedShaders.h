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
        static DenOfIz_ShaderProgram GetUIArrayShaderProgram( );
        static DenOfIz_ShaderProgram GetUISingleShaderProgram( );
    };
} // namespace DenOfIz
