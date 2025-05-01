/*
Den Of Iz - Game/Game Engine
Mesh Shader for Grass Rendering
*/

// Mesh Shader Attributes
#define MAX_VERTICES 64
#define MAX_PRIMITIVES 126
#define NUM_THREADS 128

// Constants for grass blade generation
#define BLADE_SEGMENTS 4
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
    float3 WorldPosition : TEXCOORD1;
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
    ) * 0.95; // Scale to fill most of the cell
}

// Generate a random height for the grass blade
float GetRandomHeight(float2 position, float2 id)
{
    float seed = dot(position + id, float2(127.1, 311.7));
    return lerp(0.7, 1.3, Random(float2(seed, seed * 0.5))) * HeightScale;
}

// Generate a random width for the grass blade
float GetRandomWidth(float2 position, float2 id)
{
    float seed = dot(position + id, float2(269.5, 183.3));
    return lerp(0.6, 1.4, Random(float2(seed, seed * 0.75))) * WidthScale;
}

// Generate a random bend direction and strength
float2 GetRandomBend(float2 position, float2 id)
{
    float2 seed = position + id;
    float angle = Random(seed) * 6.28; // 0 to 2π
    float strength = lerp(0.1, 0.4, Random(seed.yx));
    return float2(cos(angle), sin(angle)) * strength;
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
float3 ApplyWind(float3 vertexPosition, float heightRatio, float2 id, float3 basePos)
{
    // No wind at the base, increasing effect toward the tip
    if (heightRatio <= 0.05) return vertexPosition;
    
    // Apply different wind strength based on height (more at the top)
    float windStrength = pow(heightRatio, 1.5) * WindDirection.w;

    // Add some variation to make it look more natural
    float phaseOffset = Random(id) * 6.28; // Random phase offset (0 to 2π)
    float frequency = lerp(1.0, 2.0, Random(id.yx));
    float windEffect = sin(Time * frequency + phaseOffset + basePos.x * 0.2 + basePos.z * 0.3) * windStrength;
    
    // Secondary subtle motion
    float secondaryEffect = cos(Time * 0.7 + phaseOffset * 0.5 + basePos.z * 0.1) * windStrength * 0.3;

    // Apply wind in the specified direction
    vertexPosition.x += WindDirection.x * windEffect;
    vertexPosition.y += max(0, WindDirection.y * secondaryEffect) * 0.2; // Subtle vertical motion
    vertexPosition.z += WindDirection.z * windEffect;

    return vertexPosition;
}

// Calculate the LOD factor based on distance
float CalculateLodFactor(float3 position)
{
    // Approximation of distance to camera
    float distanceToCamera = length(position);
    return 1.0 - saturate(distanceToCamera / MaxDistance);
}

// Creates a 3D blade of grass with natural curve
void CreateGrassBlade(
    inout VertexOutput verts[MAX_VERTICES], 
    inout uint3 tris[MAX_PRIMITIVES],
    uint vertexStartIndex,
    uint triangleStartIndex,
    float3 basePosition,
    float height,
    float width,
    float2 bendDirection,
    float3 baseColor,
    float2 id
)
{
    // Generate blade vertices and triangles
    float3 positions[VERTICES_PER_BLADE];
    float3 normals[VERTICES_PER_BLADE];
    float2 uvs[VERTICES_PER_BLADE];
    
    // Base vertex at the bottom
    positions[0] = basePosition;
    normals[0] = float3(0, 1, 0);
    uvs[0] = float2(0.5, 0);
    
    // Calculate LOD based on distance
    float lodFactor = CalculateLodFactor(basePosition);
    
    // Create segments with increasing bend
    for (int segment = 0; segment < BLADE_SEGMENTS; segment++)
    {
        float heightRatio = (float)(segment + 1) / BLADE_SEGMENTS;
        float segmentHeight = height * heightRatio;
        
        // Width narrows as we go up
        float segmentWidth = width * (1.0 - heightRatio * 0.7);
        
        // Apply progressive bending for a natural curve
        float bendFactor = pow(heightRatio, 2.0); // Quadratic bend (more at the top)
        float3 bendOffset = float3(
            bendDirection.x * bendFactor * height * 0.15,
            0,
            bendDirection.y * bendFactor * height * 0.15
        );
        
        // Calculate position with progressive curve
        float3 segmentPos = basePosition + float3(0, segmentHeight, 0) + bendOffset;
        
        // Apply wind effect
        segmentPos = ApplyWind(segmentPos, heightRatio, id, basePosition);
        
        // Left and right points of the segment
        positions[segment * 2 + 1] = segmentPos + float3(-segmentWidth, 0, 0);
        positions[segment * 2 + 2] = segmentPos + float3(segmentWidth, 0, 0);
        
        // Calculate normals based on blade direction and bend
        float3 tangent = normalize(segmentPos - (segment > 0 ? 
            ((positions[segment * 2 - 1] + positions[segment * 2]) * 0.5) : 
            basePosition));
        
        float3 bitangent = float3(1, 0, 0); // Points left/right
        float3 normal = normalize(cross(tangent, bitangent));
        
        normals[segment * 2 + 1] = normal;
        normals[segment * 2 + 2] = normal;
        
        // UVs - vary horizontally and progress vertically
        uvs[segment * 2 + 1] = float2(0.0, heightRatio);
        uvs[segment * 2 + 2] = float2(1.0, heightRatio);
    }
    
    // Create triangles for the blade
    for (int segment = 0; segment < BLADE_SEGMENTS; segment++)
    {
        uint baseIndex = segment * 2 + 1;
        
        if (segment == 0)
        {
            // First segment connects to the base point
            tris[triangleStartIndex + segment * 2] = uint3(
                vertexStartIndex + 0,
                vertexStartIndex + baseIndex,
                vertexStartIndex + baseIndex + 1
            );
        }
        else
        {
            // Connect to previous segment
            tris[triangleStartIndex + segment * 2] = uint3(
                vertexStartIndex + baseIndex - 2,
                vertexStartIndex + baseIndex,
                vertexStartIndex + baseIndex - 1
            );
        }
        
        // Second triangle in the segment
        if (segment < BLADE_SEGMENTS - 1)
        {
            tris[triangleStartIndex + segment * 2 + 1] = uint3(
                vertexStartIndex + baseIndex,
                vertexStartIndex + baseIndex + 2,
                vertexStartIndex + baseIndex + 1
            );
        }
        else
        {
            // Last segment might have a different triangle structure
            tris[triangleStartIndex + segment * 2 + 1] = uint3(
                vertexStartIndex + baseIndex - 1,
                vertexStartIndex + baseIndex,
                vertexStartIndex + baseIndex + 1
            );
        }
    }
    
    // Apply vertex attributes
    for (int v = 0; v < VERTICES_PER_BLADE; v++)
    {
        uint vertexIndex = vertexStartIndex + v;
        float4 worldPos = float4(positions[v], 1.0);
        
        // Apply view-projection to get clip space position
        float4 clipPos = mul(worldPos, ViewProjection);

        // Set vertex outputs
        verts[vertexIndex].Position = clipPos;
        verts[vertexIndex].Normal = normals[v];
        verts[vertexIndex].TexCoord = uvs[v];
        verts[vertexIndex].WorldPosition = positions[v];
        
        // Apply color with gradient from bottom to top
        float heightFactor = v == 0 ? 0.7 : uvs[v].y;
        float colorBlend = lerp(0.7, 1.0, heightFactor); // Darker at bottom, full color at top
        verts[vertexIndex].Color = float4(baseColor * colorBlend, 1.0);
    }
}

// Basic mesh shader - generates realistic grass blades
[numthreads(NUM_THREADS, 1, 1)]
[outputtopology("triangle")]
void main(
    in uint3 DTid : SV_DispatchThreadID,
    out vertices VertexOutput verts[MAX_VERTICES],
    out indices uint3 tris[MAX_PRIMITIVES]
)
{
    // Calculate position in grid based on dispatch thread ID
    uint threadID = DTid.x;
    uint gridSize = 10; // Matches the dispatch size in the C++ code
    
    uint gridX = threadID % gridSize;
    uint gridY = threadID / gridSize;

    uint totalVertices = 0;
    uint totalTriangles = 0;
    uint bladesPerPatch = 50;

    if (gridX < gridSize && gridY < gridSize) {
        totalVertices = bladesPerPatch * VERTICES_PER_BLADE;
        totalTriangles = bladesPerPatch * PRIMITIVES_PER_BLADE;
    }
    SetMeshOutputCounts(min(totalVertices, MAX_VERTICES), min(totalTriangles, MAX_PRIMITIVES));

    if (totalVertices > 0) {
        // Calculate world space position for this grid cell
        float cellWorldSize = 1.0; // Size of each cell in world units
        float worldX = (float(gridX) - (gridSize * 0.5) + 0.5) * cellWorldSize;
        float worldZ = (float(gridY) - (gridSize * 0.5) + 0.5) * cellWorldSize;
        float3 gridCellCenter = float3(worldX, 0, worldZ);

        // Generate multiple grass blades within this grid cell
        for (uint blade = 0; blade < bladesPerPatch; blade++)
        {
            // Calculate a unique ID for this blade
            float2 bladeID = float2(DTid.x * 16 + blade, DTid.y * 16 + blade);

            // Random position within the cell
            float2 offset = GetRandomOffset(gridCellCenter.xz, bladeID);
            float3 bladePosition = float3(gridCellCenter.x + offset.x * cellWorldSize * 0.5,
                                          0,
                                          gridCellCenter.z + offset.y * cellWorldSize * 0.5);

            // Random properties for this blade
            float bladeHeight = GetRandomHeight(gridCellCenter.xz, bladeID);
            float bladeWidth = GetRandomWidth(gridCellCenter.xz, bladeID);
            float2 bladeBend = GetRandomBend(gridCellCenter.xz, bladeID);
            float3 bladeColor = GetRandomColor(gridCellCenter.xz, bladeID);

            // Create the grass blade vertices and triangles
            uint vertexStart = blade * VERTICES_PER_BLADE;
            uint triangleStart = blade * PRIMITIVES_PER_BLADE;

            CreateGrassBlade(
                verts, tris,
                vertexStart, triangleStart,
                bladePosition,
                bladeHeight,
                bladeWidth,
                bladeBend,
                bladeColor,
                bladeID
            );
        }
    }
}