struct PSInput {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

cbuffer ConstantBuffer : register(b0)
{
    float4 deltaTime;
};

PSInput main(float4 position : POSITION0, float4 color : COLOR0) {
    PSInput result;
    result.position = position;
    result.color = color * deltaTime;
    return result;
}