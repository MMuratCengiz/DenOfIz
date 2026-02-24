#define HLSL
#ifndef UINT
#define UINT uint32_t
#endif

#include "ModelViewerRaytracing.h"

struct BistroVertex
{
    float4 Position;
    float4 Normal;
    float2 UV;
    float4 Tangent;
    float4 Bitangent;
};

struct RayTraceMeshInfo
{
    uint indexOffsetBytes;
    uint uvAttributeOffsetBytes;
    uint normalAttributeOffsetBytes;
    uint tangentAttributeOffsetBytes;
    uint bitangentAttributeOffsetBytes;
    uint positionAttributeOffsetBytes;
    uint attributeStrideBytes;
    uint materialInstanceId;
};

struct MaterialData
{
    uint albedoTextureHandle;
    uint normalTextureHandle;
    uint metallicRoughnessTextureHandle;
    uint emissiveTextureHandle;
    uint occlusionTextureHandle;
    uint padding[ 3 ];
};

struct SceneConstantBuffer
{
    float4x4 projectionToWorld;
    float4   cameraPosition;
    float4   sunDirection;
    float4   sunColor;
    float4   ambientColor;
    float    reflectance;
    float    elapsedTime;
    uint     useShadowRays;
    uint     frameCount;
};

Texture2D    g_Textures[] : register( t0, space0 );
SamplerState g_Sampler : register( s1, space0 );

RaytracingAccelerationStructure     g_scene : register( t1, space1 );
StructuredBuffer<MaterialData>      g_Materials : register( t2, space1 );
RWTexture2D<float4>                 g_renderTarget : register( u3, space1 );
ByteAddressBuffer                   g_Vertices : register( t4, space1 );
ByteAddressBuffer                   g_Indices : register( t5, space1 );
StructuredBuffer<RayTraceMeshInfo>  g_meshInfo : register( t6, space1 );
RWTexture2D<float4>                 g_accumulationBuffer : register( u7, space1 );
ConstantBuffer<SceneConstantBuffer> g_sceneCB : register( b0, space1 );

static const uint   INVALID_TEXTURE_HANDLE  = 0xFFFFFFFF;
static const uint   MAX_RAY_RECURSION_DEPTH = 3;
static const float  InShadowRadiance        = 0.35f;
static const float4 BackgroundColor         = float4( 0.8f, 0.9f, 1.0f, 1.0f );

float3 HitWorldPosition( )
{
    return WorldRayOrigin( ) + RayTCurrent( ) * WorldRayDirection( );
}

uint3 Load3x16BitIndices( uint offsetBytes )
{
    const uint  dwordAlignedOffset = offsetBytes & ~3;
    const uint2 four16BitIndices   = g_Indices.Load2( dwordAlignedOffset );

    uint3 indices;
    if ( dwordAlignedOffset == offsetBytes )
    {
        indices.x = four16BitIndices.x & 0xffff;
        indices.y = ( four16BitIndices.x >> 16 ) & 0xffff;
        indices.z = four16BitIndices.y & 0xffff;
    }
    else
    {
        indices.x = ( four16BitIndices.x >> 16 ) & 0xffff;
        indices.y = four16BitIndices.y & 0xffff;
        indices.z = ( four16BitIndices.y >> 16 ) & 0xffff;
    }
    return indices;
}

uint3 Load3x32BitIndices( uint offsetBytes )
{
    uint3 indices;
    indices.x = g_Indices.Load( offsetBytes );
    indices.y = g_Indices.Load( offsetBytes + 4 );
    indices.z = g_Indices.Load( offsetBytes + 8 );
    return indices;
}

float2 GetUVAttribute( uint byteOffset )
{
    return asfloat( g_Vertices.Load2( byteOffset ) );
}

float3 GetFloat3Attribute( uint byteOffset )
{
    return asfloat( g_Vertices.Load3( byteOffset ) );
}

float4 GetFloat4Attribute( uint byteOffset )
{
    return asfloat( g_Vertices.Load4( byteOffset ) );
}

float4 SampleTextureWithGradients( uint textureHandle, float2 uv, float2 ddxUV, float2 ddyUV )
{
    if ( textureHandle != INVALID_TEXTURE_HANDLE )
    {
        return g_Textures[ NonUniformResourceIndex( textureHandle ) ].SampleGrad( g_Sampler, uv, ddxUV, ddyUV );
    }
    return float4( 1, 1, 1, 1 );
}

float3 UnpackNormal( float3 packedNormal )
{
    return packedNormal * 2.0 - 1.0;
}

float3 TransformTangentToWorld( float3 tangentNormal, float3 worldNormal, float3 worldTangent, float3 worldBitangent )
{
    worldNormal    = normalize( worldNormal );
    worldTangent   = normalize( worldTangent );
    worldBitangent = normalize( worldBitangent );

    float3x3 TBN = float3x3( worldTangent, worldBitangent, worldNormal );

    return normalize( mul( tangentNormal, TBN ) );
}
float DistributionGGX( float3 N, float3 H, float roughness )
{
    float a      = roughness * roughness;
    float a2     = a * a;
    float NdotH  = max( dot( N, H ), 0.0 );
    float NdotH2 = NdotH * NdotH;

    float num   = a2;
    float denom = ( NdotH2 * ( a2 - 1.0 ) + 1.0 );
    denom       = 3.14159265359 * denom * denom;

    return num / max( denom, 0.001 );
}

float GeometrySchlickGGX( float NdotV, float roughness )
{
    float r = ( roughness + 1.0 );
    float k = ( r * r ) / 8.0;

    float num   = NdotV;
    float denom = NdotV * ( 1.0 - k ) + k;

    return num / max( denom, 0.001 );
}

float GeometrySmith( float3 N, float3 V, float3 L, float roughness )
{
    float NdotV = max( dot( N, V ), 0.0 );
    float NdotL = max( dot( N, L ), 0.0 );
    float ggx2  = GeometrySchlickGGX( NdotV, roughness );
    float ggx1  = GeometrySchlickGGX( NdotL, roughness );

    return ggx1 * ggx2;
}

float3 FresnelSchlick( float cosTheta, float3 F0 )
{
    return F0 + ( 1.0 - F0 ) * pow( clamp( 1.0 - cosTheta, 0.0, 1.0 ), 5.0 );
}

float3 FresnelSchlickRoughness( float cosTheta, float3 F0, float roughness )
{
    return F0 + ( max( float3( 1.0 - roughness, 1.0 - roughness, 1.0 - roughness ), F0 ) - F0 ) * pow( clamp( 1.0 - cosTheta, 0.0, 1.0 ), 5.0 );
}

float4 TraceRadianceRay( float3 origin, float3 direction, uint currentRayRecursionDepth )
{
    if ( currentRayRecursionDepth >= MAX_RAY_RECURSION_DEPTH )
    {
        return float4( 0, 0, 0, 0 );
    }

    RayDesc rayDesc;
    rayDesc.Origin    = origin;
    rayDesc.Direction = direction;
    rayDesc.TMin      = 0.001f;
    rayDesc.TMax      = 10000.0f;

    RayPayload rayPayload;
    rayPayload.color          = float4( 0, 0, 0, 0 );
    rayPayload.recursionDepth = currentRayRecursionDepth + 1;
    rayPayload.SkipShading    = false;
    rayPayload.RayHitT        = FLT_MAX;

    TraceRay( g_scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES,
              ~0, // InstanceInclusionMask
              0,  // RayContributionToHitGroupIndex
              1,  // MultiplierForGeometryContributionToHitGroupIndex
              0,  // MissShaderIndex
              rayDesc, rayPayload );

    return rayPayload.color;
}

bool TraceShadowRay( float3 origin, float3 direction, float maxT )
{
    RayDesc rayDesc;
    rayDesc.Origin    = origin;
    rayDesc.Direction = direction;
    rayDesc.TMin      = 0.001f;
    rayDesc.TMax      = maxT - 0.001f;

    ShadowRayPayload shadowPayload;
    shadowPayload.hit = false;

    TraceRay( g_scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER,
              ~0, // InstanceInclusionMask
              0,  // RayContributionToHitGroupIndex
              1,  // MultiplierForGeometryContributionToHitGroupIndex
              1,  // MissShaderIndex for shadow rays
              rayDesc, shadowPayload );

    return shadowPayload.hit;
}

[ shader( "raygeneration" ) ] void MyRaygenShader( )
{
    uint2 pixelCoord = DispatchRaysIndex( ).xy;

    float3 origin, direction;
    GenerateCameraRay( pixelCoord, origin, direction, g_sceneCB.projectionToWorld, g_sceneCB.cameraPosition.xyz );

    float4 currentColor = TraceRadianceRay( origin, direction, 0 );

    uint   frameCount = g_sceneCB.frameCount;
    float4 finalColor = currentColor;

    if ( frameCount > 0 )
    {
        float4 accumulatedColor = g_accumulationBuffer[ pixelCoord ];
        float  alpha            = 1.0 / float( frameCount + 1 );
        finalColor              = lerp( accumulatedColor, currentColor, alpha );

        if ( frameCount > 64 )
        {
            alpha      = 1.0 / 64.0;
            finalColor = lerp( accumulatedColor, currentColor, alpha );
        }
    }

    g_accumulationBuffer[ pixelCoord ] = finalColor;
    g_renderTarget[ pixelCoord ]       = finalColor;
}

    [ shader( "closesthit" ) ] void MyClosestHitShader( inout RayPayload rayPayload, in BuiltInTriangleIntersectionAttributes attr )
{
    rayPayload.RayHitT = RayTCurrent( );
    if ( rayPayload.SkipShading )
    {
        return;
    }

    uint instanceId     = InstanceID( );
    uint primitiveIndex = PrimitiveIndex( );

    RayTraceMeshInfo meshInfo = g_meshInfo[ instanceId ];

    uint3 indices = Load3x32BitIndices( meshInfo.indexOffsetBytes + primitiveIndex * 3 * 4 );

    float3 barycentrics = float3( 1.0 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y );
    float2 uv0          = GetUVAttribute( meshInfo.uvAttributeOffsetBytes + indices.x * meshInfo.attributeStrideBytes );
    float2 uv1          = GetUVAttribute( meshInfo.uvAttributeOffsetBytes + indices.y * meshInfo.attributeStrideBytes );
    float2 uv2          = GetUVAttribute( meshInfo.uvAttributeOffsetBytes + indices.z * meshInfo.attributeStrideBytes );
    float2 uv           = barycentrics.x * uv0 + barycentrics.y * uv1 + barycentrics.z * uv2;
    float4 normal0_4    = GetFloat4Attribute( meshInfo.normalAttributeOffsetBytes + indices.x * meshInfo.attributeStrideBytes );
    float4 normal1_4    = GetFloat4Attribute( meshInfo.normalAttributeOffsetBytes + indices.y * meshInfo.attributeStrideBytes );
    float4 normal2_4    = GetFloat4Attribute( meshInfo.normalAttributeOffsetBytes + indices.z * meshInfo.attributeStrideBytes );

    float3 normal0 = normalize( normal0_4.xyz );
    float3 normal1 = normalize( normal1_4.xyz );
    float3 normal2 = normalize( normal2_4.xyz );

    float3   objectNormal     = normalize( normal0 * barycentrics.x + normal1 * barycentrics.y + normal2 * barycentrics.z );
    float3x4 objectToWorld3x4 = ObjectToWorld3x4( );
    float3x3 normalMatrix     = float3x3( objectToWorld3x4[ 0 ].xyz, objectToWorld3x4[ 1 ].xyz, objectToWorld3x4[ 2 ].xyz );
    float3   vertexNormal     = normalize( mul( objectNormal, normalMatrix ) );

    float4 tangent0_4    = GetFloat4Attribute( meshInfo.tangentAttributeOffsetBytes + indices.x * meshInfo.attributeStrideBytes );
    float4 tangent1_4    = GetFloat4Attribute( meshInfo.tangentAttributeOffsetBytes + indices.y * meshInfo.attributeStrideBytes );
    float4 tangent2_4    = GetFloat4Attribute( meshInfo.tangentAttributeOffsetBytes + indices.z * meshInfo.attributeStrideBytes );
    float3 tangent0      = normalize( tangent0_4.xyz );
    float3 tangent1      = normalize( tangent1_4.xyz );
    float3 tangent2      = normalize( tangent2_4.xyz );
    float3 objectTangent = normalize( tangent0 * barycentrics.x + tangent1 * barycentrics.y + tangent2 * barycentrics.z );
    float3 worldTangent  = normalize( mul( objectTangent, normalMatrix ) );

    float4 bitangent0_4    = GetFloat4Attribute( meshInfo.bitangentAttributeOffsetBytes + indices.x * meshInfo.attributeStrideBytes );
    float4 bitangent1_4    = GetFloat4Attribute( meshInfo.bitangentAttributeOffsetBytes + indices.y * meshInfo.attributeStrideBytes );
    float4 bitangent2_4    = GetFloat4Attribute( meshInfo.bitangentAttributeOffsetBytes + indices.z * meshInfo.attributeStrideBytes );
    float3 bitangent0      = normalize( bitangent0_4.xyz );
    float3 bitangent1      = normalize( bitangent1_4.xyz );
    float3 bitangent2      = normalize( bitangent2_4.xyz );
    float3 objectBitangent = normalize( bitangent0 * barycentrics.x + bitangent1 * barycentrics.y + bitangent2 * barycentrics.z );
    float3 worldBitangent  = normalize( mul( objectBitangent, normalMatrix ) );
    float4 pos0_4          = GetFloat4Attribute( meshInfo.positionAttributeOffsetBytes + indices.x * meshInfo.attributeStrideBytes );
    float4 pos1_4          = GetFloat4Attribute( meshInfo.positionAttributeOffsetBytes + indices.y * meshInfo.attributeStrideBytes );
    float4 pos2_4          = GetFloat4Attribute( meshInfo.positionAttributeOffsetBytes + indices.z * meshInfo.attributeStrideBytes );
    float3 pos0            = pos0_4.xyz;
    float3 pos1            = pos1_4.xyz;
    float3 pos2            = pos2_4.xyz;

    float3 worldPosition  = HitWorldPosition( );
    float3 triangleNormal = normalize( cross( pos2 - pos0, pos1 - pos0 ) );
    uint2  threadID       = DispatchRaysIndex( ).xy;
    float3 ddxOrigin, ddxDir, ddyOrigin, ddyDir;
    GenerateCameraRay( uint2( threadID.x + 1, threadID.y ), ddxOrigin, ddxDir, g_sceneCB.projectionToWorld, g_sceneCB.cameraPosition.xyz );
    GenerateCameraRay( uint2( threadID.x, threadID.y + 1 ), ddyOrigin, ddyDir, g_sceneCB.projectionToWorld, g_sceneCB.cameraPosition.xyz );

    float3 xOffsetPoint = RayPlaneIntersection( worldPosition, triangleNormal, ddxOrigin, ddxDir );
    float3 yOffsetPoint = RayPlaneIntersection( worldPosition, triangleNormal, ddyOrigin, ddyDir );

    float3 baryX = BarycentricCoordinates( xOffsetPoint, pos0, pos1, pos2 );
    float3 baryY = BarycentricCoordinates( yOffsetPoint, pos0, pos1, pos2 );

    float2       ddxUV         = ( baryX.x * uv0 + baryX.y * uv1 + baryX.z * uv2 ) - uv;
    float2       ddyUV         = ( baryY.x * uv0 + baryY.y * uv1 + baryY.z * uv2 ) - uv;
    uint         materialIndex = meshInfo.materialInstanceId;
    MaterialData material      = g_Materials[ materialIndex ];
    float4       albedo        = float4( 1, 1, 1, 1 );
    if ( material.albedoTextureHandle != INVALID_TEXTURE_HANDLE )
    {
        albedo = SampleTextureWithGradients( material.albedoTextureHandle, uv, ddxUV, ddyUV );
    }
    float3 worldNormal = vertexNormal;
    if ( material.normalTextureHandle != INVALID_TEXTURE_HANDLE )
    {
        float3 normalMapSample = SampleTextureWithGradients( material.normalTextureHandle, uv, ddxUV, ddyUV ).xyz;
        float3 tangentNormal   = UnpackNormal( normalMapSample );

        float  tangentW               = tangent0_4.w;
        float3 reconstructedBitangent = cross( vertexNormal, worldTangent ) * tangentW;
        worldNormal                   = TransformTangentToWorld( tangentNormal, vertexNormal, worldTangent, reconstructedBitangent );
    }
    worldNormal     = normalize( worldNormal );
    float metallic  = 0.0;
    float roughness = 0.5;
    if ( material.metallicRoughnessTextureHandle != INVALID_TEXTURE_HANDLE )
    {
        float4 metallicRoughnessSample = SampleTextureWithGradients( material.metallicRoughnessTextureHandle, uv, ddxUV, ddyUV );
        metallic                       = metallicRoughnessSample.b;
        roughness                      = metallicRoughnessSample.g;
    }
    roughness       = clamp( roughness, 0.045, 1.0 );
    float3 emissive = float3( 0, 0, 0 );
    if ( material.emissiveTextureHandle != INVALID_TEXTURE_HANDLE )
    {
        emissive = SampleTextureWithGradients( material.emissiveTextureHandle, uv, ddxUV, ddyUV ).rgb;
    }
    float ao = 1.0;
    if ( material.occlusionTextureHandle != INVALID_TEXTURE_HANDLE )
    {
        ao = SampleTextureWithGradients( material.occlusionTextureHandle, uv, ddxUV, ddyUV ).r;
    }
    float3 F0     = float3( 0.04, 0.04, 0.04 );
    F0            = lerp( F0, albedo.rgb, metallic );
    float3 V      = normalize( g_sceneCB.cameraPosition.xyz - worldPosition );
    float3 L      = normalize( g_sceneCB.sunDirection.xyz );
    float3 H      = normalize( V + L );
    float  shadow = 1.0;
    if ( g_sceneCB.useShadowRays )
    {
        float3  shadowOrigin = worldPosition + worldNormal * 0.01;
        RayDesc shadowRay;
        shadowRay.Origin    = shadowOrigin;
        shadowRay.Direction = L;
        shadowRay.TMin      = 0.01;
        shadowRay.TMax      = 1000.0;

        ShadowRayPayload shadowPayload;
        shadowPayload.hit = true;

        TraceRay( g_scene, RAY_FLAG_CULL_BACK_FACING_TRIANGLES | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH | RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_SKIP_CLOSEST_HIT_SHADER, 0xFF, 0, 1, 1,
                  shadowRay, shadowPayload );

        shadow = shadowPayload.hit ? 0.2 : 1.0;
    }
    float  NdotV = max( dot( worldNormal, V ), 0.0 );
    float  NdotL = max( dot( worldNormal, L ), 0.0 );
    float  NdotH = max( dot( worldNormal, H ), 0.0 );
    float  VdotH = max( dot( V, H ), 0.0 );
    float  D     = DistributionGGX( worldNormal, H, roughness );
    float  G     = GeometrySmith( worldNormal, V, L, roughness );
    float3 F     = FresnelSchlick( VdotH, F0 );

    float3 kS = F;
    float3 kD = float3( 1.0, 1.0, 1.0 ) - kS;
    kD *= 1.0 - metallic;

    float3 numerator        = D * G * F;
    float  denominator      = 4.0 * max( NdotV, 0.001 ) * max( NdotL, 0.001 );
    float3 specular         = numerator / max( denominator, 0.001 );
    float3 radiance         = g_sceneCB.sunColor.rgb * shadow;
    float3 directLighting   = ( kD * albedo.rgb / 3.14159265359 + specular ) * radiance * NdotL;
    float3 ambient          = g_sceneCB.ambientColor.rgb * albedo.rgb * ao;
    float3 indirectLighting = float3( 0, 0, 0 );
    if ( rayPayload.recursionDepth == 0 && metallic < 0.8 && roughness > 0.4 )
    {
        float3 tangent   = abs( worldNormal.x ) > 0.9 ? float3( 0, 1, 0 ) : float3( 1, 0, 0 );
        float3 bitangent = normalize( cross( worldNormal, tangent ) );
        tangent          = cross( bitangent, worldNormal );

        uint  pixelIndex = DispatchRaysIndex( ).x + DispatchRaysIndex( ).y * 1920;
        float seed       = frac( sin( float( pixelIndex ) * 12.9898 + g_sceneCB.elapsedTime * 78.233 ) * 43758.5453 );
        float phi        = 2.0 * 3.14159265359 * frac( seed );
        float cosTheta   = sqrt( frac( seed * 1.61803398875 ) );
        float sinTheta   = sqrt( 1.0 - cosTheta * cosTheta );

        float3 sampleDir   = float3( cos( phi ) * sinTheta, sin( phi ) * sinTheta, cosTheta );
        float3 indirectDir = normalize( sampleDir.x * tangent + sampleDir.y * bitangent + sampleDir.z * worldNormal );

        float4 indirectColor = TraceRadianceRay( worldPosition + worldNormal * 0.01, indirectDir, rayPayload.recursionDepth );
        float3 irradiance    = indirectColor.rgb;

        float3 F_indirect  = FresnelSchlickRoughness( NdotV, F0, roughness );
        float3 kS_indirect = F_indirect;
        float3 kD_indirect = 1.0 - kS_indirect;
        kD_indirect *= 1.0 - metallic;

        float3 diffuseIndirect = irradiance * albedo.rgb * kD_indirect;
        float  NdotL_indirect  = max( dot( worldNormal, indirectDir ), 0.0 );
        indirectLighting       = diffuseIndirect * ao * NdotL_indirect * 0.3;
    }
    float3 reflectionColor = float3( 0, 0, 0 );
    if ( rayPayload.recursionDepth < 2 )
    {
        float minRoughness = metallic > 0.5 ? 0.05 : 0.3;
        if ( roughness < minRoughness || metallic > 0.5 )
        {
            float3 R = reflect( -V, worldNormal );

            float4 reflectedRadiance = TraceRadianceRay( worldPosition + worldNormal * 0.01, R, rayPayload.recursionDepth );

            float3 F_reflection = FresnelSchlickRoughness( NdotV, F0, roughness );
            float  metalFactor  = metallic * metallic;
            reflectionColor     = reflectedRadiance.rgb * F_reflection * lerp( 0.3, 1.0, metalFactor ) * ao;
        }
    }
    float3 finalColor = directLighting + ambient + indirectLighting + reflectionColor + emissive;

    float3 x          = max( float3( 0, 0, 0 ), finalColor - 0.004 );
    float3 toneMapped = ( x * ( 6.2 * x + 0.5 ) ) / ( x * ( 6.2 * x + 1.7 ) + 0.06 );
    toneMapped        = saturate( toneMapped );

    rayPayload.color = float4( toneMapped, albedo.a );
}

[ shader( "miss" ) ] void MyMissShader( inout RayPayload payload )
{
    float3 unitDirection = normalize( WorldRayDirection( ) );
    float  t             = 0.5 * ( unitDirection.y + 1.0 );
    float3 skyColor      = lerp( float3( 1.0, 1.0, 1.0 ), float3( 0.5, 0.7, 1.0 ), t );
    payload.color        = float4( skyColor, 1.0 );
    payload.RayHitT      = FLT_MAX;
}

    [ shader( "anyhit" ) ] void MyAnyHitShader( inout RayPayload rayPayload, in BuiltInTriangleIntersectionAttributes attr )
{
    uint instanceId     = InstanceID( );
    uint primitiveIndex = PrimitiveIndex( );

    RayTraceMeshInfo meshInfo = g_meshInfo[ instanceId ];

    uint3 indices = Load3x32BitIndices( meshInfo.indexOffsetBytes + primitiveIndex * 3 * 4 );

    float3 barycentrics = float3( 1.0 - attr.barycentrics.x - attr.barycentrics.y, attr.barycentrics.x, attr.barycentrics.y );

    float2 uv0 = GetUVAttribute( meshInfo.uvAttributeOffsetBytes + indices.x * meshInfo.attributeStrideBytes );
    float2 uv1 = GetUVAttribute( meshInfo.uvAttributeOffsetBytes + indices.y * meshInfo.attributeStrideBytes );
    float2 uv2 = GetUVAttribute( meshInfo.uvAttributeOffsetBytes + indices.z * meshInfo.attributeStrideBytes );
    float2 uv  = barycentrics.x * uv0 + barycentrics.y * uv1 + barycentrics.z * uv2;

    uint         materialIndex = meshInfo.materialInstanceId;
    MaterialData material      = g_Materials[ materialIndex ];

    if ( material.albedoTextureHandle != INVALID_TEXTURE_HANDLE )
    {
        uint2  threadID = DispatchRaysIndex( ).xy;
        float3 ddxOrigin, ddxDir, ddyOrigin, ddyDir;
        GenerateCameraRay( uint2( threadID.x + 1, threadID.y ), ddxOrigin, ddxDir, g_sceneCB.projectionToWorld, g_sceneCB.cameraPosition.xyz );
        GenerateCameraRay( uint2( threadID.x, threadID.y + 1 ), ddyOrigin, ddyDir, g_sceneCB.projectionToWorld, g_sceneCB.cameraPosition.xyz );

        float4 pos0_4 = GetFloat4Attribute( meshInfo.positionAttributeOffsetBytes + indices.x * meshInfo.attributeStrideBytes );
        float4 pos1_4 = GetFloat4Attribute( meshInfo.positionAttributeOffsetBytes + indices.y * meshInfo.attributeStrideBytes );
        float4 pos2_4 = GetFloat4Attribute( meshInfo.positionAttributeOffsetBytes + indices.z * meshInfo.attributeStrideBytes );
        float3 pos0   = pos0_4.xyz;
        float3 pos1   = pos1_4.xyz;
        float3 pos2   = pos2_4.xyz;

        float3 worldPosition  = HitWorldPosition( );
        float3 triangleNormal = normalize( cross( pos2 - pos0, pos1 - pos0 ) );

        float3 xOffsetPoint = RayPlaneIntersection( worldPosition, triangleNormal, ddxOrigin, ddxDir );
        float3 yOffsetPoint = RayPlaneIntersection( worldPosition, triangleNormal, ddyOrigin, ddyDir );

        float3 baryX = BarycentricCoordinates( xOffsetPoint, pos0, pos1, pos2 );
        float3 baryY = BarycentricCoordinates( yOffsetPoint, pos0, pos1, pos2 );

        float2 ddxUV = ( baryX.x * uv0 + baryX.y * uv1 + baryX.z * uv2 ) - uv;
        float2 ddyUV = ( baryY.x * uv0 + baryY.y * uv1 + baryY.z * uv2 ) - uv;

        float4 albedo = SampleTextureWithGradients( material.albedoTextureHandle, uv, ddxUV, ddyUV );

        if ( albedo.a < 0.5 )
        {
            IgnoreHit( );
        }
    }
}

[ shader( "miss" ) ] void MyShadowMissShader( inout ShadowRayPayload payload ) { payload.hit = false; }
