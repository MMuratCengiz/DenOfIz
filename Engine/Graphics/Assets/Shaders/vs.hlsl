struct PSInput {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

cbuffer ConstantBuffer : register(b0)
{
    float deltaTime;
};

PSInput main(float4 position : POSITION0) {
    PSInput result;
    result.position = position;
    result.color = float4(deltaTime, 0.5 * deltaTime, 0.5* deltaTime, 1.0f);
    result.color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    return result;
}