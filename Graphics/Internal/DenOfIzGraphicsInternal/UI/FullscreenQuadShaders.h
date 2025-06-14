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

#include "DenOfIzGraphics/Utilities/Interop.h"

namespace DenOfIz::EmbeddedFullscreenQuadShaders
{
    static auto FullscreenQuadVertexShaderSource = R"(
struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

VSOutput main(uint vertexID : SV_VertexID)
{
    VSOutput output;
    
    // Generate fullscreen triangle
    // vertexID 0: (-1, -1) -> (0, 1)
    // vertexID 1: (-1, 3) -> (0, -1)
    // vertexID 2: (3, -1) -> (2, 1)
    float2 uv = float2((vertexID << 1) & 2, vertexID & 2);
    output.Position = float4(uv * float2(2.0, -2.0) + float2(-1.0, 1.0), 0.0, 1.0);
    output.TexCoord = uv;
    
    return output;
})";

    static auto FullscreenQuadPixelShaderSource = R"(
struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
};

Texture2D SourceTexture : register(t0);
SamplerState LinearSampler : register(s0);

float4 main(PSInput input) : SV_TARGET
{
    return SourceTexture.Sample(LinearSampler, input.TexCoord);
})";
    static std::vector<Byte> StringToByteArray( const char *str )
    {
        const size_t       len = strlen( str );
        std::vector<Byte> result( len );
        for ( size_t i = 0; i < len; i++ )
        {
            result[ i ] = static_cast<Byte>( str[ i ] );
        }
        return result;
    }

    static std::vector<Byte> GetFullscreenQuadVertexShaderBytes( )
    {
        return StringToByteArray( FullscreenQuadVertexShaderSource );
    }

    static std::vector<Byte> GetFullscreenQuadPixelShaderBytes( )
    {
        return StringToByteArray( FullscreenQuadPixelShaderSource );
    }

} // namespace DenOfIz::EmbeddedFullscreenQuadShaders
