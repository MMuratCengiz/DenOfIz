/*
Den Of Iz - Game/Game Engine
Copyright (c) 2020-2024 Muhammed Murat Cengiz

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

// This file contains modified code from the DirectX Tool Kit, released under the MIT License:
//--------------------------------------------------------------------------------------
// File: Geometry.cpp
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------
#include <DenOfIzGraphics/Data/Geometry.h>
#include <stdexcept>

using namespace DenOfIz;
using namespace DirectX;

constexpr float SQRT2 = 1.41421356237309504880f;
constexpr float SQRT3 = 1.73205080756887729352f;
constexpr float SQRT6 = 2.44948974278317809820f;

inline void CheckIndexOverflow( const size_t value )
{
    // Use >=, not > comparison, because some D3D level 9_x hardware does not support 0xFFFF index values.
    if ( value >= 65535u )
    {
        throw std::out_of_range( "Index value out of range: cannot tessellate primitive so finely" );
    }
}

void VertexEmplace( InteropArray<GeometryVertexData> &vertices, FXMVECTOR iposition, FXMVECTOR inormal, FXMVECTOR itextureCoordinate )
{
    GeometryVertexData &vertexData = vertices.EmplaceElement( );
    vertexData.Position.X          = XMVectorGetX( iposition );
    vertexData.Position.Y          = XMVectorGetY( iposition );
    vertexData.Position.Z          = XMVectorGetZ( iposition );
    vertexData.Normal.X            = XMVectorGetX( inormal );
    vertexData.Normal.Y            = XMVectorGetY( inormal );
    vertexData.Normal.Z            = XMVectorGetZ( inormal );
    vertexData.TextureCoordinate.U = XMVectorGetX( itextureCoordinate );
    vertexData.TextureCoordinate.V = XMVectorGetY( itextureCoordinate );
}

// Collection types used when generating the geometry.
inline void EmplaceIndex( GeometryData &data, const size_t value )
{
    CheckIndexOverflow( value );
    data.Indices.AddElement( static_cast<uint32_t>( value ) );
}

// Helper for flipping winding of geometric primitives for LH vs. RH coordinates
inline void ReverseWinding( GeometryData &data )
{
    assert( ( data.Indices.NumElements( ) % 3 ) == 0 );
    for ( int i = 0; i < data.Indices.NumElements( ); i += 3 )
    {
        data.Indices.Swap( i, i + 2 );
    }

    for ( int i = 0; i < data.Vertices.NumElements( ); ++i )
    {
        data.Vertices.GetElement( i ).TextureCoordinate.U = ( 1.f - data.Vertices.GetElement( i ).TextureCoordinate.U );
    }
}

// Helper for inverting normals of geometric primitives for 'inside' vs. 'outside' viewing
inline void InvertNormals( GeometryData &data )
{
    for ( int i = 0; i < data.Vertices.NumElements( ); ++i )
    {
        auto &it    = data.Vertices.GetElement( i );
        it.Normal.X = -it.Normal.X;
        it.Normal.Y = -it.Normal.Y;
        it.Normal.Z = -it.Normal.Z;
    }
}

//--------------------------------------------------------------------------------------
// Quad, XY Plane
//--------------------------------------------------------------------------------------
GeometryData Geometry::BuildQuadXY( const QuadDesc &quadDesc )
{
    const bool   rightHanded   = quadDesc.BuildDesc.IsSet( BuildDesc::RightHanded );
    const bool   invertNormals = quadDesc.BuildDesc.IsSet( BuildDesc::InvertNormals );
    GeometryData result{ };
    auto        &vertices = result.Vertices;
    auto        &indices  = result.Indices;

    const float halfWidth  = quadDesc.Width / 2.0f;
    const float halfHeight = quadDesc.Height / 2.0f;

    const XMVECTOR positions[ 4 ] = {
        XMVectorSet( -halfWidth, halfHeight, 0.0f, 0.0f ),  // 0: Top-Left (TL)
        XMVectorSet( -halfWidth, -halfHeight, 0.0f, 0.0f ), // 1: Bottom-Left (BL)
        XMVectorSet( halfWidth, -halfHeight, 0.0f, 0.0f ),  // 2: Bottom-Right (BR)
        XMVectorSet( halfWidth, halfHeight, 0.0f, 0.0f )    // 3: Top-Right (TR)
    };

    const XMVECTOR texCoords[ 4 ] = {
        XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f ), // 0: TL UV (0,0)
        XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ), // 1: BL UV (0,1)
        XMVectorSet( 1.0f, 1.0f, 0.0f, 0.0f ), // 2: BR UV (1,1)
        XMVectorSet( 1.0f, 0.0f, 0.0f, 0.0f )  // 3: TR UV (1,0)
    };

    const XMVECTOR normal = rightHanded ? g_XMIdentityR2 : g_XMNegIdentityR2; // +Z for RH, -Z for LH

    VertexEmplace( vertices, positions[ 0 ], normal, texCoords[ 0 ] );
    VertexEmplace( vertices, positions[ 1 ], normal, texCoords[ 1 ] );
    VertexEmplace( vertices, positions[ 2 ], normal, texCoords[ 2 ] );
    VertexEmplace( vertices, positions[ 3 ], normal, texCoords[ 3 ] );

    indices.AddElement( 0 );
    indices.AddElement( 1 );
    indices.AddElement( 2 );
    indices.AddElement( 0 );
    indices.AddElement( 2 );
    indices.AddElement( 3 );

    if ( !rightHanded )
    {
        // This will change indices to: 0, 2, 1 and 0, 3, 2 (CW)
        ReverseWinding( result );
    }

    if ( invertNormals )
    {
        InvertNormals( result );
    }

    return result;
}

//--------------------------------------------------------------------------------------
// Quad on the XZ plane
//--------------------------------------------------------------------------------------
GeometryData Geometry::BuildQuadXZ( const QuadDesc &quadDesc )
{
    const bool   rightHanded   = quadDesc.BuildDesc.IsSet( BuildDesc::RightHanded );
    const bool   invertNormals = quadDesc.BuildDesc.IsSet( BuildDesc::InvertNormals );
    GeometryData result{ };
    auto        &vertices = result.Vertices;
    auto        &indices  = result.Indices;

    const float halfWidth = quadDesc.Width / 2.0f;
    const float halfDepth = quadDesc.Height / 2.0f;

    const XMVECTOR positions[ 4 ] = {
        XMVectorSet( -halfWidth, 0.0f, halfDepth, 0.0f ),  // 0: Top-Left (in XZ view)
        XMVectorSet( -halfWidth, 0.0f, -halfDepth, 0.0f ), // 1: Bottom-Left
        XMVectorSet( halfWidth, 0.0f, -halfDepth, 0.0f ),  // 2: Bottom-Right
        XMVectorSet( halfWidth, 0.0f, halfDepth, 0.0f )    // 3: Top-Right
    };

    const XMVECTOR texCoords[ 4 ] = {
        XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f ), // 0: TL UV (0,0)
        XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ), // 1: BL UV (0,1)
        XMVectorSet( 1.0f, 1.0f, 0.0f, 0.0f ), // 2: BR UV (1,1)
        XMVectorSet( 1.0f, 0.0f, 0.0f, 0.0f )  // 3: TR UV (1,0)
    };

    const XMVECTOR normal = g_XMIdentityR1; // +Y Axis (0, 1, 0)

    VertexEmplace( vertices, positions[ 0 ], normal, texCoords[ 0 ] );
    VertexEmplace( vertices, positions[ 1 ], normal, texCoords[ 1 ] );
    VertexEmplace( vertices, positions[ 2 ], normal, texCoords[ 2 ] );
    VertexEmplace( vertices, positions[ 3 ], normal, texCoords[ 3 ] );

    indices.AddElement( 0 );
    indices.AddElement( 1 );
    indices.AddElement( 2 );
    indices.AddElement( 0 );
    indices.AddElement( 2 );
    indices.AddElement( 3 );

    if ( !rightHanded )
    {
        // This will change indices to: 0, 2, 1 and 0, 3, 2 (CW relative to +Y normal)
        ReverseWinding( result );
    }

    if ( invertNormals )
    {
        InvertNormals( result );
    }

    return result;
}

//--------------------------------------------------------------------------------------
// Cube (aka a Hexahedron) or Box
//--------------------------------------------------------------------------------------
GeometryData Geometry::BuildBox( const BoxDesc &boxDesc )
{
    const XMFLOAT3 size( boxDesc.Width, boxDesc.Height, boxDesc.Depth );
    const bool     rightHanded   = boxDesc.BuildDesc.IsSet( BuildDesc::RightHanded );
    const bool     invertNormals = boxDesc.BuildDesc.IsSet( BuildDesc::InvertNormals );
    GeometryData   result{ };
    auto          &vertices = result.Vertices;

    // A box has six faces, each one pointing in a different direction.
    constexpr int FaceCount = 6;

    static const XMVECTORF32 faceNormals[ FaceCount ] = {
        { { { 0, 0, 1, 0 } } }, { { { 0, 0, -1, 0 } } }, { { { 1, 0, 0, 0 } } }, { { { -1, 0, 0, 0 } } }, { { { 0, 1, 0, 0 } } }, { { { 0, -1, 0, 0 } } },
    };

    static constexpr XMVECTORF32 textureCoordinates[ 4 ] = {
        { { { 1, 0, 0, 0 } } },
        { { { 1, 1, 0, 0 } } },
        { { { 0, 1, 0, 0 } } },
        { { { 0, 0, 0, 0 } } },
    };

    XMVECTOR tsize = XMLoadFloat3( &size );
    tsize          = XMVectorDivide( tsize, g_XMTwo );

    // Create each face in turn.
    for ( int i = 0; i < FaceCount; i++ )
    {
        const XMVECTOR normal = faceNormals[ i ];

        // Get two vectors perpendicular both to the face normal and to each other.
        const XMVECTOR basis = ( i >= 4 ) ? g_XMIdentityR2 : g_XMIdentityR1;

        const XMVECTOR side1 = XMVector3Cross( normal, basis );
        const XMVECTOR side2 = XMVector3Cross( normal, side1 );

        // Six indices (two triangles) per face.
        const size_t vbase = vertices.NumElements( );
        EmplaceIndex( result, vbase + 0 );
        EmplaceIndex( result, vbase + 1 );
        EmplaceIndex( result, vbase + 2 );

        EmplaceIndex( result, vbase + 0 );
        EmplaceIndex( result, vbase + 2 );
        EmplaceIndex( result, vbase + 3 );

        // Four vertices per face.
        // (normal - side1 - side2) * tsize // normal // t0
        VertexEmplace( vertices, XMVectorMultiply( XMVectorSubtract( XMVectorSubtract( normal, side1 ), side2 ), tsize ), normal, textureCoordinates[ 0 ] );

        // (normal - side1 + side2) * tsize // normal // t1
        VertexEmplace( vertices, XMVectorMultiply( XMVectorAdd( XMVectorSubtract( normal, side1 ), side2 ), tsize ), normal, textureCoordinates[ 1 ] );

        // (normal + side1 + side2) * tsize // normal // t2
        VertexEmplace( vertices, XMVectorMultiply( XMVectorAdd( normal, XMVectorAdd( side1, side2 ) ), tsize ), normal, textureCoordinates[ 2 ] );

        // (normal + side1 - side2) * tsize // normal // t3
        VertexEmplace( vertices, XMVectorMultiply( XMVectorSubtract( XMVectorAdd( normal, side1 ), side2 ), tsize ), normal, textureCoordinates[ 3 ] );
    }

    // Build RH above
    if ( !rightHanded )
    {
        ReverseWinding( result );
    }

    if ( invertNormals )
    {
        InvertNormals( result );
    }

    return result;
}

//--------------------------------------------------------------------------------------
// Sphere
//--------------------------------------------------------------------------------------
GeometryData Geometry::BuildSphere( const SphereDesc &sphereDesc )
{
    const float  diameter      = sphereDesc.Diameter;
    const size_t tessellation  = sphereDesc.Tessellation;
    const bool   rightHanded   = sphereDesc.BuildDesc.IsSet( BuildDesc::RightHanded );
    const bool   invertNormals = sphereDesc.BuildDesc.IsSet( BuildDesc::InvertNormals );
    GeometryData result{ };
    auto        &vertices = result.Vertices;

    if ( tessellation < 3 )
    {
        throw std::invalid_argument( "tesselation parameter must be at least 3" );
    }

    const size_t verticalSegments   = tessellation;
    const size_t horizontalSegments = tessellation * 2;

    const float radius = diameter / 2;

    // Create rings of vertices at progressively higher latitudes.
    for ( size_t i = 0; i <= verticalSegments; i++ )
    {
        const float v = 1 - static_cast<float>( i ) / static_cast<float>( verticalSegments );

        const float latitude = static_cast<float>( i ) * XM_PI / static_cast<float>( verticalSegments ) - XM_PIDIV2;
        float       dy, dxz;

        XMScalarSinCos( &dy, &dxz, latitude );

        // Create a single ring of vertices at this latitude.
        for ( size_t j = 0; j <= horizontalSegments; j++ )
        {
            const float u = static_cast<float>( j ) / static_cast<float>( horizontalSegments );

            const float longitude = static_cast<float>( j ) * XM_2PI / static_cast<float>( horizontalSegments );
            float       dx, dz;

            XMScalarSinCos( &dx, &dz, longitude );

            dx *= dxz;
            dz *= dxz;

            const XMVECTOR normal            = XMVectorSet( dx, dy, dz, 0 );
            const XMVECTOR textureCoordinate = XMVectorSet( u, v, 0, 0 );

            VertexEmplace( vertices, XMVectorScale( normal, radius ), normal, textureCoordinate );
        }
    }

    // Fill the index buffer with triangles joining each pair of latitude rings.
    const size_t stride = horizontalSegments + 1;

    for ( size_t i = 0; i < verticalSegments; i++ )
    {
        for ( size_t j = 0; j <= horizontalSegments; j++ )
        {
            const size_t nextI = i + 1;
            const size_t nextJ = ( j + 1 ) % stride;

            EmplaceIndex( result, i * stride + j );
            EmplaceIndex( result, nextI * stride + j );
            EmplaceIndex( result, i * stride + nextJ );

            EmplaceIndex( result, i * stride + nextJ );
            EmplaceIndex( result, nextI * stride + j );
            EmplaceIndex( result, nextI * stride + nextJ );
        }
    }

    // Build RH above
    if ( !rightHanded )
    {
        ReverseWinding( result );
    }

    if ( invertNormals )
    {
        InvertNormals( result );
    }

    return result;
}

//--------------------------------------------------------------------------------------
// Geodesic sphere
//--------------------------------------------------------------------------------------
GeometryData Geometry::BuildGeoSphere( const GeoSphereDesc &geoSphereDesc )
{
    float        diameter     = geoSphereDesc.Diameter;
    size_t       tessellation = geoSphereDesc.Tessellation;
    bool         rightHanded  = geoSphereDesc.BuildDesc.IsSet( BuildDesc::RightHanded );
    GeometryData result{ };
    auto        &vertices = result.Vertices;
    auto        &indices  = result.Indices;

    // An undirected edge between two vertices, represented by a pair of indexes into a vertex array.
    // Because this edge is undirected, (a,b) is the same as (b,a).
    using UndirectedEdge = std::pair<uint16_t, uint16_t>;

    // Makes an undirected edge. Rather than overloading comparison operators to give us the (a,b)==(b,a) property,
    // we'll just ensure that the larger of the two goes first. This will simplify things greatly.
    auto makeUndirectedEdge = []( const uint16_t a, const uint16_t b ) noexcept { return std::make_pair( std::max( a, b ), std::min( a, b ) ); };

    // Key: an edge
    // Value: the index of the vertex which lies midway between the two vertices pointed to by the key value
    // This map is used to avoid duplicating vertices when subdividing triangles along edges.
    using EdgeSubdivisionMap = std::map<UndirectedEdge, uint16_t>;

    static constexpr XMFLOAT3 OctahedronVertices[] = {
        // when looking down the negative z-axis (into the screen)
        XMFLOAT3( 0, 1, 0 ),  // 0 top
        XMFLOAT3( 0, 0, -1 ), // 1 front
        XMFLOAT3( 1, 0, 0 ),  // 2 right
        XMFLOAT3( 0, 0, 1 ),  // 3 back
        XMFLOAT3( -1, 0, 0 ), // 4 left
        XMFLOAT3( 0, -1, 0 ), // 5 bottom
    };
    static const uint16_t OctahedronIndices[] = {
        0, 1, 2, // top front-right face
        0, 2, 3, // top back-right face
        0, 3, 4, // top back-left face
        0, 4, 1, // top front-left face
        5, 1, 4, // bottom front-left face
        5, 4, 3, // bottom back-left face
        5, 3, 2, // bottom back-right face
        5, 2, 1, // bottom front-right face
    };

    const float radius = diameter / 2.0f;

    // Start with an octahedron; copy the data into the vertex/index collection.

    std::vector<XMFLOAT3> vertexPositions( std::begin( OctahedronVertices ), std::end( OctahedronVertices ) );

    for ( auto &octahedronIndex : OctahedronIndices )
    {
        indices.AddElement( octahedronIndex );
    }

    // We know these values by looking at the above index list for the octahedron. Despite the subdivisions that are
    // about to go on, these values aren't ever going to change because the vertices don't move around in the array.
    // We'll need these values later on to fix the singularities that show up at the poles.
    constexpr uint16_t northPoleIndex = 0;
    constexpr uint16_t southPoleIndex = 5;

    for ( size_t iSubdivision = 0; iSubdivision < tessellation; ++iSubdivision )
    {
        assert( indices.NumElements( ) % 3 == 0 ); // sanity

        // We use this to keep track of which edges have already been subdivided.
        EdgeSubdivisionMap subdividedEdges;

        // The new index collection after subdivision.
        InteropArray<uint32_t> newIndices;

        const size_t triangleCount = indices.NumElements( ) / 3;
        for ( size_t iTriangle = 0; iTriangle < triangleCount; ++iTriangle )
        {
            // For each edge on this triangle, create a new vertex in the middle of that edge.
            // The winding order of the triangles we output are the same as the winding order of the inputs.

            // Indices of the vertices making up this triangle
            const uint16_t iv0 = indices.GetElement( iTriangle * 3 + 0 );
            const uint16_t iv1 = indices.GetElement( iTriangle * 3 + 1 );
            const uint16_t iv2 = indices.GetElement( iTriangle * 3 + 2 );

            // Get the new vertices
            XMFLOAT3 v01{ }; // vertex on the midpoint of v0 and v1
            XMFLOAT3 v12{ }; // ditto v1 and v2
            XMFLOAT3 v20{ }; // ditto v2 and v0
            uint16_t iv01;   // index of v01
            uint16_t iv12;   // index of v12
            uint16_t iv20;   // index of v20

            // Function that, when given the index of two vertices, creates a new vertex at the midpoint of those vertices.
            auto const divideEdge = [ & ]( const uint16_t i0, const uint16_t i1, XMFLOAT3 &outVertex, uint16_t &outIndex )
            {
                const UndirectedEdge edge = makeUndirectedEdge( i0, i1 );

                // Check to see if we've already generated this vertex
                if ( const auto it = subdividedEdges.find( edge ); it != subdividedEdges.end( ) )
                {
                    // We've already generated this vertex before
                    outIndex  = it->second;                  // the index of this vertex
                    outVertex = vertexPositions[ outIndex ]; // and the vertex itself
                }
                else
                {
                    // Haven't generated this vertex before: so add it now

                    // outVertex = (vertices[i0] + vertices[i1]) / 2
                    XMStoreFloat3( &outVertex, XMVectorScale( XMVectorAdd( XMLoadFloat3( &vertexPositions[ i0 ] ), XMLoadFloat3( &vertexPositions[ i1 ] ) ), 0.5f ) );

                    outIndex = static_cast<uint16_t>( vertexPositions.size( ) );
                    CheckIndexOverflow( outIndex );
                    vertexPositions.push_back( outVertex );

                    // Now add it to the map.
                    auto entry = std::make_pair( edge, outIndex );
                    subdividedEdges.insert( entry );
                }
            };

            // Add/get new vertices and their indices
            divideEdge( iv0, iv1, v01, iv01 );
            divideEdge( iv1, iv2, v12, iv12 );
            divideEdge( iv0, iv2, v20, iv20 );

            // Add the new indices. We have four new triangles from our original one:
            //        v0
            //        o
            //       /a\
            //  v20 o---o v01
            //     /b\c/d\
            // v2 o---o---o v1
            //       v12
            const uint16_t indicesToAdd[] = {
                iv0,  iv01, iv20, // a
                iv20, iv12, iv2,  // b
                iv20, iv01, iv12, // c
                iv01, iv1,  iv12, // d
            };

            for ( const auto &indexToAdd : indicesToAdd )
            {
                newIndices.AddElement( indexToAdd );
            }
        }

        indices = std::move( newIndices );
    }

    // Now that we've completed subdivision, fill in the final vertex collection
    vertices.Resize( vertexPositions.size( ) );
    for ( const auto &it : vertexPositions )
    {
        auto const normal = XMVector3Normalize( XMLoadFloat3( &it ) );
        auto const pos    = XMVectorScale( normal, radius );

        XMFLOAT3 normalFloat3{ };
        XMStoreFloat3( &normalFloat3, normal );

        // calculate texture coordinates for this vertex
        const float longitude = atan2f( normalFloat3.x, -normalFloat3.z );
        const float latitude  = acosf( normalFloat3.y );

        const float u = longitude / XM_2PI + 0.5f;
        const float v = latitude / XM_PI;

        auto const texCoord = XMVectorSet( 1.0f - u, v, 0.0f, 0.0f );
        VertexEmplace( vertices, pos, normal, texCoord );
    }

    // There are a couple of fixes to do. One is a texture coordinate wraparound fixup. At some point, there will be
    // a set of triangles somewhere in the mesh with texture coordinates such that the wraparound across 0.0/1.0
    // occurs across that triangle. e.g. when the left hand side of the triangle has a U coordinate of 0.98 and the
    // right hand side has a U coordinate of 0.0. The intent is that such a triangle should render with a U of 0.98 to
    // 1.0, not 0.98 to 0.0. If we don't do this fixup, there will be a visible seam across one side of the sphere.
    //
    // Luckily this is relatively easy to fix. There is a straight edge which runs down the prime meridian of the
    // completed sphere. If you imagine the vertices along that edge, they circumscribe a semicircular arc starting at
    // y=1 and ending at y=-1, and sweeping across the range of z=0 to z=1. x stays zero. It's along this edge that we
    // need to duplicate our vertices - and provide the correct texture coordinates.
    const size_t preFixupVertexCount = vertices.NumElements( );
    for ( size_t i = 0; i < preFixupVertexCount; ++i )
    {
        // This vertex is on the prime meridian if position.x and texture coordinates are both zero (allowing for small epsilon).
        const bool isOnPrimeMeridian = XMVector2NearEqual( XMVectorSet( vertices.GetElement( i ).Position.X, vertices.GetElement( i ).TextureCoordinate.U, 0.0f, 0.0f ),
                                                           XMVectorZero( ), XMVectorSplatEpsilon( ) );

        if ( isOnPrimeMeridian )
        {
            size_t newIndex = vertices.NumElements( ); // the index of this vertex that we're about to add
            CheckIndexOverflow( newIndex );

            // copy this vertex, correct the texture coordinate, and add the vertex
            GeometryVertexData v  = vertices.GetElement( i );
            v.TextureCoordinate.U = 1.0f;
            vertices.AddElement( v );

            // Now find all the triangles which contain this vertex and update them if necessary
            for ( size_t j = 0; j < indices.NumElements( ); j += 3 )
            {
                uint32_t *triIndex0 = &indices.GetElement( j + 0 );
                uint32_t *triIndex1 = &indices.GetElement( j + 1 );
                uint32_t *triIndex2 = &indices.GetElement( j + 2 );

                if ( *triIndex0 == i )
                {
                    // nothing; just keep going
                }
                else if ( *triIndex1 == i )
                {
                    std::swap( triIndex0, triIndex1 ); // swap the pointers (not the values)
                }
                else if ( *triIndex2 == i )
                {
                    std::swap( triIndex0, triIndex2 ); // swap the pointers (not the values)
                }
                else
                {
                    // this triangle doesn't use the vertex we're interested in
                    continue;
                }

                // If we got to this point then triIndex0 is the pointer to the index to the vertex we're looking at
                assert( *triIndex0 == i );
                assert( *triIndex1 != i && *triIndex2 != i ); // assume no degenerate triangles

                const GeometryVertexData &v0 = vertices.GetElement( *triIndex0 );
                const GeometryVertexData &v1 = vertices.GetElement( *triIndex1 );
                const GeometryVertexData &v2 = vertices.GetElement( *triIndex2 );

                // check the other two vertices to see if we might need to fix this triangle

                if ( abs( v0.TextureCoordinate.U - v1.TextureCoordinate.U ) > 0.5f || abs( v0.TextureCoordinate.U - v2.TextureCoordinate.U ) > 0.5f )
                {
                    // yep; replace the specified index to point to the new, corrected vertex
                    *triIndex0 = static_cast<uint16_t>( newIndex );
                }
            }
        }
    }

    // And one last fix we need to do: the poles. A common use-case of a sphere mesh is to map a rectangular texture onto
    // it. If that happens, then the poles become singularities which map the entire top and bottom rows of the texture
    // onto a single point. In general there's no real way to do that right. But to match the behavior of non-geodesic
    // spheres, we need to duplicate the pole vertex for every triangle that uses it. This will introduce seams near the
    // poles, but reduce stretching.
    auto const fixPole = [ & ]( const size_t poleIndex )
    {
        const auto &poleVertex            = vertices.GetElement( poleIndex );
        bool        overwrittenPoleVertex = false; // overwriting the original pole vertex saves us one vertex

        for ( size_t i = 0; i < indices.NumElements( ); i += 3 )
        {
            // These pointers point to the three indices which make up this triangle. pPoleIndex is the pointer to the
            // entry in the index array which represents the pole index, and the other two pointers point to the other
            // two indices making up this triangle.
            uint32_t *pPoleIndex;
            uint32_t *pOtherIndex0;
            uint32_t *pOtherIndex1;
            if ( indices.GetElement( i + 0 ) == poleIndex )
            {
                pPoleIndex   = &indices.GetElement( i + 0 );
                pOtherIndex0 = &indices.GetElement( i + 1 );
                pOtherIndex1 = &indices.GetElement( i + 2 );
            }
            else if ( indices.GetElement( i + 1 ) == poleIndex )
            {
                pPoleIndex   = &indices.GetElement( i + 1 );
                pOtherIndex0 = &indices.GetElement( i + 2 );
                pOtherIndex1 = &indices.GetElement( i + 0 );
            }
            else if ( indices.GetElement( i + 2 ) == poleIndex )
            {
                pPoleIndex   = &indices.GetElement( i + 2 );
                pOtherIndex0 = &indices.GetElement( i + 0 );
                pOtherIndex1 = &indices.GetElement( i + 1 );
            }
            else
            {
                continue;
            }

            const auto &otherVertex0 = vertices.GetElement( *pOtherIndex0 );
            const auto &otherVertex1 = vertices.GetElement( *pOtherIndex1 );

            // Calculate the texture coordinates for the new pole vertex, add it to the vertices and update the index
            GeometryVertexData newPoleVertex  = poleVertex;
            newPoleVertex.TextureCoordinate.U = ( otherVertex0.TextureCoordinate.U + otherVertex1.TextureCoordinate.U ) / 2;
            newPoleVertex.TextureCoordinate.V = poleVertex.TextureCoordinate.V;

            if ( !overwrittenPoleVertex )
            {
                vertices.GetElement( poleIndex ) = newPoleVertex;
                overwrittenPoleVertex            = true;
            }
            else
            {
                CheckIndexOverflow( vertices.NumElements( ) );

                *pPoleIndex = static_cast<uint16_t>( vertices.NumElements( ) );
                vertices.AddElement( newPoleVertex );
            }
        }
    };

    fixPole( northPoleIndex );
    fixPole( southPoleIndex );

    // Build RH above
    if ( !rightHanded )
    {
        ReverseWinding( result );
    }

    return result;
}

//--------------------------------------------------------------------------------------
// Cylinder / Cone
//--------------------------------------------------------------------------------------

// Helper computes a point on a unit circle, aligned to the x/z plane and centered on the origin.
inline XMVECTOR GetCircleVector( const size_t i, const size_t tessellation ) noexcept
{
    const float angle = static_cast<float>( i ) * XM_2PI / static_cast<float>( tessellation );
    float       dx, dz;

    XMScalarSinCos( &dx, &dz, angle );

    const XMVECTORF32 v = { { { dx, 0, dz, 0 } } };
    return v;
}

inline XMVECTOR GetCircleTangent( const size_t i, const size_t tessellation ) noexcept
{
    const float angle = ( static_cast<float>( i ) * XM_2PI / static_cast<float>( tessellation ) ) + XM_PIDIV2;
    float       dx, dz;

    XMScalarSinCos( &dx, &dz, angle );

    const XMVECTORF32 v = { { { dx, 0, dz, 0 } } };
    return v;
}

// Helper creates a triangle fan to close the end of a cylinder / cone
void CreateCylinderCap( GeometryData &result, const size_t tessellation, const float height, const float radius, const bool isTop )
{
    auto &vertices = result.Vertices;
    // Create cap indices.
    for ( size_t i = 0; i < tessellation - 2; i++ )
    {
        size_t i1 = ( i + 1 ) % tessellation;
        size_t i2 = ( i + 2 ) % tessellation;

        if ( isTop )
        {
            std::swap( i1, i2 );
        }

        const size_t vbase = vertices.NumElements( );
        EmplaceIndex( result, vbase );
        EmplaceIndex( result, vbase + i1 );
        EmplaceIndex( result, vbase + i2 );
    }

    // Which end of the cylinder is this?
    XMVECTOR normal       = g_XMIdentityR1;
    XMVECTOR textureScale = g_XMNegativeOneHalf;

    if ( !isTop )
    {
        normal       = XMVectorNegate( normal );
        textureScale = XMVectorMultiply( textureScale, g_XMNegateX );
    }

    // Create cap vertices.
    for ( size_t i = 0; i < tessellation; i++ )
    {
        const XMVECTOR circleVector = GetCircleVector( i, tessellation );

        const XMVECTOR position = XMVectorAdd( XMVectorScale( circleVector, radius ), XMVectorScale( normal, height ) );

        const XMVECTOR textureCoordinate = XMVectorMultiplyAdd( XMVectorSwizzle<0, 2, 3, 3>( circleVector ), textureScale, g_XMOneHalf );

        VertexEmplace( vertices, position, normal, textureCoordinate );
    }
}

GeometryData Geometry::BuildCylinder( const CylinderDesc &cylinderDesc )
{
    const float  diameter     = cylinderDesc.Diameter;
    float        height       = cylinderDesc.Height;
    const size_t tessellation = cylinderDesc.Tessellation;
    const bool   rightHanded  = cylinderDesc.BuildDesc.IsSet( BuildDesc::RightHanded );
    GeometryData result{ };
    auto        &vertices = result.Vertices;

    if ( tessellation < 3 )
        throw std::invalid_argument( "tesselation parameter must be at least 3" );

    height /= 2;

    const XMVECTOR topOffset = XMVectorScale( g_XMIdentityR1, height );

    const float  radius = diameter / 2;
    const size_t stride = tessellation + 1;

    // Create a ring of triangles around the outside of the cylinder.
    for ( size_t i = 0; i <= tessellation; i++ )
    {
        const XMVECTOR normal = GetCircleVector( i, tessellation );

        const XMVECTOR sideOffset = XMVectorScale( normal, radius );

        const float u = static_cast<float>( i ) / static_cast<float>( tessellation );

        const XMVECTOR textureCoordinate = XMLoadFloat( &u );

        VertexEmplace( vertices, XMVectorAdd( sideOffset, topOffset ), normal, textureCoordinate );
        VertexEmplace( vertices, XMVectorSubtract( sideOffset, topOffset ), normal, XMVectorAdd( textureCoordinate, g_XMIdentityR1 ) );

        EmplaceIndex( result, i * 2 );
        EmplaceIndex( result, ( i * 2 + 2 ) % ( stride * 2 ) );
        EmplaceIndex( result, i * 2 + 1 );

        EmplaceIndex( result, i * 2 + 1 );
        EmplaceIndex( result, ( i * 2 + 2 ) % ( stride * 2 ) );
        EmplaceIndex( result, ( i * 2 + 3 ) % ( stride * 2 ) );
    }

    // Create flat triangle fan caps to seal the top and bottom.
    CreateCylinderCap( result, tessellation, height, radius, true );
    CreateCylinderCap( result, tessellation, height, radius, false );

    // Build RH above
    if ( !rightHanded )
    {
        ReverseWinding( result );
    }

    return result;
}

// Creates a cone primitive.
GeometryData Geometry::BuildCone( const ConeDesc &coneDesc )
{
    const float  diameter     = coneDesc.Diameter;
    float        height       = coneDesc.Height;
    const size_t tessellation = coneDesc.Tessellation;
    const bool   rightHanded  = coneDesc.BuildDesc.IsSet( BuildDesc::RightHanded );
    GeometryData result{ };
    auto        &vertices = result.Vertices;

    if ( tessellation < 3 )
        throw std::invalid_argument( "tesselation parameter must be at least 3" );

    height /= 2;

    const XMVECTOR topOffset = XMVectorScale( g_XMIdentityR1, height );

    const float  radius = diameter / 2;
    const size_t stride = tessellation + 1;

    // Create a ring of triangles around the outside of the cone.
    for ( size_t i = 0; i <= tessellation; i++ )
    {
        const XMVECTOR circle = GetCircleVector( i, tessellation );

        const XMVECTOR sideOffset = XMVectorScale( circle, radius );

        const float u = static_cast<float>( i ) / static_cast<float>( tessellation );

        const XMVECTOR textureCoordinate = XMLoadFloat( &u );

        const XMVECTOR pt = XMVectorSubtract( sideOffset, topOffset );

        XMVECTOR normal = XMVector3Cross( GetCircleTangent( i, tessellation ), XMVectorSubtract( topOffset, pt ) );
        normal          = XMVector3Normalize( normal );

        // Duplicate the top vertex for distinct normals
        VertexEmplace( vertices, topOffset, normal, g_XMZero );
        VertexEmplace( vertices, pt, normal, XMVectorAdd( textureCoordinate, g_XMIdentityR1 ) );

        EmplaceIndex( result, i * 2 );
        EmplaceIndex( result, ( i * 2 + 3 ) % ( stride * 2 ) );
        EmplaceIndex( result, ( i * 2 + 1 ) % ( stride * 2 ) );
    }

    // Create flat triangle fan caps to seal the bottom.
    CreateCylinderCap( result, tessellation, height, radius, false );

    // Build RH above
    if ( !rightHanded )
    {
        ReverseWinding( result );
    }

    return result;
}

//--------------------------------------------------------------------------------------
// Torus
//--------------------------------------------------------------------------------------
GeometryData Geometry::BuildTorus( const TorusDesc &torusDesc )
{
    const float  diameter     = torusDesc.Diameter;
    const float  thickness    = torusDesc.Thickness;
    const size_t tessellation = torusDesc.Tessellation;
    const bool   rightHanded  = torusDesc.BuildDesc.IsSet( BuildDesc::RightHanded );
    GeometryData result{ };
    auto        &vertices = result.Vertices;

    if ( tessellation < 3 )
        throw std::invalid_argument( "tesselation parameter must be at least 3" );

    const size_t stride = tessellation + 1;

    // First we loop around the main ring of the torus.
    for ( size_t i = 0; i <= tessellation; i++ )
    {
        const float u = static_cast<float>( i ) / static_cast<float>( tessellation );

        const float outerAngle = static_cast<float>( i ) * XM_2PI / static_cast<float>( tessellation ) - XM_PIDIV2;

        // Create a transform matrix that will align geometry to
        // slice perpendicularly though the current ring position.
        const XMMATRIX transform = XMMatrixTranslation( diameter / 2, 0, 0 ) * XMMatrixRotationY( outerAngle );

        // Now we loop along the other axis, around the side of the tube.
        for ( size_t j = 0; j <= tessellation; j++ )
        {
            const float v = 1 - static_cast<float>( j ) / static_cast<float>( tessellation );

            const float innerAngle = static_cast<float>( j ) * XM_2PI / static_cast<float>( tessellation ) + XM_PI;
            float       dx, dy;

            XMScalarSinCos( &dy, &dx, innerAngle );

            // Create a vertex.
            XMVECTOR       normal            = XMVectorSet( dx, dy, 0, 0 );
            XMVECTOR       position          = XMVectorScale( normal, thickness / 2 );
            const XMVECTOR textureCoordinate = XMVectorSet( u, v, 0, 0 );

            position = XMVector3Transform( position, transform );
            normal   = XMVector3TransformNormal( normal, transform );

            VertexEmplace( vertices, position, normal, textureCoordinate );

            // And create indices for two triangles.
            const size_t nextI = ( i + 1 ) % stride;
            const size_t nextJ = ( j + 1 ) % stride;

            EmplaceIndex( result, i * stride + j );
            EmplaceIndex( result, i * stride + nextJ );
            EmplaceIndex( result, nextI * stride + j );

            EmplaceIndex( result, i * stride + nextJ );
            EmplaceIndex( result, nextI * stride + nextJ );
            EmplaceIndex( result, nextI * stride + j );
        }
    }

    // Build RH above
    if ( !rightHanded )
    {
        ReverseWinding( result );
    }

    return result;
}

//--------------------------------------------------------------------------------------
// Tetrahedron
//--------------------------------------------------------------------------------------
GeometryData Geometry::BuildTetrahedron( const TetrahedronDesc &tetrahedronDesc )
{
    const float  size        = tetrahedronDesc.Size;
    const bool   rightHanded = tetrahedronDesc.BuildDesc.IsSet( BuildDesc::RightHanded );
    GeometryData result{ };
    auto        &vertices = result.Vertices;
    const auto  &indices  = result.Indices;

    static constexpr XMVECTORF32 verts[ 4 ] = { { { { 0.f, 0.f, 1.f, 0 } } },
                                                { { { 2.f * SQRT2 / 3.f, 0.f, -1.f / 3.f, 0 } } },
                                                { { { -SQRT2 / 3.f, SQRT6 / 3.f, -1.f / 3.f, 0 } } },
                                                { { { -SQRT2 / 3.f, -SQRT6 / 3.f, -1.f / 3.f, 0 } } } };

    static const uint32_t faces[ 4 * 3 ] = {
        0, 1, 2, 0, 2, 3, 0, 3, 1, 1, 3, 2,
    };

    for ( size_t j = 0; j < std::size( faces ); j += 3 )
    {
        const uint32_t v0 = faces[ j ];
        const uint32_t v1 = faces[ j + 1 ];
        const uint32_t v2 = faces[ j + 2 ];

        XMVECTOR normal = XMVector3Cross( XMVectorSubtract( verts[ v1 ].v, verts[ v0 ].v ), XMVectorSubtract( verts[ v2 ].v, verts[ v0 ].v ) );
        normal          = XMVector3Normalize( normal );

        const size_t base = vertices.NumElements( );
        EmplaceIndex( result, base );
        EmplaceIndex( result, base + 1 );
        EmplaceIndex( result, base + 2 );

        // Duplicate vertices to use face normals
        XMVECTOR position = XMVectorScale( verts[ v0 ], size );
        VertexEmplace( vertices, position, normal, g_XMZero /* 0, 0 */ );

        position = XMVectorScale( verts[ v1 ], size );
        VertexEmplace( vertices, position, normal, g_XMIdentityR0 /* 1, 0 */ );

        position = XMVectorScale( verts[ v2 ], size );
        VertexEmplace( vertices, position, normal, g_XMIdentityR1 /* 0, 1 */ );
    }

    // Built LH above
    if ( rightHanded )
    {
        ReverseWinding( result );
    }

    assert( vertices.NumElements( ) == 4 * 3 );
    assert( indices.NumElements( ) == 4 * 3 );
    return result;
}

//--------------------------------------------------------------------------------------
// Octahedron
//--------------------------------------------------------------------------------------
GeometryData Geometry::BuildOctahedron( const OctahedronDesc &octahedronDesc )
{
    const float  size        = octahedronDesc.Size;
    const bool   rightHanded = octahedronDesc.BuildDesc.IsSet( BuildDesc::RightHanded );
    GeometryData result{ };
    auto        &vertices = result.Vertices;
    const auto  &indices  = result.Indices;

    static const XMVECTORF32 verts[ 6 ] = { { { { 1, 0, 0, 0 } } },  { { { -1, 0, 0, 0 } } }, { { { 0, 1, 0, 0 } } },
                                            { { { 0, -1, 0, 0 } } }, { { { 0, 0, 1, 0 } } },  { { { 0, 0, -1, 0 } } } };

    static const uint32_t faces[ 8 * 3 ] = { 4, 0, 2, 4, 2, 1, 4, 1, 3, 4, 3, 0, 5, 2, 0, 5, 1, 2, 5, 3, 1, 5, 0, 3 };

    for ( size_t j = 0; j < std::size( faces ); j += 3 )
    {
        const uint32_t v0 = faces[ j ];
        const uint32_t v1 = faces[ j + 1 ];
        const uint32_t v2 = faces[ j + 2 ];

        XMVECTOR normal = XMVector3Cross( XMVectorSubtract( verts[ v1 ].v, verts[ v0 ].v ), XMVectorSubtract( verts[ v2 ].v, verts[ v0 ].v ) );
        normal          = XMVector3Normalize( normal );

        const size_t base = vertices.NumElements( );
        EmplaceIndex( result, base );
        EmplaceIndex( result, base + 1 );
        EmplaceIndex( result, base + 2 );

        // Duplicate vertices to use face normals
        XMVECTOR position = XMVectorScale( verts[ v0 ], size );
        VertexEmplace( vertices, position, normal, g_XMZero /* 0, 0 */ );

        position = XMVectorScale( verts[ v1 ], size );
        VertexEmplace( vertices, position, normal, g_XMIdentityR0 /* 1, 0 */ );

        position = XMVectorScale( verts[ v2 ], size );
        VertexEmplace( vertices, position, normal, g_XMIdentityR1 /* 0, 1*/ );
    }

    // Built LH above
    if ( rightHanded )
    {
        ReverseWinding( result );
    }

    assert( vertices.NumElements( ) == 8 * 3 );
    assert( indices.NumElements( ) == 8 * 3 );
    return result;
}

//--------------------------------------------------------------------------------------
// Dodecahedron
//--------------------------------------------------------------------------------------
GeometryData Geometry::BuildDodecahedron( const DodecahedronDesc &dodecahedronDesc )
{
    const float  size        = dodecahedronDesc.Size;
    const bool   rightHanded = dodecahedronDesc.BuildDesc.IsSet( BuildDesc::RightHanded );
    GeometryData result{ };
    auto        &vertices = result.Vertices;
    const auto  &indices  = result.Indices;

    constexpr float a = 1.f / SQRT3;
    constexpr float b = 0.356822089773089931942f; // sqrt( ( 3 - sqrt(5) ) / 6 )
    constexpr float c = 0.934172358962715696451f; // sqrt( ( 3 + sqrt(5) ) / 6 );

    static const XMVECTORF32 verts[ 20 ] = { { { { a, a, a, 0 } } },   { { { a, a, -a, 0 } } },  { { { a, -a, a, 0 } } },   { { { a, -a, -a, 0 } } }, { { { -a, a, a, 0 } } },
                                             { { { -a, a, -a, 0 } } }, { { { -a, -a, a, 0 } } }, { { { -a, -a, -a, 0 } } }, { { { b, c, 0, 0 } } },   { { { -b, c, 0, 0 } } },
                                             { { { b, -c, 0, 0 } } },  { { { -b, -c, 0, 0 } } }, { { { c, 0, b, 0 } } },    { { { c, 0, -b, 0 } } },  { { { -c, 0, b, 0 } } },
                                             { { { -c, 0, -b, 0 } } }, { { { 0, b, c, 0 } } },   { { { 0, -b, c, 0 } } },   { { { 0, b, -c, 0 } } },  { { { 0, -b, -c, 0 } } } };

    static const uint32_t faces[ 12 * 5 ] = {
        0, 8,  9,  4, 16, 0, 16, 17, 2, 12, 12, 2, 10, 3, 13, 9, 5,  15, 14, 4,  3, 19, 18, 1,  13, 7, 11, 6, 14, 15,
        0, 12, 13, 1, 8,  8, 1,  18, 5, 9,  16, 4, 14, 6, 17, 6, 11, 10, 2,  17, 7, 15, 5,  18, 19, 7, 19, 3, 10, 11,
    };

    static const XMVECTORF32 textureCoordinates[ 5 ] = { { { { 0.654508f, 0.0244717f, 0, 0 } } },
                                                         { { { 0.0954915f, 0.206107f, 0, 0 } } },
                                                         { { { 0.0954915f, 0.793893f, 0, 0 } } },
                                                         { { { 0.654508f, 0.975528f, 0, 0 } } },
                                                         { { { 1.f, 0.5f, 0, 0 } } } };

    static const uint32_t textureIndex[ 12 ][ 5 ] = {
        { 0, 1, 2, 3, 4 }, { 2, 3, 4, 0, 1 }, { 4, 0, 1, 2, 3 }, { 1, 2, 3, 4, 0 }, { 2, 3, 4, 0, 1 }, { 0, 1, 2, 3, 4 },
        { 1, 2, 3, 4, 0 }, { 4, 0, 1, 2, 3 }, { 4, 0, 1, 2, 3 }, { 1, 2, 3, 4, 0 }, { 0, 1, 2, 3, 4 }, { 2, 3, 4, 0, 1 },
    };

    size_t t = 0;
    for ( size_t j = 0; j < std::size( faces ); j += 5, ++t )
    {
        const uint32_t v0 = faces[ j ];
        const uint32_t v1 = faces[ j + 1 ];
        const uint32_t v2 = faces[ j + 2 ];
        const uint32_t v3 = faces[ j + 3 ];
        const uint32_t v4 = faces[ j + 4 ];

        XMVECTOR normal = XMVector3Cross( XMVectorSubtract( verts[ v1 ].v, verts[ v0 ].v ), XMVectorSubtract( verts[ v2 ].v, verts[ v0 ].v ) );
        normal          = XMVector3Normalize( normal );

        const size_t base = vertices.NumElements( );

        EmplaceIndex( result, base );
        EmplaceIndex( result, base + 1 );
        EmplaceIndex( result, base + 2 );

        EmplaceIndex( result, base );
        EmplaceIndex( result, base + 2 );
        EmplaceIndex( result, base + 3 );

        EmplaceIndex( result, base );
        EmplaceIndex( result, base + 3 );
        EmplaceIndex( result, base + 4 );

        // Duplicate vertices to use face normals
        XMVECTOR position = XMVectorScale( verts[ v0 ], size );
        VertexEmplace( vertices, position, normal, textureCoordinates[ textureIndex[ t ][ 0 ] ] );

        position = XMVectorScale( verts[ v1 ], size );
        VertexEmplace( vertices, position, normal, textureCoordinates[ textureIndex[ t ][ 1 ] ] );

        position = XMVectorScale( verts[ v2 ], size );
        VertexEmplace( vertices, position, normal, textureCoordinates[ textureIndex[ t ][ 2 ] ] );

        position = XMVectorScale( verts[ v3 ], size );
        VertexEmplace( vertices, position, normal, textureCoordinates[ textureIndex[ t ][ 3 ] ] );

        position = XMVectorScale( verts[ v4 ], size );
        VertexEmplace( vertices, position, normal, textureCoordinates[ textureIndex[ t ][ 4 ] ] );
    }

    // Built LH above
    if ( rightHanded )
    {
        ReverseWinding( result );
    }

    assert( vertices.NumElements( ) == 12 * 5 );
    assert( indices.NumElements( ) == 12 * 3 * 3 );

    return result;
}

//--------------------------------------------------------------------------------------
// Icosahedron
//--------------------------------------------------------------------------------------
GeometryData Geometry::BuildIcosahedron( const IcosahedronDesc &icosahedronDesc )
{
    const float  size        = icosahedronDesc.Size;
    const bool   rightHanded = icosahedronDesc.BuildDesc.IsSet( BuildDesc::RightHanded );
    GeometryData result{ };
    auto        &vertices = result.Vertices;

    constexpr float t  = 1.618033988749894848205f; // (1 + sqrt(5)) / 2
    constexpr float t2 = 1.519544995837552493271f; // sqrt( 1 + sqr( (1 + sqrt(5)) / 2 ) )

    static const XMVECTORF32 verts[ 12 ] = { { { { t / t2, 1.f / t2, 0, 0 } } },   { { { -t / t2, 1.f / t2, 0, 0 } } },  { { { t / t2, -1.f / t2, 0, 0 } } },
                                             { { { -t / t2, -1.f / t2, 0, 0 } } }, { { { 1.f / t2, 0, t / t2, 0 } } },   { { { 1.f / t2, 0, -t / t2, 0 } } },
                                             { { { -1.f / t2, 0, t / t2, 0 } } },  { { { -1.f / t2, 0, -t / t2, 0 } } }, { { { 0, t / t2, 1.f / t2, 0 } } },
                                             { { { 0, -t / t2, 1.f / t2, 0 } } },  { { { 0, t / t2, -1.f / t2, 0 } } },  { { { 0, -t / t2, -1.f / t2, 0 } } } };

    static const uint32_t faces[ 20 * 3 ] = { 0, 8, 4,  0, 5,  10, 2, 4, 9, 2, 11, 5, 1, 6, 8, 1, 10, 7, 3, 9, 6, 3, 7, 11, 0,  10, 8, 1,  8, 10,
                                              2, 9, 11, 3, 11, 9,  4, 2, 0, 5, 0,  2, 6, 1, 3, 7, 3,  1, 8, 6, 4, 9, 4, 6,  10, 5,  7, 11, 7, 5 };

    for ( size_t j = 0; j < std::size( faces ); j += 3 )
    {
        const uint32_t v0 = faces[ j ];
        const uint32_t v1 = faces[ j + 1 ];
        const uint32_t v2 = faces[ j + 2 ];

        XMVECTOR normal = XMVector3Cross( XMVectorSubtract( verts[ v1 ].v, verts[ v0 ].v ), XMVectorSubtract( verts[ v2 ].v, verts[ v0 ].v ) );
        normal          = XMVector3Normalize( normal );

        const size_t base = vertices.NumElements( );
        EmplaceIndex( result, base );
        EmplaceIndex( result, base + 1 );
        EmplaceIndex( result, base + 2 );

        // Duplicate vertices to use face normals
        XMVECTOR position = XMVectorScale( verts[ v0 ], size );
        VertexEmplace( vertices, position, normal, g_XMZero /* 0, 0 */ );

        position = XMVectorScale( verts[ v1 ], size );
        VertexEmplace( vertices, position, normal, g_XMIdentityR0 /* 1, 0 */ );

        position = XMVectorScale( verts[ v2 ], size );
        VertexEmplace( vertices, position, normal, g_XMIdentityR1 /* 0, 1 */ );
    }

    // Built LH above
    if ( rightHanded )
    {
        ReverseWinding( result );
    }

    assert( vertices.NumElements( ) == 20 * 3 );
    return result;
}
