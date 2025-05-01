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
    float3 WorldPosition : TEXCOORD1;
};

// Advanced lighting calculation with different components
float3 CalculateLighting(float3 normal, float3 color, float2 uv, float3 worldPosition)
{
    // Directional light - main light from above
    float3 mainLightDir = normalize(float3(0.2, 0.8, 0.3));
    float3 mainLightColor = float3(1.0, 0.95, 0.8) * 0.8; // Warm sunlight
    
    // Secondary fill light - softer from the side
    float3 fillLightDir = normalize(float3(-0.5, 0.3, -0.2));
    float3 fillLightColor = float3(0.6, 0.7, 1.0) * 0.3; // Cooler fill light
    
    // Ambient light - environment lighting
    float3 ambient = color * float3(0.2, 0.25, 0.3); // Bluish ambient for sky reflection
    
    // Main light diffuse
    float mainDiff = max(dot(normal, mainLightDir), 0.0);
    // Soften the lighting to make it less harsh
    mainDiff = pow(mainDiff, 0.8) * 0.8;
    float3 mainDiffuse = color * mainDiff * mainLightColor;
    
    // Fill light diffuse - gives depth
    float fillDiff = max(dot(normal, fillLightDir), 0.0);
    fillDiff = pow(fillDiff, 0.6) * 0.6; // Softer falloff for fill light
    float3 fillDiffuse = color * fillDiff * fillLightColor;
    
    // Rim lighting effect for silhouette enhancement
    float3 viewDir = normalize(float3(0.0, 0.0, -1.0)); // Simplification - assumes view from Z axis
    float rim = 1.0 - max(dot(viewDir, normal), 0.0);
    rim = pow(rim, 3.0) * 0.4;
    float3 rimLight = color * rim * float3(1.0, 0.97, 0.9); // Warm rim light
    
    // Self-shadowing effect based on UV.y (height on the blade)
    float shadowFactor = 1.0 - pow(1.0 - uv.y, 2.0) * 0.3;
    
    // Add a subtle subsurface scattering effect for thin blades
    float sss = max(0.0, dot(-viewDir, mainLightDir)) * pow(uv.y, 0.5) * 0.2;
    float3 subsurface = color * sss * float3(0.5, 0.8, 0.2); // Green-tinted SSS
    
    // Apply distance fog or depth haze (subtle)
    float fogFactor = saturate(length(worldPosition) / MaxDistance * 0.7);
    float3 fogColor = float3(0.7, 0.8, 0.9); // Light blue sky color
    
    // Combine all lighting components
    float3 finalColor = ambient + (mainDiffuse + fillDiffuse + rimLight + subsurface) * shadowFactor;
    
    // Apply very subtle fog
    return lerp(finalColor, fogColor, fogFactor * 0.2);
}

// Improved grass texture function that varies based on blade parameters
float4 SampleGrassTexture(float2 uv, float4 color)
{
    // Sample the base texture
    float4 texColor = grassTexture.Sample(grassSampler, uv);
    
    // Adjust alpha for more natural shape - more transparent at edges and tips
    float edgeFade = 1.0 - abs(uv.x - 0.5) * 2.0; // Fade at edges (0 at edges, 1 in center)
    float tipFade = 1.0 - pow(uv.y, 1.5); // More aggressive fade at tips
    
    // Combine fades with texture alpha
    float alpha = texColor.a * edgeFade * (1.0 - tipFade * 0.7);
    
    // Allow some transparency at the tip
    alpha = lerp(alpha, alpha * 0.6, uv.y * uv.y);
    
    // Add some subtle detail variation
    float detail = sin(uv.y * 20.0 + uv.x * 5.0) * 0.05 + 0.95;
    
    return float4(texColor.rgb * detail, alpha);
}

// Main pixel shader function
float4 main(PixelInput input) : SV_TARGET
{
    // Sample grass texture with improved alpha control
    float4 texColor = SampleGrassTexture(input.TexCoord, input.Color);
    
    // Apply advanced lighting
    float3 litColor = CalculateLighting(normalize(input.Normal), input.Color.rgb, input.TexCoord, input.WorldPosition);
    
    // Apply subtle color variations along the blade
    float heightVariation = sin(input.TexCoord.y * 8.0) * 0.05 + 0.95;
    
    // Combine lighting with texture and color
    float3 finalColor = litColor * texColor.rgb * heightVariation;
    
    // Apply alpha from texture
    return float4(finalColor, texColor.a);
}