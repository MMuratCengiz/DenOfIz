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
    float4x4 ViewProjection;
    float4x4 Model;
    
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

// Mesh shader payload (output)
struct MeshPayload
{
    uint NumVertices : SV_VERTEXCOUNT;
    uint NumPrimitives : SV_PRIMCOUNT;
    VertexOutput Vertices[MAX_VERTICES];
    uint3 Indices[MAX_PRIMITIVES];
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

// Main mesh shader function
[numthreads(NUM_THREADS, 1, 1)]
[outputtopology("triangle")]
void main(
    uint groupID : SV_GroupID,
    uint groupThreadID : SV_GroupThreadID,
    out MeshPayload output
)
{
    // Initialize output defaults
    output.NumVertices = 0;
    output.NumPrimitives = 0;
    
    // Calculate the position of this patch in the terrain grid
    const float gridSize = 40.0; // Match the model matrix scale from CPU side
    const float patchSize = gridSize / 20.0; // 20x20 grid as in DispatchMesh
    
    float2 patchPosition = float2(
        (float(groupID.x) - 10.0) * patchSize, 
        (float(groupID.y) - 10.0) * patchSize
    );
    
    // Determine number of grass blades in this patch based on density
    uint numBladesPerPatch = min(uint(DensityFactor * 8), 32); // Maximum 32 blades per patch
    
    // Check if this thread should create a grass blade
    if (groupThreadID < numBladesPerPatch)
    {
        // Calculate a unique ID for this grass blade
        float2 bladeId = float2(groupThreadID, groupID.x * 1000 + groupID.y);
        
        // Get a random position within the patch
        float2 randomOffset = GetRandomOffset(patchPosition, bladeId);
        float2 bladePosition = patchPosition + randomOffset * patchSize;
        
        // Create a transform from local grass blade to world space
        float3 worldPosition = float3(bladePosition.x, 0.0, bladePosition.y);
        
        // Calculate variation parameters
        float bladeHeight = GetRandomHeight(patchPosition, bladeId) * HeightScale;
        float bladeWidth = GetRandomWidth(patchPosition, bladeId) * WidthScale;
        float3 bladeColor = GetRandomColor(patchPosition, bladeId);
        
        // Apply LOD simplification based on distance
        float lodFactor = CalculateLodFactor(worldPosition);
        uint activeSegments = max(1, uint(lodFactor * BLADE_SEGMENTS));
        
        // Vertex index offset for this grass blade
        uint vertexOffset = output.NumVertices;
        
        // Create the base vertex (at ground level)
        output.Vertices[vertexOffset].Position = float4(worldPosition, 1.0);
        output.Vertices[vertexOffset].Normal = float3(0.0, 1.0, 0.0);
        output.Vertices[vertexOffset].TexCoord = float2(0.5, 0.0);
        output.Vertices[vertexOffset].Color = float4(bladeColor, 1.0);
        
        // Generate vertices for each segment
        for (uint segment = 0; segment < activeSegments; segment++)
        {
            // Height percentage along the blade
            float heightPercent = float(segment + 1) / float(BLADE_SEGMENTS);
            
            // Segment height
            float segmentHeight = bladeHeight * heightPercent;
            
            // Segment width (tapers toward the tip)
            float segmentWidth = bladeWidth * (1.0 - heightPercent);
            
            // Position at this segment's height
            float3 segmentPosition = worldPosition + float3(0.0, segmentHeight, 0.0);
            
            // Apply wind effect (stronger at the top)
            segmentPosition = ApplyWind(segmentPosition, heightPercent, bladeId);
            
            // Create left and right vertices for this segment
            for (uint point = 0; point < BLADE_POINTS_PER_SEGMENT; point++)
            {
                uint vertexIndex = vertexOffset + 1 + (segment * BLADE_POINTS_PER_SEGMENT) + point;
                
                // Position offset left/right
                float horizontalOffset = (point == 0) ? -segmentWidth : segmentWidth;
                float3 pointPosition = segmentPosition + float3(horizontalOffset, 0, 0);
                
                // Set vertex data
                output.Vertices[vertexIndex].Position = float4(pointPosition, 1.0);
                output.Vertices[vertexIndex].Normal = float3(0, 0.8, 0.2); // Slight angle upward
                output.Vertices[vertexIndex].TexCoord = float2((point == 0) ? 0.0 : 1.0, heightPercent);
                
                // Darken color slightly toward the base for depth perception
                float colorFactor = lerp(0.7, 1.0, heightPercent);
                output.Vertices[vertexIndex].Color = float4(bladeColor * colorFactor, 1.0);
            }
        }
        
        // Create triangles for the grass blade
        uint indexOffset = 0;
        for (uint segment = 0; segment < activeSegments; segment++)
        {
            uint baseIndex = vertexOffset; // Root vertex
            uint leftIndex = vertexOffset + 1 + (segment * BLADE_POINTS_PER_SEGMENT);
            uint rightIndex = leftIndex + 1;
            
            if (segment == 0)
            {
                // First segment connects to the base vertex
                output.Indices[output.NumPrimitives++] = uint3(baseIndex, leftIndex, rightIndex);
            }
            else
            {
                // Connect to previous segment
                uint prevLeftIndex = vertexOffset + 1 + ((segment - 1) * BLADE_POINTS_PER_SEGMENT);
                uint prevRightIndex = prevLeftIndex + 1;
                
                // Left triangle
                output.Indices[output.NumPrimitives++] = uint3(prevLeftIndex, leftIndex, prevRightIndex);
                
                // Right triangle
                output.Indices[output.NumPrimitives++] = uint3(prevRightIndex, leftIndex, rightIndex);
            }
        }
        
        // Update vertex count
        output.NumVertices = vertexOffset + 1 + (activeSegments * BLADE_POINTS_PER_SEGMENT);
    }
    
    // Apply world transformation and projection to all vertices
    for (uint i = 0; i < output.NumVertices; i++)
    {
        // Apply model and view-projection transforms
        output.Vertices[i].Position = mul(Model, output.Vertices[i].Position);
        output.Vertices[i].Position = mul(ViewProjection, output.Vertices[i].Position);
    }
}