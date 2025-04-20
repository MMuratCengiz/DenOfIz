#define MAX_BONES 128

cbuffer BoneTransforms : register(b0)
{
    float4x4 g_BoneTransforms[MAX_BONES];
};

cbuffer PerFrameConstants : register(b1)
{
    float4x4 g_ViewProjection;
    float4 g_CameraPosition;
    float4 g_Time; // x = total time, y = delta time, z,w = unused
};

struct VS_INPUT
{
    float3 Position : POSITION0;
    float3 Normal : NORMAL0;
    float2 TexCoord : TEXCOORD0;
    float4 Tangent : TANGENT0;
    uint4 BoneIndices : BLENDINDICES0;
    float4 BoneWeights : BLENDWEIGHT0;
};

// Vertex output
struct VS_OUTPUT
{
    float4 Position : SV_POSITION;
    float3 WorldPosition : OTHER0;
    float3 Normal : NORMAL0;
    float2 TexCoord : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    
    float4 positionSkinned = float4(0.0f, 0.0f, 0.0f, 0.0f);
    float3 normalSkinned = float3(0.0f, 0.0f, 0.0f);
    
    positionSkinned = float4(input.Position.xyz, 1.0f);
    normalSkinned = input.Normal.xyz;
    
    float totalWeight = input.BoneWeights.x + input.BoneWeights.y + input.BoneWeights.z + input.BoneWeights.w;
    
    if (totalWeight > 0.001f)
    {
        for (int i = 0; i < 4; ++i)
        {
            uint boneIndex = input.BoneIndices[i];
            float boneWeight = input.BoneWeights[i];
            
            if (boneWeight > 0.001f && boneIndex < MAX_BONES)
            {
                float4x4 boneMatrix = g_BoneTransforms[boneIndex];
                
                bool isValidMatrix = true;
                for (int row = 0; row < 4 && isValidMatrix; ++row)
                {
                    for (int col = 0; col < 4 && isValidMatrix; ++col)
                    {
                        float val = boneMatrix[row][col];
                        isValidMatrix = isValidMatrix && (val > -1000000.0f && val < 1000000.0f);
                    }
                }
                
                if (isValidMatrix)
                {
                    float4 positionBone = mul(float4(input.Position.xyz, 1.0f), boneMatrix);
                    positionSkinned += positionBone * boneWeight;
                    float3 normalBone = mul(input.Normal.xyz, (float3x3)boneMatrix);
                    normalSkinned += normalBone * boneWeight;
                }
            }
        }
        
        if (length(positionSkinned) > 0.0f)
        {
            positionSkinned.w = 1.0f;
        }
        else
        {
            positionSkinned = float4(input.Position.xyz, 1.0f);
            normalSkinned = input.Normal.xyz;
        }
    }
    else
    {
        positionSkinned = float4(input.Position.xyz, 1.0f);
        normalSkinned = input.Normal.xyz;
    }

    output.WorldPosition = positionSkinned.xyz;
    output.Position = mul(positionSkinned, g_ViewProjection);
    output.Normal = normalize(normalSkinned);
    output.TexCoord = input.TexCoord;
    
    return output;
}