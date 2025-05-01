/*
Den Of Iz - Game/Game Engine
Pixel Shader for Grass Rendering
*/

// Grass texture
Texture2D grassTexture : register(t0, space0);
SamplerState grassSampler : register(s0, space0);

// Constants shared with the CPU side
cbuffer GrassConstants : register(b0, space0)
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
    float3 Padding;
};

// Input from mesh shader
struct PixelInput
{
    float4 Position : SV_Position;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

// Simple lighting calculation
float3 CalculateLighting(float3 normal, float3 color)
{
    // Simple directional light from above and slightly to the front
    float3 lightDir = normalize(float3(0.2, 0.8, 0.3));
    
    // Ambient component
    float3 ambient = color * 0.4;
    
    // Diffuse component
    float diff = max(dot(normal, lightDir), 0.0);
    float3 diffuse = color * diff * 0.6;
    
    // Add a subtle rim lighting effect for more depth
    float3 viewDir = float3(0.0, 0.0, -1.0); // Simplification - assumes view from Z axis
    float rim = 1.0 - max(dot(viewDir, normal), 0.0);
    rim = pow(rim, 4.0) * 0.2;
    
    return ambient + diffuse + (color * rim);
}

// Main pixel shader function
float4 main(PixelInput input) : SV_TARGET
{
    // Sample grass texture
    float4 texColor = grassTexture.Sample(grassSampler, input.TexCoord);
    
    // Apply lighting
    float3 litColor = CalculateLighting(normalize(input.Normal), input.Color.rgb);
    
    // Final color with texture alpha
    return float4(litColor, texColor.a);
}