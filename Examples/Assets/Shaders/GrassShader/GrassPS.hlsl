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

// Enhanced grass texture function with microtexture and edge details
float4 SampleGrassTexture(float2 uv, float4 color)
{
    // Sample the base texture
    float4 texColor = grassTexture.Sample(grassSampler, uv);
    
    // Create more detailed edge shape
    // Smooth cubic falloff from center to create more natural blade shapes
    float edgeDistance = abs(uv.x - 0.5) * 2.0;
    float edgeFade = 1.0 - pow(edgeDistance, 1.7); // Sharper towards edge, smooth in center
    
    // Top fade with ultra-fine tip
    float tipHeight = pow(uv.y, 3.0); // Sharper power curve for finer tips
    float tipFade = smoothstep(1.0, 0.4, tipHeight);
    
    // Create subtle veins along the blade
    float veinPattern = 1.0;
    float veinCenter = abs(uv.x - 0.5);
    if (veinCenter < 0.1) {
        // Central vein is slightly thicker
        veinPattern = 1.1 - veinCenter * 3.0;
    } else if (frac(veinCenter * 10.0) < 0.3) {
        // Add smaller secondary veins
        veinPattern = 1.05;
    }
    
    // Subtle micro-detail across the blade surface
    float microDetail = sin(uv.y * 40.0 + uv.x * 30.0) * 0.03 + 
                        sin(uv.x * 50.0 - uv.y * 20.0) * 0.02 + 0.97;
    
    // Combine all factors for alpha
    float baseAlpha = texColor.a * edgeFade;
    
    // More transparent at the very tip
    if (uv.y > 0.9) {
        float tipTransparency = (uv.y - 0.9) * 10.0; // 0-1 across the last 10% of blade
        baseAlpha *= 1.0 - tipTransparency * 0.6;
    }
    
    // Create subtle irregular serrations at blade edges
    if (edgeDistance > 0.9 && uv.y > 0.5) {
        float serration = sin(uv.y * 30.0) * 0.5 + 0.5;
        baseAlpha *= serration;
    }
    
    return float4(texColor.rgb * veinPattern * microDetail, baseAlpha);
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