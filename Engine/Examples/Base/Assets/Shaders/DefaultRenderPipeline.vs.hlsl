struct PSInput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

cbuffer ConstantBuffer : register(b0, space2)
{
    float4x4 model;
};

cbuffer ConstantBuffer : register(b0)
{
    float4x4 viewProjection;
};

cbuffer ConstantBuffer : register(b1)
{
    float deltaTime;
};

PSInput main(float3 position : POSITION0, float3 normal : NORMAL0, float2 texCoord : TEXCOORD0) {
    PSInput result;
    float4 position4 = float4(position.x, position.y, position.z, 1.0f);
    float4x4 mvp = mul(model, viewProjection);
    result.position = mul(position4, mvp);
    result.texCoord = texCoord * deltaTime;
    return result;
}