struct PSInput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

Texture2D texture1 : register(t0, space1);
// Texture2D tint : register(t1, space1);
SamplerState sampler1 : register(s0, space1);

float4 main(PSInput input) : SV_TARGET {
    return texture1.Sample(sampler1, input.texCoord);
}