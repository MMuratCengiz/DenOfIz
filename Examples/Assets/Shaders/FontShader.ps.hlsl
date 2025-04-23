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

float4 main(PSInput input) : SV_TARGET
{
    // Sample the MSDF texture
    float3 msdf = FontAtlas.Sample(FontSampler, input.TexCoord).rgb;
    
    // Calculate signed distance from the median of the three channels
    float sdf = median(msdf.r, msdf.g, msdf.b);
    
    // Get pixel range from uniform
    float pxRange = TextureSizeParams.z;
    
    // Calculate proper screen-space scale factor for consistent rendering
    // This helps ensure consistent thickness regardless of font size
    float screenPxRange = pxRange;
    
    // Apply antialiasing with proper range
    float screenDist = screenPxRange * (sdf - 0.5);
    float opacity = smoothstep(-0.5, 0.5, screenDist);
    
    // Mix the color with the calculated opacity
    return float4(input.Color.rgb, input.Color.a * opacity);
}