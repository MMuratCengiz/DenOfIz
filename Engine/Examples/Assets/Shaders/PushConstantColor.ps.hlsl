struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

struct PushConstants {
    float4 color;
};

[[vk::push_constant]] ConstantBuffer<PushConstants> pushConstants : register(b0, space99);

float4 main(PSInput input) : SV_TARGET {
    return pushConstants.color;
}
