struct PSInput {
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

Texture2D albedoTexture : register(t0, space1);
Texture2D normalTexture : register(t1, space1);
Texture2D roughnessTexture : register(t3, space1);
Texture2D aoTexture : register(t4, space1);

SamplerState samplerState : register(s0, space1);

float3 applyNormalMap(float3 normal, float2 texCoord) {
    float3 normalMapValue = normalTexture.Sample(samplerState, texCoord).rgb;
    return normalize(2.0f * normalMapValue - 1.0f);
}

float4 main(PSInput input) : SV_TARGET {
    float4 albedo = albedoTexture.Sample(samplerState, input.texCoord);
    float3 normal = applyNormalMap(input.normal, input.texCoord);
    float roughness = roughnessTexture.Sample(samplerState, input.texCoord).r;
    float ambientOcclusion = aoTexture.Sample(samplerState, input.texCoord).r;

    float parallaxFactor = 0.05f;
    float2 newTexCoord = input.texCoord * parallaxFactor * input.normal.xy;

    float4 parallaxAlbedo = albedoTexture.Sample(samplerState, newTexCoord);
    float3 parallaxNormal = applyNormalMap(input.normal, newTexCoord);

    parallaxAlbedo.rgb *= ambientOcclusion;

    float3 lightDirection = normalize(float3(0.0, 1.0, 1.0));
    float3 lightColor = float3(1.0, 1.0, 1.0);

    float diffuseFactor = saturate(dot(parallaxNormal, lightDirection));
    float3 diffuseLighting = lightColor * diffuseFactor;

    float3 finalColor = parallaxAlbedo.rgb * diffuseLighting * roughness;

    return float4(finalColor, parallaxAlbedo.a);
}
