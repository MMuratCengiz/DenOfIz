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

#ifndef ANALYTICPRIMITIVES_H
#define ANALYTICPRIMITIVES_H


#include "RaytracingShaderHelper.hlsli"

// Ref: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
bool SolveQuadraticEqn(float a, float b, float c, out float x0, out float x1)
{
    float discr = b * b - 4 * a * c;
    if (discr < 0) return false;
    else if (discr == 0) x0 = x1 = -0.5 * b / a;
    else {
        float q = (b > 0) ?
            -0.5 * (b + sqrt(discr)) :
            -0.5 * (b - sqrt(discr));
        x0 = q / a;
        x1 = c / q;
    }
    if (x0 > x1) swap(x0, x1);

    return true;
}

float3 CalculateNormalForARaySphereHit(in Ray ray, in float thit, float3 center)
{
    float3 hitPosition = ray.origin + thit * ray.direction;
    return normalize(hitPosition - center);
}

// Ref: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-sphere-intersection
bool SolveRaySphereIntersectionEquation(in Ray ray, out float tmin, out float tmax, in float3 center, in float radius)
{
    float3 L = ray.origin - center;
    float a = dot(ray.direction, ray.direction);
    float b = 2 * dot(ray.direction, L);
    float c = dot(L, L) - radius * radius;
    return SolveQuadraticEqn(a, b, c, tmin, tmax);
}

bool RaySphereIntersectionTest(in Ray ray, out float thit, out float tmax, out ProceduralPrimitiveAttributes attr, in float3 center = float3(0, 0, 0), in float radius = 1)
{
    float t0, t1;

    if (!SolveRaySphereIntersectionEquation(ray, t0, t1, center, radius)) return false;
    tmax = t1;

    if (t0 < RayTMin())
    {
        if (t1 < RayTMin()) return false;

        attr.normal = CalculateNormalForARaySphereHit(ray, t1, center);
        if (IsAValidHit(ray, t1, attr.normal))
        {
            thit = t1;
            return true;
        }
    }
    else
    {
        attr.normal = CalculateNormalForARaySphereHit(ray, t0, center);
        if (IsAValidHit(ray, t0, attr.normal))
        {
            thit = t0;
            return true;
        }

        attr.normal = CalculateNormalForARaySphereHit(ray, t1, center);
        if (IsAValidHit(ray, t1, attr.normal))
        {
            thit = t1;
            return true;
        }
    }
    return false;
}

bool RaySolidSphereIntersectionTest(in Ray ray, out float thit, out float tmax, in float3 center = float3(0, 0, 0), in float radius = 1)
{
    float t0, t1;

    if (!SolveRaySphereIntersectionEquation(ray, t0, t1, center, radius))
        return false;

    thit = max(t0, RayTMin());
    tmax = min(t1, RayTCurrent());

    return true;
}

bool RaySpheresIntersectionTest(in Ray ray, out float thit, out ProceduralPrimitiveAttributes attr)
{
    const int N = 3;
    float3 centers[N] =
    {
        float3(-0.3, -0.3, -0.3),
        float3(0.1, 0.1, 0.4),
        float3(0.35,0.35, 0.0)
    };
    float  radii[N] = { 0.6, 0.3, 0.15 };
    bool hitFound = false;

    thit = RayTCurrent();

    for (int i = 0; i < N; i++)
    {
        float _thit;
        float _tmax;
        ProceduralPrimitiveAttributes _attr = (ProceduralPrimitiveAttributes)0;
        if (RaySphereIntersectionTest(ray, _thit, _tmax, _attr, centers[i], radii[i]))
        {
            if (_thit < thit)
            {
                thit = _thit;
                attr = _attr;
                hitFound = true;
            }
        }
    }
    return hitFound;
}

// Ref: https://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
bool RayAABBIntersectionTest(Ray ray, float3 aabb[2], out float tmin, out float tmax)
{
    float3 tmin3, tmax3;
    int3 sign3 = ray.direction > 0;

    // TODO: handle cases where ray origin is within a slab
    const float FLT_INFINITY = 1.#INF;
    float3 invRayDirection;
    invRayDirection.x = ray.direction.x != 0 ? 1.0f / ray.direction.x : (ray.direction.x > 0 ? FLT_INFINITY : -FLT_INFINITY);
    invRayDirection.y = ray.direction.y != 0 ? 1.0f / ray.direction.y : (ray.direction.y > 0 ? FLT_INFINITY : -FLT_INFINITY);
    invRayDirection.z = ray.direction.z != 0 ? 1.0f / ray.direction.z : (ray.direction.z > 0 ? FLT_INFINITY : -FLT_INFINITY);

    tmin3.x = (aabb[1 - sign3.x].x - ray.origin.x) * invRayDirection.x;
    tmax3.x = (aabb[sign3.x].x - ray.origin.x) * invRayDirection.x;

    tmin3.y = (aabb[1 - sign3.y].y - ray.origin.y) * invRayDirection.y;
    tmax3.y = (aabb[sign3.y].y - ray.origin.y) * invRayDirection.y;

    tmin3.z = (aabb[1 - sign3.z].z - ray.origin.z) * invRayDirection.z;
    tmax3.z = (aabb[sign3.z].z - ray.origin.z) * invRayDirection.z;

    tmin = max(max(tmin3.x, tmin3.y), tmin3.z);
    tmax = min(min(tmax3.x, tmax3.y), tmax3.z);

    return tmax > tmin && tmax >= RayTMin() && tmin <= RayTCurrent();
}

bool RayAABBIntersectionTest(Ray ray, float3 aabb[2], out float thit, out ProceduralPrimitiveAttributes attr)
{
    float tmin, tmax;
    if (RayAABBIntersectionTest(ray, aabb, tmin, tmax))
    {
        if (tmin < RayTMin() || tmin > RayTCurrent())
            return false;

        thit = tmin;

        float3 hitPosition = ray.origin + thit * ray.direction;
        float3 distanceToBounds[2] = {
            abs(aabb[0] - hitPosition),
            abs(aabb[1] - hitPosition)
        };
        const float eps = 0.0001;
        if (distanceToBounds[0].x < eps) attr.normal = float3(-1, 0, 0);
        else if (distanceToBounds[0].y < eps) attr.normal = float3(0, -1, 0);
        else if (distanceToBounds[0].z < eps) attr.normal = float3(0, 0, -1);
        else if (distanceToBounds[1].x < eps) attr.normal = float3(1, 0, 0);
        else if (distanceToBounds[1].y < eps) attr.normal = float3(0, 1, 0);
        else if (distanceToBounds[1].z < eps) attr.normal = float3(0, 0, 1);

        return IsAValidHit(ray, thit, attr.normal);
    }
    return false;
}

#endif