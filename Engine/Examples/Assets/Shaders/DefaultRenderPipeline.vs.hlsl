struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 texCoord : TEXCOORD;
};

cbuffer ConstantBuffer : register( b0, space2 )
{
    float4x4 model;
};

cbuffer ConstantBuffer : register( b0 )
{
    float4x4 viewProjection;
};

cbuffer ConstantBuffer : register( b1 )
{
    float deltaTime;
};

Texture2D heightTexture : register(t2, space1);
SamplerState samplerState : register(s0, space1);

PSInput main( float3 position : POSITION0, float3 normal : NORMAL0, float2 texCoord : TEXCOORD0 )
{
    PSInput result;

    float height = heightTexture.SampleLevel(samplerState, position.xy, 0.0f).r;

    float4   position4 = float4( position, 1.0f );
    position4.z += height * 0.001;
    float4x4 mvp       = mul( model, viewProjection );
    result.position    = mul( position4, mvp );

    result.texCoord = texCoord * deltaTime;
    result.normal   = normalize( mul( float4( normal, 0.0 ), model ).xyz );

    return result;
}
