struct PSInput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

Texture2D albedoTexture : register(t0, space1);
Texture2D normalTexture : register(t1, space1);
Texture2D heightTexture : register(t2, space1);
Texture2D metallicTexture : register(t3, space1);
Texture2D roughnessTexture : register(t4, space1);
Texture2D aoTexture : register(t5, space1);
SamplerState albedoSampler : register(s0, space1);

float4 main(PSInput input) : SV_TARGET {
    float4 albedo = albedoTexture.Sample(albedoSampler, input.texCoord);
    return albedo;
}