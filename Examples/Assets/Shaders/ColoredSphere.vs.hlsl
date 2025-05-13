struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float3 worldPos : POSITION1;
};

cbuffer ConstantBuffer : register(b0, space30)
{
    float4x4 model;
};

cbuffer ConstantBuffer : register(b0)
{
    float4x4 viewProjection;
};

PSInput main(float3 position : POSITION0, float3 normal : NORMAL0, float2 texCoord : TEXCOORD0)
{
    PSInput result;

    float4 position4 = float4(position, 1.0f);
    float4x4 mvp = mul(model, viewProjection);
    result.position = mul(position4, mvp);
    result.worldPos = mul(position4, model).xyz;
    result.normal = normalize(mul(float4(normal, 0.0), model).xyz);

    return result;
}