/*
Den Of Iz - Game/Game Engine
Mesh Shader for Grass Rendering
*/

// Mesh Shader Attributes
#define MAX_VERTICES 64
#define MAX_PRIMITIVES 126
#define NUM_THREADS 128

// Constants for grass blade generation
#define BLADE_SEGMENTS 3
#define BLADE_POINTS_PER_SEGMENT 2
#define VERTICES_PER_BLADE ((BLADE_SEGMENTS * BLADE_POINTS_PER_SEGMENT) + 1) // +1 for the base point
#define PRIMITIVES_PER_BLADE (BLADE_SEGMENTS * 2) // Triangles per blade

// Constants shared with the CPU side
cbuffer GrassConstants : register(b0, space0)
{
    row_major float4x4 ViewProjection;
    row_major float4x4 Model;

    float4 WindDirection; // xyz = direction, w = strength
    float4 GrassColor;
    float4 GrassColorVariation;

    float Time;
    float DensityFactor;
    float HeightScale;
    float WidthScale;

    float MaxDistance;
    float3 Padding;
};

// Output vertex structure
struct VertexOutput
{
    float4 Position : SV_Position;
    float3 Normal : NORMAL;
    float2 TexCoord : TEXCOORD0;
    float4 Color : COLOR0;
};

// Random number generator
float Random(float2 seed)
{
    return frac(sin(dot(seed, float2(12.9898, 78.233))) * 43758.5453);
}

// Generate a random position within the patch
float2 GetRandomOffset(float2 position, float2 id)
{
    float2 seed = position + id;
    return float2(
        Random(seed) * 2.0 - 1.0,
        Random(seed.yx) * 2.0 - 1.0
    ) * 0.5; // Scale down the offset
}

// Generate a random height for the grass blade
float GetRandomHeight(float2 position, float2 id)
{
    float seed = dot(position + id, float2(127.1, 311.7));
    return lerp(0.5, 1.0, Random(float2(seed, seed * 0.5)));
}

// Generate a random width for the grass blade
float GetRandomWidth(float2 position, float2 id)
{
    float seed = dot(position + id, float2(269.5, 183.3));
    return lerp(0.7, 1.3, Random(float2(seed, seed * 0.75)));
}

// Generate random color variation
float3 GetRandomColor(float2 position, float2 id)
{
    float3 variation;
    float seed = dot(position + id, float2(127.1, 269.5));

    variation.x = lerp(-GrassColorVariation.x, GrassColorVariation.x, Random(float2(seed, 0.0)));
    variation.y = lerp(-GrassColorVariation.y, GrassColorVariation.y, Random(float2(seed, 1.0)));
    variation.z = lerp(-GrassColorVariation.z, GrassColorVariation.z, Random(float2(seed, 2.0)));

    return GrassColor.xyz + variation;
}

// Calculate wind effect based on time and position
float3 ApplyWind(float3 vertexPosition, float height, float2 id)
{
    // Apply different wind strength based on height
    float windStrength = height * WindDirection.w;

    // Add some variation to make it look more natural
    float phaseOffset = Random(id) * 6.28; // Random phase offset (0 to 2π)
    float windEffect = sin(Time * 2.0 + phaseOffset) * windStrength;

    // Apply wind in the specified direction
    vertexPosition.x += WindDirection.x * windEffect;
    vertexPosition.y += abs(WindDirection.y * windEffect) * 0.2; // Less vertical movement
    vertexPosition.z += WindDirection.z * windEffect;

    return vertexPosition;
}

// Calculate the LOD factor based on distance
float CalculateLodFactor(float3 position)
{
    // This could be improved to use actual camera position instead of view projection matrix
    float distanceToCamera = length(position);
    return 1.0 - saturate(distanceToCamera / MaxDistance);
}

// Basic mesh shader - extremely simplified version
[numthreads(NUM_THREADS, 1, 1)]
[outputtopology("triangle")]
void main(
    in uint3 DTid : SV_DispatchThreadID,
    out vertices VertexOutput verts[MAX_VERTICES],
    out indices uint3 tris[MAX_PRIMITIVES]
)
{
    // Always set the output counts first
    SetMeshOutputCounts(8, 12);

    // Calculate position in grid based on dispatch thread ID
    // Convert to grid coordinates (0 to 19 in X and Y)
    uint gridX = DTid.x / NUM_THREADS;
    uint gridY = DTid.y;

    // Convert to world position (-10 to 10 in both X and Z)
    float gridSize = 40.0;
    float cellSize = gridSize / 20.0;
    float offsetX = (float(gridX) - 10.0 + 0.5) * cellSize;
    float offsetZ = (float(gridY) - 10.0 + 0.5) * cellSize;

    // Simple plane with grass blades
    float3 positions[8] = {
        // Base vertices (bottom) - make the patch much larger
        float3(-0.5, 0, -0.5),
        float3(0.5, 0, -0.5),
        float3(0.5, 0, 0.5),
        float3(-0.5, 0, 0.5),

        // Top vertices (grass tips)
        float3(-0.25, 1.5, -0.25),
        float3(0.25, 1.5, -0.25),
        float3(0.25, 1.5, 0.25),
        float3(-0.25, 1.5, 0.25)
    };

    float2 uvs[8] = {
        // Bottom UVs
        float2(0, 0),
        float2(1, 0),
        float2(1, 0),
        float2(0, 0),

        // Top UVs
        float2(0.25, 1),
        float2(0.75, 1),
        float2(0.75, 1),
        float2(0.25, 1)
    };

    float3 normals[8] = {
        // Bottom normals (up)
        float3(0, 1, 0),
        float3(0, 1, 0),
        float3(0, 1, 0),
        float3(0, 1, 0),

        // Top normals (angled outward)
        float3(-0.2, 0.8, -0.2),
        float3(0.2, 0.8, -0.2),
        float3(0.2, 0.8, 0.2),
        float3(-0.2, 0.8, 0.2)
    };

    // Indices for the triangles
    uint3 indices[12] = {
        // Bottom square (2 triangles)
        uint3(0, 1, 2),
        uint3(0, 2, 3),

        // Sides (8 triangles)
        uint3(0, 4, 1),
        uint3(1, 4, 5),
        uint3(1, 5, 2),
        uint3(2, 5, 6),
        uint3(2, 6, 3),
        uint3(3, 6, 7),
        uint3(3, 7, 0),
        uint3(0, 7, 4),

        // Top square (2 triangles)
        uint3(4, 6, 5),
        uint3(4, 7, 6)
    };

    // Generate random color variation based on grid position
    float3 colorVariation;
    colorVariation.x = sin(gridX * 0.1) * GrassColorVariation.x;
    colorVariation.y = cos(gridY * 0.1) * GrassColorVariation.y;
    colorVariation.z = sin((gridX + gridY) * 0.05) * GrassColorVariation.z;

    // Apply wind effect
    float windEffect = sin(Time + gridX * 0.2 + gridY * 0.3) * WindDirection.w;
    float windX = WindDirection.x * windEffect * 0.3;
    float windZ = WindDirection.z * windEffect * 0.3;

    // Create the vertices
    for (int i = 0; i < 8; i++) {
        float3 pos = positions[i];

        // Apply wind to upper vertices only (indices 4-7)
        if (i >= 4) {
            pos.x += windX * pos.y;
            pos.z += windZ * pos.y;
        }

        // Apply grid offset
        pos.x = pos.x * cellSize + offsetX;
        pos.z = pos.z * cellSize + offsetZ;

        // Transform to world space (no need to apply model matrix as we've manually positioned)
        float4 worldPos = float4(pos, 1.0);

        // Apply view-projection
        float4 clipPos = mul(ViewProjection, worldPos);

        // Set vertex
        verts[i].Position = clipPos;
        verts[i].Normal = normals[i];
        verts[i].TexCoord = uvs[i];

        // Apply color with variation
        float3 vertColor = GrassColor.xyz + colorVariation;

        // Darken lower vertices for ground effect
        float heightFactor = (i >= 4) ? 1.0 : 0.6;
        verts[i].Color = float4(vertColor * heightFactor, 1.0);
    }

    // Create the triangles
    for (int j = 0; j < 12; j++) {
        tris[j] = indices[j];
    }
}