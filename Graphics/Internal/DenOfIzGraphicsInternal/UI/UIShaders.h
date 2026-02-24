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

#include <vector>

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
    float4 FontParams; // x: atlas width, y: atlas height, z: pixel range, w: first non-font texture index
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

    static auto UIPixelShaderSourceArray = R"(
struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
    uint TextureIndex : TEXINDEX;
};

Texture2D Textures[128] : register(t0, space0);
SamplerState LinearSampler : register(s0, space0);

cbuffer UIUniforms : register(b0, space1)
{
    float4x4 Projection;
    float4 ScreenSize; // xy: screen dimensions, zw: unused
    float4 FontParams; // x: atlas width, y: atlas height, z: pixel range, w: first non-font texture index
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

    // Textures with index < FontParams.w are font textures (MTSDF)
    // Textures with index >= FontParams.w are regular image textures
    if (input.TextureIndex > 0 && input.TextureIndex < (uint)FontParams.w)
    {
        float4 mtsdf = texColor;
        float3 msdf = mtsdf.rgb;
        float sd = median(msdf.r, msdf.g, msdf.b);

        float2 textureSize = FontParams.xy;
        float pxRange = FontParams.z;

        float screenPxRangeValue = max(screenPxRange(input.TexCoord, pxRange, textureSize), 1.0);
        float pxDist = screenPxRangeValue * clamp(sd - 0.5, mtsdf.a - 0.52, mtsdf.a - 0.48);
        float opacity = clamp(pxDist + 0.5, 0.0, 1.0);

        float4 finalColor = float4(input.Color.rgb, input.Color.a * opacity);
        if (finalColor.a < 0.1f) {
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

    static auto UIPixelShaderSourceSingle = R"(
struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
    uint TextureIndex : TEXINDEX;
};

Texture2D Texture : register(t0, space0);
SamplerState LinearSampler : register(s0, space0);

cbuffer UIUniforms : register(b0, space1)
{
    float4x4 Projection;
    float4 ScreenSize; // xy: screen dimensions, zw: unused
    float4 FontParams; // x: atlas width, y: atlas height, z: pixel range, w: first non-font texture index
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

    float4 texColor = Texture.Sample(LinearSampler, input.TexCoord);

    // TextureIndex 1 = font texture (MTSDF), TextureIndex 2 = regular image
    if (input.TextureIndex == 1)
    {
        float4 mtsdf = texColor;
        float3 msdf = mtsdf.rgb;
        float sd = median(msdf.r, msdf.g, msdf.b);

        float2 textureSize = FontParams.xy;
        float pxRange = FontParams.z;

        float screenPxRangeValue = max(screenPxRange(input.TexCoord, pxRange, textureSize), 1.0);
        float pxDist = screenPxRangeValue * clamp(sd - 0.5, mtsdf.a - 0.52, mtsdf.a - 0.48);
        float opacity = clamp(pxDist + 0.5, 0.0, 1.0);

        float4 finalColor = float4(input.Color.rgb, input.Color.a * opacity);
        if (finalColor.a < 0.1f) {
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

    static std::vector<Byte> StringToByteArray( const char *str )
    {
        const size_t      len = strlen( str );
        std::vector<Byte> result( len );
        for ( size_t i = 0; i < len; i++ )
        {
            result[ i ] = static_cast<Byte>( str[ i ] );
        }
        return result;
    }

    // Get shader sources as byte arrays
    static std::vector<Byte> GetUIVertexShaderBytes( )
    {
        return StringToByteArray( UIVertexShaderSource );
    }

    static std::vector<Byte> GetUIPixelShaderBytes( bool useSrvArray = true )
    {
        return StringToByteArray( useSrvArray ? UIPixelShaderSourceArray : UIPixelShaderSourceSingle );
    }

} // namespace DenOfIz::EmbeddedUIShaders
