/*
Den Of Iz - Game/Game Engine
Mesh Shader for Grass Rendering
*/

// Mesh Shader Attributes - using max possible limits for mesh shader
#define MAX_VERTICES 256  // Maximum vertices per thread group
#define MAX_PRIMITIVES 256 // Maximum primitives per thread group
#define NUM_THREADS 128   // Thread count per thread group

// Constants for grass blade generation
// Optimized to use fewer vertices per blade while still maintaining quality
#define BLADE_SEGMENTS 3   // Reduced segments for efficiency
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
    float TerrainScale;
    float TerrainHeight;
    float TerrainRoughness;
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

// Advanced random number generator with better distribution
float RandomImproved(float2 seed, float offset)
{
    return frac(sin(dot(seed + offset, float2(12.9898, 78.233))) * 43758.5453);
}

// Generate a random height for the grass blade with biased distribution
float GetRandomHeight(float2 position, float2 id)
{
    // Create more variation in heights with clustering for natural look
    float seed = dot(position + id, float2(127.1, 311.7));
    
    // Use different distributions based on position for natural clusters
    float r = RandomImproved(float2(seed, seed * 0.5), 0.0);
    
    // Bias toward medium heights with some short and tall blades
    // This creates a more natural distribution than uniform random
    float height;
    if (r < 0.7) {
        // 70% are medium height (looks more natural with majority similar height)
        height = lerp(0.75, 1.05, RandomImproved(id, 0.3));
    } else if (r < 0.9) {
        // 20% are taller
        height = lerp(1.05, 1.35, RandomImproved(id, 0.6));
    } else {
        // 10% are shorter
        height = lerp(0.5, 0.75, RandomImproved(id, 0.9));
    }
    
    return height * HeightScale;
}

// Generate a random width for the grass blade with natural distribution
float GetRandomWidth(float2 position, float2 id)
{
    float seed = dot(position + id, float2(269.5, 183.3));
    
    // Bias toward thinner blades for more delicate appearance
    float r = pow(RandomImproved(float2(seed, seed * 0.75), 0.42), 1.5);
    return lerp(0.5, 1.2, r) * WidthScale;
}

// Generate a random bend direction and strength with more natural clusters
float2 GetRandomBend(float2 position, float2 id)
{
    // Create gradual changes in bend direction by including position
    float2 seed = position * 0.1 + id;
    
    // Create a natural flow pattern - grass tends to bend in similar directions in clusters
    float noise1 = sin(position.x * 0.05 + position.y * 0.07) * 0.5;
    float noise2 = cos(position.x * 0.06 - position.y * 0.04) * 0.5;
    
    float baseAngle = noise1 + noise2;
    float angleVariation = RandomImproved(seed, 0.27) * 0.4; // Small variation from base angle
    float angle = baseAngle + angleVariation;
    
    // Vary strength based on different factors
    float strength = lerp(0.1, 0.45, RandomImproved(seed.yx, 0.75));
    return float2(cos(angle), sin(angle)) * strength;
}

// Generate random color variation with natural clustering
float3 GetRandomColor(float2 position, float2 id)
{
    float3 variation;
    float seed = dot(position + id, float2(127.1, 269.5));
    
    // Create subtle color gradients based on position
    float gradientX = sin(position.x * 0.02) * 0.5 + 0.5;
    float gradientZ = cos(position.y * 0.025) * 0.5 + 0.5;
    
    // Base variation on position for natural color clusters
    variation.x = lerp(-GrassColorVariation.x, GrassColorVariation.x, 
                 RandomImproved(float2(seed, 0.0), gradientX));
    variation.y = lerp(-GrassColorVariation.y, GrassColorVariation.y, 
                 RandomImproved(float2(seed, 1.0), gradientZ));
    variation.z = lerp(-GrassColorVariation.z, GrassColorVariation.z, 
                 RandomImproved(float2(seed, 2.0), gradientX * gradientZ));
    
    // Make some areas slightly yellower (dry patches)
    if (gradientX * gradientZ > 0.7 && RandomImproved(id, 0.33) > 0.7) {
        variation.x += 0.1;
        variation.y += 0.05;
        variation.z -= 0.05;
    }
    
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

// Calculate terrain height based on position
float SampleTerrainHeight(float2 position)
{
    // Create procedural terrain with different noise layers
    float2 scaledPos = position * 0.05;
    
    // Base large hills
    float height = sin(scaledPos.x * 1.5) * cos(scaledPos.y * 1.5) * 0.5;
    
    // Medium details
    height += sin(scaledPos.x * 3.0 + 0.5) * sin(scaledPos.y * 4.0) * 0.25;
    
    // Small details (only if roughness is high enough)
    if (TerrainRoughness > 0.4) {
        height += sin(scaledPos.x * 8.0 + scaledPos.y * 7.0) * sin(scaledPos.y * 9.0 - scaledPos.x * 3.0) * 0.15 * ((TerrainRoughness - 0.4) / 0.6);
    }
    
    // Add some small noise for very local variations
    float localSeed = dot(position, float2(127.1, 311.7));
    float localNoise = frac(sin(localSeed) * 43758.5453) * 0.1 - 0.05;
    height += localNoise * TerrainRoughness;
    
    // Apply overall scaling
    height *= TerrainHeight * TerrainScale;
    
    // Ensure minimum height is 0 to prevent grass below ground
    return max(0.0, height);
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
    in uint3 Gid : SV_GroupID,
    in uint3 GTid : SV_GroupThreadID,
    in uint GIndex : SV_GroupIndex,
    out vertices VertexOutput verts[MAX_VERTICES],
    out indices uint3 tris[MAX_PRIMITIVES]
)
{
    // Calculate position in grid based on dispatch thread ID
    uint threadID = DTid.x;
    uint gridSize = 64; // Matches the increased dispatch size in the C++ code (64x64)
    
    uint gridX = threadID % gridSize;
    uint gridY = threadID / gridSize;

    uint totalVertices = 0;
    uint totalTriangles = 0;
    
    // Calculate optimal blades per patch:
    // MAX_VERTICES / VERTICES_PER_BLADE = max possible blades
    // But need to ensure we don't exceed MAX_PRIMITIVES
    uint maxBladesByVertices = MAX_VERTICES / VERTICES_PER_BLADE;
    uint maxBladesByPrimitives = MAX_PRIMITIVES / PRIMITIVES_PER_BLADE;
    uint bladesPerPatch = min(maxBladesByVertices, maxBladesByPrimitives);
    
    // Additional safety check for edge cases, but allow more blades
    // With reduced segments, we can fit more blades per patch
    bladesPerPatch = min(bladesPerPatch, 42); // Increased safe upper limit
    
    if (gridX < gridSize && gridY < gridSize) {
        totalVertices = bladesPerPatch * VERTICES_PER_BLADE;
        totalTriangles = bladesPerPatch * PRIMITIVES_PER_BLADE;
    }

    SetMeshOutputCounts(totalVertices, totalTriangles);
    GroupMemoryBarrierWithGroupSync();

    if (totalVertices > 0) {
        // Calculate world space position for this grid cell
        // Use smaller cell size for more precise positioning and denser patches
        float cellWorldSize = 0.65; // Significantly reduced cell size for tighter packing
        float worldX = (float(gridX) - (gridSize * 0.5) + 0.5) * cellWorldSize;
        float worldZ = (float(gridY) - (gridSize * 0.5) + 0.5) * cellWorldSize;
        float3 gridCellCenter = float3(worldX, 0, worldZ);
        
        // Use density factor to control blade density
        float density = DensityFactor * 0.1; // Scale factor for density
        
        // Generate multiple grass blades within this grid cell with improved positioning
        for (uint blade = 0; blade < bladesPerPatch; blade++)
        {
            // Calculate a unique ID for this blade with much more variation
            // This prevents visible patterns in the grass
            float2 bladeID = float2(
                DTid.x * 73.926 + blade * 0.317 + sin(blade * 0.59) * 12.567,
                DTid.y * 52.713 + blade * 0.727 + cos(blade * 0.83) * 17.391
            );
            
            // Calculate a deterministic but complex offset to create a natural distribution
            // Improved random distribution using blue noise-like patterns
            float angle = frac(sin(dot(bladeID, float2(12.9898, 78.233))) * 43758.5453) * 6.28318;
            float radius = sqrt(frac(sin(dot(bladeID, float2(53.7842, 28.7456))) * 87234.7531));
            
            // Jitter offset to break uniformity
            float2 jitter = float2(
                sin(bladeID.x * 0.1 + bladeID.y * 0.13) * 0.1,
                cos(bladeID.y * 0.07 + bladeID.x * 0.19) * 0.1
            );
            
            // Create offset with improved spiral-like distribution for natural look
            float2 offset = float2(
                cos(angle) * radius + jitter.x,
                sin(angle) * radius + jitter.y
            );
            
            // Extend range slightly to fill gaps between cells
            offset *= 1.15;
            
            // Calculate world position for this blade
            float2 worldPos2D = float2(
                gridCellCenter.x + offset.x * cellWorldSize,
                gridCellCenter.z + offset.y * cellWorldSize
            );
            
            // Sample terrain height at this position
            float terrainHeight = SampleTerrainHeight(worldPos2D);
            
            // Position the blade at the calculated position with terrain height
            float3 bladePosition = float3(
                worldPos2D.x,
                terrainHeight, // Use terrain height instead of 0
                worldPos2D.y
            );

            // Random properties for this blade, adjusted for terrain height
            // Taller grass in lower areas, shorter on peaks
            float heightModifier = 1.0 - saturate(terrainHeight / (TerrainHeight * TerrainScale * 0.75));
            heightModifier = lerp(0.8, 1.2, heightModifier); // Range from 0.8 (peaks) to 1.2 (valleys)
            
            float bladeHeight = GetRandomHeight(gridCellCenter.xz, bladeID) * heightModifier;
            float bladeWidth = GetRandomWidth(gridCellCenter.xz, bladeID);
            
            // More bend at higher positions due to "wind"
            float bendModifier = saturate(terrainHeight / (TerrainHeight * TerrainScale * 0.5));
            bendModifier = lerp(1.0, 1.3, bendModifier); // Range from 1.0 (valleys) to 1.3 (peaks)
            float2 bladeBend = GetRandomBend(gridCellCenter.xz, bladeID) * bendModifier;
            
            // Color variation based on terrain - slightly yellower at peaks, greener in valleys
            float3 bladeColor = GetRandomColor(gridCellCenter.xz, bladeID);
            if (terrainHeight > TerrainHeight * TerrainScale * 0.7) {
                // Shift color toward yellow for higher areas
                float heightFactor = (terrainHeight - TerrainHeight * TerrainScale * 0.7) / (TerrainHeight * TerrainScale * 0.3);
                bladeColor.x += heightFactor * 0.15; // More red
                bladeColor.y += heightFactor * 0.05; // More green
                bladeColor.z -= heightFactor * 0.1;  // Less blue
            }

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