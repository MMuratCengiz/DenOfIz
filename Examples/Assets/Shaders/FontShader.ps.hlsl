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
    float4 TextureSizeParams; // xy: texture size, z: pixel range, w: unused
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
    float sd = median(msdf.r, msdf.g, msdf.b);
    float2 textureSize = TextureSizeParams.xy;
    float pxRange = TextureSizeParams.z;
    float screenPxRangeValue = screenPxRange(input.TexCoord, pxRange, textureSize);
    float screenPxDistance = screenPxRangeValue * (sd - 0.5);
    float opacity = clamp(screenPxDistance + 0.5, 0.0, 1.0);
    opacity = opacity * mtsdf.a;
    return float4(input.Color.rgb, input.Color.a * opacity);
}