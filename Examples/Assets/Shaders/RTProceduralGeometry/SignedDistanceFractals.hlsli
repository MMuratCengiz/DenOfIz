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

#ifndef SIGNEDDISTANCEFRACTALS_H
#define SIGNEDDISTANCEFRACTALS_H

#include "RaytracingShaderHelper.hlsli"
#include "SignedDistancePrimitives.hlsli"

// More info here http://blog.hvidtfeldts.net/index.php/2011/08/distance-estimated-3d-fractals-iii-folding-space/
float sdFractalPyramid(in float3 position, float3 h, in float Scale = 2.0f)
{
    float a = h.z * h.y / h.x;
    float3 v1 = float3(0, h.z, 0);
    float3 v2 = float3(-a, 0, a);
    float3 v3 = float3(a, 0, -a);
    float3 v4 = float3(a, 0, a);
    float3 v5 = float3(-a, 0, -a);

    int n = 0;
    for (n = 0; n < N_FRACTAL_ITERATIONS; n++)
    {
        float dist, d;
        float3 v;
        v = v1; dist = length_toPow2(position - v1); 
        d = length_toPow2(position - v2); if (d < dist) { v = v2; dist = d; }
        d = length_toPow2(position - v3); if (d < dist) { v = v3; dist = d; }
        d = length_toPow2(position - v4); if (d < dist) { v = v4; dist = d; }
        d = length_toPow2(position - v5); if (d < dist) { v = v5; dist = d; }

        position = Scale * position - v * (Scale - 1.0);
    }
    float distance = sdPyramid(position, h);

    return distance * pow(Scale, float(-n));
}
#endif