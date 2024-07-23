struct PSInput {
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};


cbuffer ConstantBuffer : register(b0)
{
    float4x4 mvpMatrix;
};

cbuffer ConstantBuffer : register(b1)
{
    float deltaTime;
};

PSInput main(float3 position : POSITION0, float3 normal : NORMAL0, float2 texCoord : TEXCOORD0) {
    PSInput result;
    float4 position4 = float4(position.x, position.y, position.z, 1.0f);
    result.position = mul(position4, mvpMatrix);
//     result.position = position4;
    result.texCoord = texCoord * deltaTime;
    return result;
}