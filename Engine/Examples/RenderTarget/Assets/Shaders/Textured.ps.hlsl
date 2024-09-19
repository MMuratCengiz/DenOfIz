struct PSInput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

Texture2D texture : register(t0, space1);
SamplerState sampler : register(s0, space1);

float4 main(PSInput input) : SV_TARGET {
    return texture1.Sample(sampler1, input.texCoord);
}