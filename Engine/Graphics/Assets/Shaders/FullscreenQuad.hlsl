#pragma pack_matrix (row_major)

struct Output
{
    float4 position_cs : SV_POSITION;
    float2 texcoord : TEXCOORD;
};

Output main(uint id: SV_VertexID)
{
    Output output;

    output.texcoord = float2(id % 2, (id % 4) >> 1);
    output.position_cs = float4(output.texcoord * float2(2, -2) + float2(-1, 1), 0, 1);

    return output;
}