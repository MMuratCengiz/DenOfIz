/*
Den Of Iz - Game/Game Engine
Simple Vertex Shader for Terrain Rendering
*/

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD0;
    float3 worldPos : TEXCOORD1;
};

// Terrain specific constants - matches the GrassConstants structure
cbuffer GrassConstants : register(b0)
{
    float4x4 ViewProjection;
    float4x4 Model;
    
    float4 WindDirection; // xyz = direction, w = strength
    float4 GrassColor;
    float4 GrassColorVariation;
    
    float Time;
    float DensityFactor;
    float HeightScale;
    float WidthScale;
    
    float MaxDistance;
    float TerrainScale;
    float TerrainHeight;
    float TerrainRoughness;
};

Texture2D terrainTexture : register(t0);
SamplerState terrainSampler : register(s0);

PSInput main(float3 position : POSITION0, float3 normal : NORMAL0, float2 texCoord : TEXCOORD0)
{
    PSInput result;
    float4x4 mvp = mul(Model, ViewProjection);
    float4 worldPosition = mul(float4(position, 1.0f), Model);
    result.position = mul(float4(position, 1.0f), mvp);
    result.normal = normalize(mul(float4(normal, 0.0), Model).xyz);
    result.texCoord = texCoord * 4.0;
    result.worldPos = worldPosition.xyz;
    return result;
}