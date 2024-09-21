#pragma pack_matrix (row_major)

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

PSInput main(uint id: SV_VertexID)
{
    PSInput result;

    result.texCoord = float2(id % 2, (id % 4) >> 1);
    result.position = float4(result.texCoord * float2(2, -2) + float2(-1, 1), 0, 1);

    return result;
}