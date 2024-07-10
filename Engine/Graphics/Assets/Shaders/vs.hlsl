struct PSInput {
    float4 position : SV_POSITION;
    float4 color : COLOR;
};

cbuffer ConstantBuffer : register(b0)
{
    float deltaTime;
};

PSInput main(float3 position : POSITION0, float3 normal : NORMAL0, float2 texCoord : TEXCOORD0) {
    PSInput result;
    result.position = float4(position.x, position.y, position.z, 1.0f);
    result.color = float4(deltaTime, 0.5 * deltaTime, 0.5* deltaTime, 1.0f);
    result.color = float4(1.0f, 0.0f, 0.0f, 1.0f);
    return result;
}