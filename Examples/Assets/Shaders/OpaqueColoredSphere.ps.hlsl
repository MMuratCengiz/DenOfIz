struct PSInput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPos : POSITION1;
};

// Per-sphere color parameter
struct MaterialParams {
    float4 color;
    float refractionIndex;
    float fresnelPower;
    float2 padding;
};

cbuffer ConstantBuffer : register(b0, space1)
{
    MaterialParams materialParams;
};

float4 main(PSInput input) : SV_TARGET {
    // Basic lighting calculation
    float3 lightDirection = normalize(float3(1.0, 1.0, -1.0));
    float3 baseColor = materialParams.color.rgb;
    
    // Calculate diffuse lighting
    float diffuseFactor = saturate(dot(input.normal, lightDirection));
    float ambientFactor = 0.3; // Add some ambient light
    
    // Add some specular highlight
    float3 viewDir = normalize(float3(0, 0, -5) - input.worldPos); // Assume camera at (0,0,-5)
    float3 halfVec = normalize(lightDirection + viewDir);
    float specFactor = pow(saturate(dot(input.normal, halfVec)), 32.0) * 0.5;
    
    // Combine lighting components
    float3 finalColor = baseColor * (diffuseFactor + ambientFactor) + specFactor;
    
    return float4(finalColor, 1.0); // Fully opaque
}