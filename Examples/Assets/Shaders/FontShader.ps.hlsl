struct PSInput
{
    float4 Position : SV_POSITION;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

Texture2D FontAtlas : register(t0);
SamplerState FontSampler : register(s0);

float4 main(PSInput input) : SV_TARGET
{
    float alpha = FontAtlas.Sample(FontSampler, input.TexCoord).r;
    return float4(input.Color.rgb, input.Color.a * alpha);
}