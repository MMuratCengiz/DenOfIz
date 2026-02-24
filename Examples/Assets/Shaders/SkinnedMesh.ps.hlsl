cbuffer MaterialConstants : register(b2)
{
    float4 g_DiffuseColor;
    float4 g_AmbientColor;
    float g_SpecularPower;
    float g_SpecularIntensity;
    float2 g_Padding;
};

cbuffer PerFrameConstants : register(b1)
{
    float4x4 g_ViewProjection;
    float4 g_CameraPosition;
    float4 g_Time; // x = total time, y = delta time, z,w = unused
};

static const float3 g_LightDirection = float3(0.577f, -0.577f, 0.577f);
static const float3 g_LightColor = float3(1.0f, 1.0f, 1.0f);

Texture2D g_DiffuseTexture : register(t0);
SamplerState g_Sampler : register(s0);

struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float3 WorldPosition : OTHER0;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 texColor = g_DiffuseTexture.Sample(g_Sampler, input.TexCoord);
    float4 diffuseColor = texColor * g_DiffuseColor;
    
    if (all(diffuseColor.rgb < 0.01f)) {
        diffuseColor = float4(0.8f, 0.8f, 0.8f, 1.0f);
    }
    
    float3 normal = normalize(input.Normal);
    
    float ndotl = max(0.0f, dot(normal, -g_LightDirection));
    float3 diffuse = g_LightColor * ndotl;
    
    float3 viewDir = normalize(g_CameraPosition.xyz - input.WorldPosition);
    float3 halfVec = normalize(-g_LightDirection + viewDir);
    float ndoth = max(0.0f, dot(normal, halfVec));
    float specFactor = pow(ndoth, g_SpecularPower) * g_SpecularIntensity;
    float3 specular = g_LightColor * specFactor;
    float3 ambient = max(g_AmbientColor.rgb, float3(0.1f, 0.1f, 0.1f));
    float3 finalColor = diffuseColor.rgb * (ambient + diffuse) + specular;
    return float4(finalColor, diffuseColor.a);
}