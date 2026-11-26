struct PSInput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPos : POSITION1;
};

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
    float3 lightDirection = normalize(float3(1.0, 1.0, -1.0));
    float3 baseColor = materialParams.color.rgb;

    float diffuseFactor = saturate(dot(input.normal, lightDirection));
    float ambientFactor = 0.3;

    float3 viewDir = normalize(float3(0, 0, -5) - input.worldPos);
    float3 halfVec = normalize(lightDirection + viewDir);
    float specFactor = pow(saturate(dot(input.normal, halfVec)), 32.0) * 0.5;
    
    float3 finalColor = baseColor * (diffuseFactor + ambientFactor) + specFactor;

    return float4(finalColor, 1.0);
}