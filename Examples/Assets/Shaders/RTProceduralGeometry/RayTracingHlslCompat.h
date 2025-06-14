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

#ifndef RAYTRACINGHLSLCOMPAT_H
#define RAYTRACINGHLSLCOMPAT_H

//**********************************************************************************************
//
// RaytracingHLSLCompat.h
//
// A header with shared definitions for C++ and HLSL source files.
//
//**********************************************************************************************

#ifdef HLSL
#include "HlslCompat.h"
#define ALIGN16_FIELD
#define ALIGN64_FIELD
#define ALIGN16_STRUCT
#define ALIGN64_STRUCT
#else
using namespace DirectX;
// Shader will use byte encoding to access vertex indices.
typedef uint16_t Index;

#define ALIGN16_FIELD alignas( 16 )
#define ALIGN64_FIELD alignas( 64 )
#define ALIGN16_STRUCT __declspec( align( 16 ) )
#define ALIGN64_STRUCT __declspec( align( 64 ) )
#endif

// Number of metaballs to use within an AABB.
#define N_METABALLS 3 // = {3, 5}

// Limitting calculations only to metaballs a ray intersects can speed up raytracing
// dramatically particularly when there is a higher number of metaballs used.
// Use of dynamic loops can have detrimental effects to performance for low iteration counts
// and outweighing any potential gains from avoiding redundant calculations.
// Requires: USE_DYNAMIC_LOOPS set to 1 to take effect.
#if N_METABALLS >= 5
#define USE_DYNAMIC_LOOPS 1
#define LIMIT_TO_ACTIVE_METABALLS 1
#else
#define USE_DYNAMIC_LOOPS 0
#define LIMIT_TO_ACTIVE_METABALLS 0
#endif

#define N_FRACTAL_ITERATIONS 4 // = <1,...>

// PERFORMANCE TIP: Set max recursion depth as low as needed
// as drivers may apply optimization strategies for low recursion depths.
#define MAX_RAY_RECURSION_DEPTH 3 // ~ primary rays + reflections + shadow rays from reflected geometry.

struct ProceduralPrimitiveAttributes
{
    XMFLOAT3 normal;
};

struct RayPayload
{
    XMFLOAT4 color;
    UINT     recursionDepth;
};

struct ShadowRayPayload
{
    bool hit;
};

struct SceneConstantBuffer
{
    XMMATRIX projectionToWorld;
    XMVECTOR cameraPosition;
    XMVECTOR lightPosition;
    XMVECTOR lightAmbientColor;
    XMVECTOR lightDiffuseColor;
    float    reflectance;
    float    elapsedTime;                 // Elapsed application time.
};

// Attributes per primitive type.
struct PrimitiveConstantBuffer
{
    XMFLOAT4 albedo;
    float    reflectanceCoef;
    float    diffuseCoef;
    float    specularCoef;
    float    specularPower;
    float    stepScale;

    float _pad0;
    float _pad1;
    float _pad2;
};

// Attributes per primitive instance.
struct PrimitiveInstanceConstantBuffer
{
    UINT instanceIndex;
    UINT primitiveType; // Procedural primitive type
    UINT _pad0;
    UINT _pad1;
};

struct LocalData
{
    PrimitiveConstantBuffer         materialCB;
    PrimitiveInstanceConstantBuffer aabbCB;
};

// Dynamic attributes per primitive instance.
struct PrimitiveInstancePerFrameBuffer
{
    XMMATRIX localSpaceToBottomLevelAS; // Matrix from local primitive space to bottom-level object space.
    XMMATRIX bottomLevelASToLocalSpace; // Matrix from bottom-level object space to local primitive space.
};

struct Vertex
{
    XMFLOAT4 position;
    XMFLOAT4 normal;
};

// Ray types traced in this sample.
namespace RayType
{
    enum Enum
    {
        Radiance = 0, // ~ Primary, reflected camera/view rays calculating color for each hit.
        Shadow,       // ~ Shadow/visibility rays, only testing for occlusion
        Count
    };
} // namespace RayType

namespace TraceRayParameters
{
    static const UINT InstanceMask = ~0; // Everything is visible.
    namespace HitGroup
    {
        static const UINT Offset[ RayType::Count ] = {
            0, // Radiance ray
            1  // Shadow ray
        };
        static const UINT GeometryStride = RayType::Count;
    } // namespace HitGroup
    namespace MissShader
    {
        static const UINT Offset[ RayType::Count ] = {
            0, // Radiance ray
            1  // Shadow ray
        };
    }
} // namespace TraceRayParameters

// From: http://blog.selfshadow.com/publications/s2015-shading-course/hoffman/s2015_pbs_physics_math_slides.pdf
static const XMFLOAT4 ChromiumReflectance = XMFLOAT4( 0.549f, 0.556f, 0.554f, 1.0f );

static const XMFLOAT4 BackgroundColor  = XMFLOAT4( 0.8f, 0.9f, 1.0f, 1.0f );
static const float    InShadowRadiance = 0.35f;

namespace AnalyticPrimitive
{
    enum Enum
    {
        AABB = 0,
        Spheres,
        Count
    };
} // namespace AnalyticPrimitive

namespace VolumetricPrimitive
{
    enum Enum
    {
        Metaballs = 0,
        Count
    };
} // namespace VolumetricPrimitive

namespace SignedDistancePrimitive
{
    enum Enum
    {
        MiniSpheres = 0,
        IntersectedRoundCube,
        SquareTorus,
        TwistedTorus,
        Cog,
        Cylinder,
        FractalPyramid,
        Count
    };
} // namespace SignedDistancePrimitive

#endif // RAYTRACINGHLSLCOMPAT_H
