#include <DenOfIzGraphics/Data/PrimitiveBuilder.h>

using namespace DenOfIz;

// Helper function to add a vertex with attributes based on BuildDesc
void AddVertex(std::vector<float> &vertices, const BitSet<BuildDesc> &desc, float x, float y, float z, float nx = 0.0f, float ny = 0.0f, float nz = 0.0f, float tx = 0.0f,
               float ty = 0.0f, float tz = 0.0f, float bx = 0.0f, float by = 0.0f, float bz = 0.0f, float u = 0.0f, float v = 0.0f)
{
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(z);
    vertices.push_back(1.0f);

    if ( desc.IsSet(BuildDesc::Normal) )
    {
        vertices.push_back(nx);
        vertices.push_back(ny);
        vertices.push_back(nz);
    }

    if ( desc.IsSet(BuildDesc::Tangent) )
    {
        vertices.push_back(tx);
        vertices.push_back(ty);
        vertices.push_back(tz);
    }

    if ( desc.IsSet(BuildDesc::Bitangent) )
    {
        vertices.push_back(bx);
        vertices.push_back(by);
        vertices.push_back(bz);
    }

    if ( desc.IsSet(BuildDesc::TexCoord) )
    {
        vertices.push_back(u);
        vertices.push_back(v);
    }
}
PrimitiveData PrimitiveBuilder::BuildCube(const CubeDesc &desc)
{
    PrimitiveData data;
    float         halfWidth  = desc.Width / 2.0f;
    float         halfHeight = desc.Height / 2.0f;
    float         halfDepth  = desc.Depth / 2.0f;

    std::vector<float> positions = {
        -halfWidth, -halfHeight, -halfDepth, halfWidth, -halfHeight, -halfDepth, halfWidth, halfHeight, -halfDepth, -halfWidth, halfHeight, -halfDepth,
        -halfWidth, -halfHeight, halfDepth,  halfWidth, -halfHeight, halfDepth,  halfWidth, halfHeight, halfDepth,  -halfWidth, halfHeight, halfDepth
    };

    std::vector<float> normals = { 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f };

    std::vector<uint32_t> indices = {
        0,  1,  2,  0,  2,  3,  // Front
        4,  5,  6,  4,  6,  7,  // Back
        8,  9,  10, 8,  10, 11, // Left
        12, 13, 14, 12, 14, 15, // Right
        16, 17, 18, 16, 18, 19, // Bottom
        20, 21, 22, 20, 22, 23  // Top
    };

    for ( size_t i = 0; i < indices.size(); i += 3 )
    {
        for ( size_t j = 0; j < 3; ++j )
        {
            uint32_t index = indices[ i + j ];
            float    x     = positions[ index * 3 ];
            float    y     = positions[ index * 3 + 1 ];
            float    z     = positions[ index * 3 + 2 ];

            float nx = normals[ i / 6 * 3 ];
            float ny = normals[ i / 6 * 3 + 1 ];
            float nz = normals[ i / 6 * 3 + 2 ];

            float tx = 1.0f, ty = 0.0f, tz = 0.0f;
            float bx = 0.0f, by = 0.0f, bz = 1.0f;

            float u = (j == 0 || j == 3) ? 0.0f : 1.0f;
            float v = (j == 0 || j == 1) ? 0.0f : 1.0f;

            AddVertex(data.Vertices, desc.BuildDesc, x, y, z, nx, ny, nz, tx, ty, tz, bx, by, bz, u, v);
        }
    }

    data.Indices = indices;
    return data;
}

PrimitiveData PrimitiveBuilder::BuildSphere(const SphereDesc &desc)
{
    PrimitiveData data;
    const int     segments = 16;
    const int     rings    = 16;

    // Generate vertices
    for ( int i = 0; i <= rings; ++i )
    {
        float theta    = i * std::numbers::pi_v<float> / rings;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);

        for ( int j = 0; j <= segments; ++j )
        {
            float phi    = j * 2 * std::numbers::pi_v<float> / segments;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);

            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;

            float u = 1.0f - (float)j / segments;
            float v = (float)i / rings;

            data.Vertices.push_back(desc.Radius * x);
            data.Vertices.push_back(desc.Radius * y);
            data.Vertices.push_back(desc.Radius * z);

            if ( desc.BuildDesc.IsSet(BuildDesc::Normal) )
            {
                data.Vertices.push_back(x);
                data.Vertices.push_back(y);
                data.Vertices.push_back(z);
            }

            if ( desc.BuildDesc.IsSet(BuildDesc::TexCoord) )
            {
                data.Vertices.push_back(u);
                data.Vertices.push_back(v);
            }
        }
    }

    // Generate indices
    for ( int i = 0; i < rings; ++i )
    {
        for ( int j = 0; j < segments; ++j )
        {
            int first  = (i * (segments + 1)) + j;
            int second = first + segments + 1;

            data.Indices.push_back(first);
            data.Indices.push_back(second);
            data.Indices.push_back(first + 1);

            data.Indices.push_back(second);
            data.Indices.push_back(second + 1);
            data.Indices.push_back(first + 1);
        }
    }

    return data;
}

PrimitiveData PrimitiveBuilder::BuildCylinder(const CylinderDesc &desc)
{
    PrimitiveData data;
    const int     segments = 16;

    // Generate vertices for sides
    for ( int i = 0; i <= segments; ++i )
    {
        float theta = i * 2 * std::numbers::pi_v<float> / segments;
        float x     = cos(theta);
        float z     = sin(theta);

        for ( int j = 0; j <= 1; ++j )
        {
            float y = j * desc.Height;

            // Position
            data.Vertices.push_back(desc.Radius * x);
            data.Vertices.push_back(y);
            data.Vertices.push_back(desc.Radius * z);

            // Normal
            if ( desc.BuildDesc.IsSet(BuildDesc::Normal) )
            {
                data.Vertices.push_back(x);
                data.Vertices.push_back(0.0f);
                data.Vertices.push_back(z);
            }

            // TexCoord
            if ( desc.BuildDesc.IsSet(BuildDesc::TexCoord) )
            {
                data.Vertices.push_back((float)i / segments);
                data.Vertices.push_back((float)j);
            }
        }
    }

    // Generate vertices for top and bottom faces
    for ( int i = 0; i <= segments; ++i )
    {
        float theta = i * 2 * std::numbers::pi_v<float> / segments;
        float x     = cos(theta);
        float z     = sin(theta);

        for ( int j = 0; j <= 1; ++j )
        {
            float y = j * desc.Height - desc.Height / 2.0f;

            // Position
            data.Vertices.push_back(desc.Radius * x);
            data.Vertices.push_back(y);
            data.Vertices.push_back(desc.Radius * z);

            // Normal
            if ( desc.BuildDesc.IsSet(BuildDesc::Normal) )
            {
                data.Vertices.push_back(0.0f);
                data.Vertices.push_back(j == 0 ? -1.0f : 1.0f);
                data.Vertices.push_back(0.0f);
            }

            // TexCoord
            if ( desc.BuildDesc.IsSet(BuildDesc::TexCoord) )
            {
                data.Vertices.push_back((x + 1.0f) / 2.0f);
                data.Vertices.push_back((z + 1.0f) / 2.0f);
            }
        }
    }

    // Generate indices for sides
    for ( int i = 0; i < segments; ++i )
    {
        int base = i * 2;
        data.Indices.push_back(base);
        data.Indices.push_back(base + 1);
        data.Indices.push_back(base + 2);

        data.Indices.push_back(base + 1);
        data.Indices.push_back(base + 3);
        data.Indices.push_back(base + 2);
    }

    // Generate indices for top and bottom faces
    int base = (segments + 1) * 2;
    for ( int i = 0; i < segments; ++i )
    {
        data.Indices.push_back(base);
        data.Indices.push_back(base + 1 + i);
        data.Indices.push_back(base + 1 + (i + 1) % segments);

        data.Indices.push_back(base + 1 + i);
        data.Indices.push_back(base + 2 + i);
        data.Indices.push_back(base + 1 + (i + 1) % segments);
    }

    return data;
}

PrimitiveData PrimitiveBuilder::BuildCone(const ConeDesc &desc)
{
    PrimitiveData data;
    const int     segments = 16;

    // Generate vertices for sides
    for ( int i = 0; i <= segments; ++i )
    {
        float theta = i * 2 * std::numbers::pi_v<float> / segments;
        float x     = cos(theta);
        float z     = sin(theta);

        // Position
        data.Vertices.push_back(desc.Radius * x);
        data.Vertices.push_back(0.0f);
        data.Vertices.push_back(desc.Radius * z);

        // Normal
        if ( desc.BuildDesc.IsSet(BuildDesc::Normal) )
        {
            data.Vertices.push_back(x);
            data.Vertices.push_back(desc.Radius);
            data.Vertices.push_back(z);
        }

        // TexCoord
        if ( desc.BuildDesc.IsSet(BuildDesc::TexCoord) )
        {
            data.Vertices.push_back((float)i / segments);
            data.Vertices.push_back(0.0f);
        }
    }

    // Generate vertices for base
    int base = (segments + 1) * 3;
    data.Vertices.push_back(0.0f);
    data.Vertices.push_back(-desc.Height / 2.0f);
    data.Vertices.push_back(0.0f);

    // Normal
    if ( desc.BuildDesc.IsSet(BuildDesc::Normal) )
    {
        data.Vertices.push_back(0.0f);
        data.Vertices.push_back(-1.0f);
        data.Vertices.push_back(0.0f);
    }

    // TexCoord
    if ( desc.BuildDesc.IsSet(BuildDesc::TexCoord) )
    {
        data.Vertices.push_back(0.5f);
        data.Vertices.push_back(0.5f);
    }

    // Generate indices for sides
    for ( int i = 0; i < segments; ++i )
    {
        data.Indices.push_back(i);
        data.Indices.push_back(i + 1);
        data.Indices.push_back(base / 3);
    }

    return data;
}

PrimitiveData PrimitiveBuilder::BuildTorus(const TorusDesc &desc)
{
    PrimitiveData data;
    const int     circleSegments = 16;
    const int     tubeSegments   = 16;

    for ( int i = 0; i <= circleSegments; ++i )
    {
        float u        = (float)i / circleSegments;
        float theta    = u * 2 * std::numbers::pi_v<float>;
        float cosTheta = cos(theta);
        float sinTheta = sin(theta);

        for ( int j = 0; j <= tubeSegments; ++j )
        {
            float v      = (float)j / tubeSegments;
            float phi    = v * 2 * std::numbers::pi_v<float>;
            float cosPhi = cos(phi);
            float sinPhi = sin(phi);

            float x = (desc.Radius + desc.TubeRadius * cosPhi) * cosTheta;
            float y = desc.TubeRadius * sinPhi;
            float z = (desc.Radius + desc.TubeRadius * cosPhi) * sinTheta;

            float nx = cosTheta * cosPhi;
            float ny = sinPhi;
            float nz = sinTheta * cosPhi;

            float tx = -sinTheta;
            float ty = 0.0f;
            float tz = cosTheta;

            float bx = -sinPhi * cosTheta;
            float by = cosPhi;
            float bz = -sinPhi * sinTheta;

            data.Vertices.push_back(x);
            data.Vertices.push_back(y);
            data.Vertices.push_back(z);

            if ( desc.BuildDesc.IsSet(BuildDesc::Normal) )
            {
                data.Vertices.push_back(nx);
                data.Vertices.push_back(ny);
                data.Vertices.push_back(nz);
            }

            if ( desc.BuildDesc.IsSet(BuildDesc::Tangent) )
            {
                data.Vertices.push_back(tx);
                data.Vertices.push_back(ty);
                data.Vertices.push_back(tz);
            }

            if ( desc.BuildDesc.IsSet(BuildDesc::Bitangent) )
            {
                data.Vertices.push_back(bx);
                data.Vertices.push_back(by);
                data.Vertices.push_back(bz);
            }

            if ( desc.BuildDesc.IsSet(BuildDesc::TexCoord) )
            {
                data.Vertices.push_back(u);
                data.Vertices.push_back(v);
            }
        }
    }

    for ( int i = 0; i < circleSegments; ++i )
    {
        for ( int j = 0; j < tubeSegments; ++j )
        {
            int base = (i * (tubeSegments + 1) + j) * 3;
            data.Indices.push_back(base);
            data.Indices.push_back(base + 3);
            data.Indices.push_back(base + 1);

            data.Indices.push_back(base + 1);
            data.Indices.push_back(base + 3);
            data.Indices.push_back(base + 4);
        }
    }

    return data;
}

PrimitiveData PrimitiveBuilder::BuildPlane(const PlaneDesc &desc)
{
    PrimitiveData data;
    float         halfWidth  = desc.Width / 2.0f;
    float         halfHeight = desc.Height / 2.0f;

    // clang-format off
    std::vector<float> positions = {
        -halfWidth,   halfWidth,  halfHeight,   1.0f,
         halfWidth,  -halfWidth,  halfHeight,   1.0f,
        -halfWidth,  -halfWidth,  halfHeight,   1.0f,
         halfWidth,   halfWidth,  halfHeight,   1.0f
    };
    // clang-format on

    std::vector<float> normals = { 0.0f, 1.0f, 0.0f };

    std::vector<uint32_t> indices = { 0, 1, 2, 0, 3, 1 };

    for ( size_t i = 0; i < indices.size(); i += 3 )
    {
        for ( size_t j = 0; j < 3; ++j )
        {
            uint32_t index = indices[ i + j ];
            float    x     = positions[ index * 3 ];
            float    y     = positions[ index * 3 + 1 ];
            float    z     = positions[ index * 3 + 2 ];

            float nx = normals[ 0 ];
            float ny = normals[ 1 ];
            float nz = normals[ 2 ];

            float tx = 1.0f, ty = 0.0f, tz = 0.0f;
            float bx = 0.0f, by = 0.0f, bz = 1.0f;

            float u = (j == 0 || j == 3) ? 0.0f : 1.0f;
            float v = (j == 0 || j == 1) ? 0.0f : 1.0f;

            AddVertex(data.Vertices, desc.BuildDesc, x, y, z, nx, ny, nz, tx, ty, tz, bx, by, bz, u, v);
        }
    }

    data.Vertices = positions;
    data.Indices = indices;
    return data;
}
