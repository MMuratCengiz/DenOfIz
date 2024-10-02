#pragma pack_matrix( row_major )

struct PSInput
{
    float4 position : SV_POSITION;
    float2 texCoord : TEXCOORD;
};

PSInput main( uint id : SV_VertexID )
{
    PSInput result;
    result.texCoord = float2( id << 1 & 2, id & 2 );
    result.position = float4( result.texCoord * float2( 2.0f, -2.0f ) + float2( -1.0f, 1.0f ), 0.0f, 1.0f );
    return result;
}
