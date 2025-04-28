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

namespace DenOfIz::EmbeddedTextRendererShaders
{
    static auto FontVertexShaderSource = R"(
struct VSInput
{
    float2 Position : POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

struct VSOutput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

cbuffer FontUniforms : register(b0)
{
    float4x4 Projection;
    float4 TextColor;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.Position = mul(float4(input.Position, 0.0, 1.0), Projection);
    output.TexCoord = input.TexCoord;
    output.Color = input.Color * TextColor;
    return output;
})";

    static auto FontPixelShaderSource = R"(
struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

Texture2D FontAtlas : register(t0);
SamplerState FontSampler : register(s0);

cbuffer FontUniforms : register(b0)
{
    float4x4 Projection;
    float4 TextColor;
    float4 TextureSizeParams; // xy: texture size, z: pixel range, w: antialiasing mode (0=None, 1=Grayscale, 2=Subpixel)
};

// MSDF rendering helper function to calculate median of 3 values
float median(float r, float g, float b)
{
    return max(min(r, g), min(max(r, g), b));
}

// Calculate the screen pixel range for proper antialiasing
float screenPxRange(float2 texCoord, float pxRange, float2 textureSize)
{
    float2 unitRange = float2(pxRange, pxRange) / textureSize;
    float2 screenTexSize = float2(1.0, 1.0) / fwidth(texCoord);
    return max(0.5 * dot(unitRange, screenTexSize), 1.0);
}

float4 main(PSInput input) : SV_TARGET
{
    float4 mtsdf = FontAtlas.Sample(FontSampler, input.TexCoord);
    float3 msdf = mtsdf.rgb;
    float2 textureSize = TextureSizeParams.xy;
    float pxRange = TextureSizeParams.z;
    uint aaMode = uint(TextureSizeParams.w);

    if (aaMode == 0) // None
    {
        float sd = median(msdf.r, msdf.g, msdf.b);
        float opacity = sd > 0.5 ? 1.0 : 0.0;
        opacity = opacity * mtsdf.a;
        return float4(input.Color.rgb, input.Color.a * opacity);
    }
    else if (aaMode == 1) // Grayscale - Standard MSDF rendering
    {
        float sd = median(msdf.r, msdf.g, msdf.b);
        float screenPxRangeValue = screenPxRange(input.TexCoord, pxRange, textureSize);
        float screenPxDistance = screenPxRangeValue * (sd - 0.5);
        float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
        opacity = opacity * mtsdf.a;
        return float4(input.Color.rgb, input.Color.a * opacity);
    }
    else // Subpixel - RGB subpixel antialiasing for LCD screens
    {
        float screenPxRangeValue = screenPxRange(input.TexCoord, pxRange, textureSize);

        float sd = median(msdf.r, msdf.g, msdf.b);
        float alphaBase = clamp(screenPxRangeValue * (sd - 0.5) + 0.5, 0.0, 1.0) * mtsdf.a;
        float2 pixelPos = input.Position.xy;
        float subpixelOffset = frac(pixelPos.x / 3.0);
        float3 weights = float3(1.0, 1.0, 1.0);
        if (subpixelOffset < 0.33) {
            weights = float3(
                lerp(1.1, 1.0, subpixelOffset * 3.0),
                lerp(0.9, 1.0, subpixelOffset * 3.0),
                lerp(0.8, 0.9, subpixelOffset * 3.0)
            );
        } else if (subpixelOffset < 0.66) {
            float localOffset = (subpixelOffset - 0.33) * 3.0;
            weights = float3(
                lerp(1.0, 0.9, localOffset),
                lerp(1.0, 1.1, localOffset),
                lerp(0.9, 1.0, localOffset)
            );
        } else {
            float localOffset = (subpixelOffset - 0.66) * 3.0;
            weights = float3(
                lerp(0.9, 0.8, localOffset),
                lerp(1.1, 0.9, localOffset),
                lerp(1.0, 1.1, localOffset)
            );
        }

        weights /= ((weights.r + weights.g + weights.b) / 3.0);

        const float subpixelStrength = 0.15; // Very subtle effect
        float3 alphaChannels = lerp(float3(alphaBase, alphaBase, alphaBase),
                                  alphaBase * weights,
                                  subpixelStrength);

        return float4(
            input.Color.r * alphaChannels.r,
            input.Color.g * alphaChannels.g,
            input.Color.b * alphaChannels.b,
            input.Color.a * alphaBase
        );
    }
})";

    // Helper function to create a byte array from a string
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
    static InteropArray<Byte> GetFontVertexShaderBytes( )
    {
        return StringToByteArray( FontVertexShaderSource );
    }

    static InteropArray<Byte> GetFontPixelShaderBytes( )
    {
        return StringToByteArray( FontPixelShaderSource );
    }
} // namespace DenOfIz::EmbeddedShaders
