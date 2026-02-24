//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************
// Adapted from DirectX-Graphics-Samples D3D12RaytracingMiniEngineSample

#ifndef MODEL_VIEWER_RAYTRACING_H
#define MODEL_VIEWER_RAYTRACING_H

#ifdef HLSL
struct RayPayload
{
    float4 color;
    uint   recursionDepth;
    bool   SkipShading;
    float  RayHitT;
};

struct ShadowRayPayload
{
    bool hit;
};

static const float FLT_MAX = asfloat( 0x7F7FFFFF );

inline void GenerateCameraRay( uint2 index, out float3 origin, out float3 direction, float4x4 projectionToWorld, float3 cameraPosition )
{
    float2 xy        = index + 0.5f; // center in the middle of the pixel
    float2 screenPos = xy / DispatchRaysDimensions( ).xy * 2.0 - 1.0;

    // Invert Y for DirectX-style coordinates
    screenPos.y = -screenPos.y;

    // Unproject into a ray
    float4 world = mul( float4( screenPos, 0, 1 ), projectionToWorld );
    world.xyz /= world.w;

    origin    = cameraPosition;
    direction = normalize( world.xyz - origin );
}

inline float3 BarycentricCoordinates( float3 pt, float3 v0, float3 v1, float3 v2 )
{
    float3 e0    = v1 - v0;
    float3 e1    = v2 - v0;
    float3 e2    = pt - v0;
    float  d00   = dot( e0, e0 );
    float  d01   = dot( e0, e1 );
    float  d11   = dot( e1, e1 );
    float  d20   = dot( e2, e0 );
    float  d21   = dot( e2, e1 );
    float  denom = 1.0 / ( d00 * d11 - d01 * d01 );
    float  v     = ( d11 * d20 - d01 * d21 ) * denom;
    float  w     = ( d00 * d21 - d01 * d20 ) * denom;
    float  u     = 1.0 - v - w;
    return float3( u, v, w );
}

inline float3 RayPlaneIntersection( float3 planeOrigin, float3 planeNormal, float3 rayOrigin, float3 rayDirection )
{
    float t = dot( -planeNormal, rayOrigin - planeOrigin ) / dot( planeNormal, rayDirection );
    return rayOrigin + rayDirection * t;
}

inline void AntiAliasSpecular( inout float3 texNormal, inout float gloss )
{
    float normalLenSq  = dot( texNormal, texNormal );
    float invNormalLen = rsqrt( normalLenSq );
    texNormal *= invNormalLen;
    gloss = lerp( 1, gloss, rcp( invNormalLen ) );
}

// Apply fresnel to modulate the specular albedo
inline void FSchlick( inout float3 specular, inout float3 diffuse, float3 lightDir, float3 halfVec )
{
    float fresnel = pow( 1.0 - saturate( dot( lightDir, halfVec ) ), 5.0 );
    specular      = lerp( specular, 1, fresnel );
    diffuse       = lerp( diffuse, 0, fresnel );
}

inline float3 ApplyLightCommon( float3 diffuseColor,  // Diffuse albedo
                                float3 specularColor, // Specular albedo
                                float  specularMask,  // Where is it shiny or dingy?
                                float  gloss,         // Specular power
                                float3 normal,        // World-space normal
                                float3 viewDir,       // World-space vector from eye to point
                                float3 lightDir,      // World-space vector from point to light
                                float3 lightColor     // Radiance of directional light
)
{
    float3 halfVec = normalize( lightDir - viewDir );
    float  nDotH   = saturate( dot( halfVec, normal ) );

    FSchlick( specularColor, diffuseColor, lightDir, halfVec );

    float specularFactor = specularMask * pow( nDotH, gloss ) * ( gloss + 2 ) / 8;

    float nDotL = saturate( dot( normal, lightDir ) );

    return nDotL * lightColor * ( diffuseColor + specularFactor * specularColor );
}

#endif // HLSL

#endif // MODEL_VIEWER_RAYTRACING_H
