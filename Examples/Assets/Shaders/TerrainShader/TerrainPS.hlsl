/*
Den Of Iz - Game/Game Engine
Simple Pixel Shader for Terrain Rendering
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

// Terrain texture sampler
Texture2D terrainTexture : register(t0);
SamplerState terrainSampler : register(s0);

float4 main(PSInput input) : SV_TARGET
{
    // Sample the terrain texture with repeating coords
    float4 texColor = terrainTexture.Sample(terrainSampler, input.texCoord);
    
    // Simple directional light calculation for basic lighting
    float3 lightDir = normalize(float3(0.5, 0.7, 0.5));
    float3 lightColor = float3(1.0, 0.98, 0.9);
    
    // Make sure normal is normalized
    float3 normal = normalize(input.normal);
    
    // Calculate diffuse lighting
    float diffuseFactor = max(dot(normal, lightDir), 0.0);
    
    // Add ambient light to prevent completely black shadows
    float3 ambient = texColor.rgb * 0.3;
    float3 diffuse = texColor.rgb * diffuseFactor * lightColor;
    
    // Combine lighting components
    float3 finalColor = ambient + diffuse;
    
    return float4(finalColor, 1.0);
}