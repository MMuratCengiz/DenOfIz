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

#include <DenOfIzGraphics/Utilities/Interop.h>

namespace DenOfIz::EmbeddedUIShaders
{
    static auto UIVertexShaderSource = R"(
struct VSInput
{
    float3 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
    uint TextureIndex : TEXINDEX;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
    uint TextureIndex : TEXINDEX;
};

cbuffer UIUniforms : register(b0, space1)
{
    float4x4 Projection;
    float4 ScreenSize; // xy: screen dimensions, zw: unused
    float4 FontParams; // x: atlas width, y: atlas height, z: pixel range, w: unused
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.Position = mul(float4(input.Position, 1.0), Projection);
    output.TexCoord = input.TexCoord;
    output.Color = input.Color;
    output.TextureIndex = input.TextureIndex;
    return output;
})";

    static auto UIPixelShaderSource = R"(
struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
    uint TextureIndex : TEXINDEX;
};

Texture2D Textures[] : register(t0, space0);
SamplerState LinearSampler : register(s0, space0);

cbuffer UIUniforms : register(b0, space1)
{
    float4x4 Projection;
    float4 ScreenSize; // xy: screen dimensions, zw: unused
    float4 FontParams; // x: atlas width, y: atlas height, z: pixel range, w: unused
};

// MSDF rendering helper function to calculate median of 3 values
float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

float screenPxRange(float2 texCoord, float pxRange, float2 textureSize)
{
    float2 unitRange = float2(pxRange, pxRange) / textureSize;
    float2 screenTexSize = float2(1.0, 1.0) / fwidth(texCoord);
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

float4 main(PSInput input) : SV_TARGET
{
    // TextureIndex 0 means solid color (no texture)
    if (input.TextureIndex == 0)
    {
        return input.Color;
    }

    float4 texColor = Textures[input.TextureIndex].Sample(LinearSampler, input.TexCoord);

    // For now, treat all non-zero textures as potential MSDF fonts
    // TODO: Better way to distinguish font textures from regular images
    if (input.TextureIndex > 0 && input.TextureIndex < 128)
    {
        // MSDF text rendering
        float3 msdf = texColor.rgb;
        float sd = median(msdf.r, msdf.g, msdf.b);

        // TODO: This assumes all font atlases are the same size, which may not be true
        // A better solution would be to store per-texture metadata
        float2 textureSize = FontParams.xy;
        float pxRange = FontParams.z;

        float screenPxRangeValue = screenPxRange(input.TexCoord, pxRange, textureSize);
        float screenPxDistance = screenPxRangeValue * (sd - 0.5);
        float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);

        if (texColor.a <= 1.0)
        {
            opacity = opacity * texColor.a;
        }
        float4 finalColor = float4(input.Color.rgb, input.Color.a * opacity);
        if (finalColor.a < 0.001f) {
            discard;
        }
        return finalColor;
    }
    else
    {
        // Regular image texture
        return texColor * input.Color;
    }
}
)";

    static InteropArray<Byte> StringToByteArray( const char *str )
    {
        const size_t       len = strlen( str );
        InteropArray<Byte> result( len );
        for ( size_t i = 0; i < len; i++ )
        {
            result.SetElement( i, static_cast<Byte>( str[ i ] ) );
        }
        return result;
    }

    // Get shader sources as byte arrays
    static InteropArray<Byte> GetUIVertexShaderBytes( )
    {
        return StringToByteArray( UIVertexShaderSource );
    }

    static InteropArray<Byte> GetUIPixelShaderBytes( )
    {
        return StringToByteArray( UIPixelShaderSource );
    }

} // namespace DenOfIz::EmbeddedUIShaders
