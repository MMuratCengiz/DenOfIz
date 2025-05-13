struct PSInput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPos : POSITION1;
};

// Per-sphere color and transparency parameters
struct MaterialParams {
    float4 color;       // RGB = base color, A = opacity multiplier
    float refractionIndex;
    float fresnelPower;
    float2 padding; // for 16-byte alignment
};

cbuffer ConstantBuffer : register(b0, space1)
{
    MaterialParams materialParams;
};

// External parameter for animated alpha
cbuffer AnimationParams : register(b1, space1)
{
    float alphaValue;
    float3 padding; // for 16-byte alignment
};

float4 main(PSInput input) : SV_TARGET {
    // Glass material properties
    float3 baseColor = materialParams.color.rgb;
    float baseOpacity = materialParams.color.a * alphaValue;
    float refractionIndex = materialParams.refractionIndex;
    float fresnelPower = materialParams.fresnelPower;
    
    // Basic lighting setup
    float3 lightDirection = normalize(float3(1.0, 1.0, -1.0));
    float3 viewDir = normalize(float3(0, 0, -5) - input.worldPos); // Assume camera at (0,0,-5)
    
    // Calculate diffuse lighting
    float diffuseFactor = saturate(dot(input.normal, lightDirection));
    float ambientFactor = 0.2; // Less ambient for glass
    
    // Fresnel effect - more reflective at glancing angles
    float fresnel = pow(1.0 - saturate(dot(viewDir, input.normal)), fresnelPower);
    
    // Specular highlight
    float3 halfVec = normalize(lightDirection + viewDir);
    float specFactor = pow(saturate(dot(input.normal, halfVec)), 64.0) * 0.7;
    
    // Refraction distortion effect (simplified)
    float edgeHighlight = pow(1.0 - abs(dot(viewDir, input.normal)), 3.0) * 0.5;
    
    // Combine lighting components
    float3 finalColor = baseColor * (diffuseFactor * 0.5 + ambientFactor);
    finalColor += specFactor; // Add specular
    finalColor += fresnel * 0.3; // Add fresnel edge highlight
    finalColor += edgeHighlight; // Add edge highlight
    
    // Glass opacity - more transparent in the center, more opaque at edges
    // Ensure minimum opacity doesn't go below 0.2 to prevent objects from completely disappearing
    float opacity = max(0.2, baseOpacity * (fresnel * 0.5 + 0.5));
    return float4(finalColor, opacity);
}