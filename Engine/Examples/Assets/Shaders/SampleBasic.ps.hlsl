struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

Texture2D tex : register(t0);
SamplerState texSampler : register(s0);

float4 main(PSInput input) : SV_Target
{
    float4 color = tex.Sample(texSampler, input.texCoord);
    return color;
}