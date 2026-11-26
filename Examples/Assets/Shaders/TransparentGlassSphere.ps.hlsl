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

cbuffer AnimationParams : register(b1, space1)
{
    float alphaValue;
    float3 padding;
};

float4 main(PSInput input) : SV_TARGET {
    float3 baseColor = materialParams.color.rgb;
    float baseOpacity = materialParams.color.a * alphaValue;
    float refractionIndex = materialParams.refractionIndex;
    float fresnelPower = materialParams.fresnelPower;

    float3 lightDirection = normalize(float3(1.0, 1.0, -1.0));
    float3 viewDir = normalize(float3(0, 0, -5) - input.worldPos);

    float diffuseFactor = saturate(dot(input.normal, lightDirection));
    float ambientFactor = 0.2;

    float fresnel = pow(1.0 - saturate(dot(viewDir, input.normal)), fresnelPower);

    float3 halfVec = normalize(lightDirection + viewDir);
    float specFactor = pow(saturate(dot(input.normal, halfVec)), 64.0) * 0.7;

    float edgeHighlight = pow(1.0 - abs(dot(viewDir, input.normal)), 3.0) * 0.5;

    float3 finalColor = baseColor * (diffuseFactor * 0.5 + ambientFactor);
    finalColor += specFactor;
    finalColor += fresnel * 0.3;
    finalColor += edgeHighlight;

    float opacity = max(0.2, baseOpacity * (fresnel * 0.5 + 0.5));
    return float4(finalColor, opacity);
}