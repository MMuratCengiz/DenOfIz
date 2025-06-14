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
#include "DenOfIzGraphics/Data/Geometry.h"
#include <DirectXMath.h>
#include <stdexcept>
#include <vector>

#include "DenOfIzGraphicsInternal/Utilities/DZArenaHelper.h"

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

void SetVertex( GeometryVertexData *vertices, const size_t index, FXMVECTOR position, FXMVECTOR normal, FXMVECTOR textureCoordinate )
{
    vertices[ index ].Position.X          = XMVectorGetX( position );
    vertices[ index ].Position.Y          = XMVectorGetY( position );
    vertices[ index ].Position.Z          = XMVectorGetZ( position );
    vertices[ index ].Normal.X            = XMVectorGetX( normal );
    vertices[ index ].Normal.Y            = XMVectorGetY( normal );
    vertices[ index ].Normal.Z            = XMVectorGetZ( normal );
    vertices[ index ].TextureCoordinate.U = XMVectorGetX( textureCoordinate );
    vertices[ index ].TextureCoordinate.V = XMVectorGetY( textureCoordinate );
}

inline void SetIndex( uint32_t *indices, const size_t index, const size_t value )
{
    CheckIndexOverflow( value );
    indices[ index ] = static_cast<uint32_t>( value );
}

// Helper for flipping winding of geometric primitives for LH vs. RH coordinates
inline void ReverseWinding( const GeometryData &data )
{
    assert( data.Indices.NumElements % 3 == 0 );
    for ( uint32_t i = 0; i < data.Indices.NumElements; i += 3 )
    {
        const uint32_t temp            = data.Indices.Elements[ i ];
        data.Indices.Elements[ i ]     = data.Indices.Elements[ i + 2 ];
        data.Indices.Elements[ i + 2 ] = temp;
    }

    for ( uint32_t i = 0; i < data.Vertices.NumElements; ++i )
    {
        data.Vertices.Elements[ i ].TextureCoordinate.U = 1.f - data.Vertices.Elements[ i ].TextureCoordinate.U;
    }
}

// Helper for inverting normals of geometric primitives for 'inside' vs. 'outside' viewing
inline void InvertNormals( const GeometryData &data )
{
    for ( uint32_t i = 0; i < data.Vertices.NumElements; ++i )
    {
        auto &it    = data.Vertices.Elements[ i ];
        it.Normal.X = -it.Normal.X;
        it.Normal.Y = -it.Normal.Y;
        it.Normal.Z = -it.Normal.Z;
    }
}

//--------------------------------------------------------------------------------------
// Quad, XY Plane
//--------------------------------------------------------------------------------------
GeometryData *Geometry::BuildQuadXY( const QuadDesc &quadDesc )
{
    const bool rightHanded   = ( quadDesc.BuildDesc & BuildDesc::RightHanded ) == BuildDesc::RightHanded;
    const bool invertNormals = ( quadDesc.BuildDesc & BuildDesc::InvertNormals ) == BuildDesc::InvertNormals;

    auto *result = new GeometryData( );

    // Calculate sizes
    constexpr size_t vertexCount = 4;
    constexpr size_t indexCount  = 6;
    constexpr size_t arenaSize   = sizeof( GeometryVertexData ) * vertexCount + sizeof( uint32_t ) * indexCount + 1024;
    result->_Arena.EnsureCapacity( arenaSize );

    // Allocate arrays
    result->Vertices.Elements    = DZArenaAllocator<GeometryVertexData>::AllocateAndConstruct( result->_Arena, vertexCount );
    result->Vertices.NumElements = static_cast<uint32_t>( vertexCount );
    result->Indices.Elements     = DZArenaAllocator<uint32_t>::Allocate( result->_Arena, indexCount );
    result->Indices.NumElements  = static_cast<uint32_t>( indexCount );

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

    // Set vertices
    for ( size_t i = 0; i < 4; ++i )
    {
        SetVertex( result->Vertices.Elements, i, positions[ i ], normal, texCoords[ i ] );
    }

    // Set indices
    SetIndex( result->Indices.Elements, 0, 0 );
    SetIndex( result->Indices.Elements, 1, 1 );
    SetIndex( result->Indices.Elements, 2, 2 );
    SetIndex( result->Indices.Elements, 3, 0 );
    SetIndex( result->Indices.Elements, 4, 2 );
    SetIndex( result->Indices.Elements, 5, 3 );

    if ( !rightHanded )
    {
        ReverseWinding( *result );
    }

    if ( invertNormals )
    {
        InvertNormals( *result );
    }

    return result;
}

//--------------------------------------------------------------------------------------
// Quad on the XZ plane
//--------------------------------------------------------------------------------------
GeometryData *Geometry::BuildQuadXZ( const QuadDesc &quadDesc )
{
    const bool rightHanded   = ( quadDesc.BuildDesc & BuildDesc::RightHanded ) == BuildDesc::RightHanded;
    const bool invertNormals = ( quadDesc.BuildDesc & BuildDesc::InvertNormals ) == BuildDesc::InvertNormals;

    auto *result = new GeometryData( );

    // Calculate sizes
    constexpr size_t vertexCount = 4;
    constexpr size_t indexCount  = 6;
    constexpr size_t arenaSize   = sizeof( GeometryVertexData ) * vertexCount + sizeof( uint32_t ) * indexCount + 1024;
    result->_Arena.EnsureCapacity( arenaSize );

    // Allocate arrays
    result->Vertices.Elements    = DZArenaAllocator<GeometryVertexData>::AllocateAndConstruct( result->_Arena, vertexCount );
    result->Vertices.NumElements = static_cast<uint32_t>( vertexCount );
    result->Indices.Elements     = DZArenaAllocator<uint32_t>::Allocate( result->_Arena, indexCount );
    result->Indices.NumElements  = static_cast<uint32_t>( indexCount );

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

    // Set vertices
    for ( size_t i = 0; i < 4; ++i )
    {
        SetVertex( result->Vertices.Elements, i, positions[ i ], normal, texCoords[ i ] );
    }

    // Set indices
    SetIndex( result->Indices.Elements, 0, 0 );
    SetIndex( result->Indices.Elements, 1, 1 );
    SetIndex( result->Indices.Elements, 2, 2 );
    SetIndex( result->Indices.Elements, 3, 0 );
    SetIndex( result->Indices.Elements, 4, 2 );
    SetIndex( result->Indices.Elements, 5, 3 );

    if ( !rightHanded )
    {
        ReverseWinding( *result );
    }

    if ( invertNormals )
    {
        InvertNormals( *result );
    }

    return result;
}

//--------------------------------------------------------------------------------------
// Cube (aka a Hexahedron) or Box
//--------------------------------------------------------------------------------------
GeometryData *Geometry::BuildBox( const BoxDesc &desc )
{
    const XMFLOAT3 size( desc.Width, desc.Height, desc.Depth );
    const bool     rightHanded   = ( desc.BuildDesc & BuildDesc::RightHanded ) == BuildDesc::RightHanded;
    const bool     invertNormals = ( desc.BuildDesc & BuildDesc::InvertNormals ) == BuildDesc::InvertNormals;

    auto *result = new GeometryData( );

    // A box has six faces, each one pointing in a different direction.
    constexpr int FaceCount = 6;

    // Calculate sizes: 24 vertices (4 per face) + 36 indices (6 per face)
    constexpr size_t vertexCount = 24;
    constexpr size_t indexCount  = 36;
    constexpr size_t arenaSize   = sizeof( GeometryVertexData ) * vertexCount + sizeof( uint32_t ) * indexCount + 1024;
    result->_Arena.EnsureCapacity( arenaSize );

    // Allocate arrays
    result->Vertices.Elements    = DZArenaAllocator<GeometryVertexData>::AllocateAndConstruct( result->_Arena, vertexCount );
    result->Vertices.NumElements = static_cast<uint32_t>( vertexCount );
    result->Indices.Elements     = DZArenaAllocator<uint32_t>::Allocate( result->_Arena, indexCount );
    result->Indices.NumElements  = static_cast<uint32_t>( indexCount );

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

    size_t vertexIndex = 0;
    size_t indexIndex  = 0;

    // Create each face in turn.
    for ( int i = 0; i < FaceCount; i++ )
    {
        const XMVECTOR normal = faceNormals[ i ];

        // Get two vectors perpendicular both to the face normal and to each other.
        const XMVECTOR basis = i >= 4 ? g_XMIdentityR2 : g_XMIdentityR1;

        const XMVECTOR side1 = XMVector3Cross( normal, basis );
        const XMVECTOR side2 = XMVector3Cross( normal, side1 );

        // Six indices (two triangles) per face.
        const size_t vbase = vertexIndex;
        SetIndex( result->Indices.Elements, indexIndex++, vbase + 0 );
        SetIndex( result->Indices.Elements, indexIndex++, vbase + 1 );
        SetIndex( result->Indices.Elements, indexIndex++, vbase + 2 );

        SetIndex( result->Indices.Elements, indexIndex++, vbase + 0 );
        SetIndex( result->Indices.Elements, indexIndex++, vbase + 2 );
        SetIndex( result->Indices.Elements, indexIndex++, vbase + 3 );

        // Four vertices per face.
        // (normal - side1 - side2) * tsize // normal // t0
        SetVertex( result->Vertices.Elements, vertexIndex++, XMVectorMultiply( XMVectorSubtract( XMVectorSubtract( normal, side1 ), side2 ), tsize ), normal,
                   textureCoordinates[ 0 ] );

        // (normal - side1 + side2) * tsize // normal // t1
        SetVertex( result->Vertices.Elements, vertexIndex++, XMVectorMultiply( XMVectorAdd( XMVectorSubtract( normal, side1 ), side2 ), tsize ), normal, textureCoordinates[ 1 ] );

        // (normal + side1 + side2) * tsize // normal // t2
        SetVertex( result->Vertices.Elements, vertexIndex++, XMVectorMultiply( XMVectorAdd( normal, XMVectorAdd( side1, side2 ) ), tsize ), normal, textureCoordinates[ 2 ] );

        // (normal + side1 - side2) * tsize // normal // t3
        SetVertex( result->Vertices.Elements, vertexIndex++, XMVectorMultiply( XMVectorSubtract( XMVectorAdd( normal, side1 ), side2 ), tsize ), normal, textureCoordinates[ 3 ] );
    }

    // Build RH above
    if ( !rightHanded )
    {
        ReverseWinding( *result );
    }

    if ( invertNormals )
    {
        InvertNormals( *result );
    }

    return result;
}

//--------------------------------------------------------------------------------------
// Sphere
//--------------------------------------------------------------------------------------
GeometryData *Geometry::BuildSphere( const SphereDesc &desc )
{
    const float  diameter      = desc.Diameter;
    const size_t tessellation  = desc.Tessellation;
    const bool   rightHanded   = ( desc.BuildDesc & BuildDesc::RightHanded ) == BuildDesc::RightHanded;
    const bool   invertNormals = ( desc.BuildDesc & BuildDesc::InvertNormals ) == BuildDesc::InvertNormals;

    if ( tessellation < 3 )
    {
        throw std::invalid_argument( "tesselation parameter must be at least 3" );
    }

    auto *result = new GeometryData( );

    const size_t verticalSegments   = tessellation;
    const size_t horizontalSegments = tessellation * 2;

    // Calculate sizes
    const size_t vertexCount = ( verticalSegments + 1 ) * ( horizontalSegments + 1 );
    const size_t indexCount  = verticalSegments * ( horizontalSegments + 1 ) * 6;

    // Estimate arena size
    const size_t arenaSize = sizeof( GeometryVertexData ) * vertexCount + sizeof( uint32_t ) * indexCount + 1024;
    result->_Arena.EnsureCapacity( arenaSize );

    // Allocate arrays
    result->Vertices.Elements    = DZArenaAllocator<GeometryVertexData>::AllocateAndConstruct( result->_Arena, vertexCount );
    result->Vertices.NumElements = static_cast<uint32_t>( vertexCount );
    result->Indices.Elements     = DZArenaAllocator<uint32_t>::Allocate( result->_Arena, indexCount );
    result->Indices.NumElements  = static_cast<uint32_t>( indexCount );

    const float radius      = diameter / 2;
    size_t      vertexIndex = 0;

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

            SetVertex( result->Vertices.Elements, vertexIndex++, XMVectorScale( normal, radius ), normal, textureCoordinate );
        }
    }

    // Fill the index buffer with triangles joining each pair of latitude rings.
    const size_t stride     = horizontalSegments + 1;
    size_t       indexIndex = 0;

    for ( size_t i = 0; i < verticalSegments; i++ )
    {
        for ( size_t j = 0; j <= horizontalSegments; j++ )
        {
            const size_t nextI = i + 1;
            const size_t nextJ = ( j + 1 ) % stride;

            SetIndex( result->Indices.Elements, indexIndex++, i * stride + j );
            SetIndex( result->Indices.Elements, indexIndex++, nextI * stride + j );
            SetIndex( result->Indices.Elements, indexIndex++, i * stride + nextJ );

            SetIndex( result->Indices.Elements, indexIndex++, i * stride + nextJ );
            SetIndex( result->Indices.Elements, indexIndex++, nextI * stride + j );
            SetIndex( result->Indices.Elements, indexIndex++, nextI * stride + nextJ );
        }
    }

    // Build RH above
    if ( !rightHanded )
    {
        ReverseWinding( *result );
    }

    if ( invertNormals )
    {
        InvertNormals( *result );
    }

    return result;
}

//--------------------------------------------------------------------------------------
// Geodesic sphere
//--------------------------------------------------------------------------------------
GeometryData *Geometry::BuildGeoSphere( const GeoSphereDesc &desc )
{
    float  diameter     = desc.Diameter;
    size_t tessellation = desc.Tessellation;
    bool   rightHanded  = ( desc.BuildDesc & BuildDesc::RightHanded ) == BuildDesc::RightHanded;
    auto  *result       = new GeometryData( );

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
    std::vector<uint32_t> indices;
    indices.reserve( 1024 ); // Initial capacity

    for ( auto &octahedronIndex : OctahedronIndices )
    {
        indices.push_back( octahedronIndex );
    }

    // We know these values by looking at the above index list for the octahedron. Despite the subdivisions that are
    // about to go on, these values aren't ever going to change because the vertices don't move around in the array.
    // We'll need these values later on to fix the singularities that show up at the poles.
    constexpr uint16_t northPoleIndex = 0;
    constexpr uint16_t southPoleIndex = 5;

    for ( size_t iSubdivision = 0; iSubdivision < tessellation; ++iSubdivision )
    {
        assert( indices.size( ) % 3 == 0 ); // sanity

        // We use this to keep track of which edges have already been subdivided.
        EdgeSubdivisionMap subdividedEdges;

        // The new index collection after subdivision.
        std::vector<uint32_t> newIndices;
        newIndices.reserve( indices.size( ) * 4 );

        const size_t triangleCount = indices.size( ) / 3;
        for ( size_t iTriangle = 0; iTriangle < triangleCount; ++iTriangle )
        {
            // For each edge on this triangle, create a new vertex in the middle of that edge.
            // The winding order of the triangles we output are the same as the winding order of the inputs.

            // Indices of the vertices making up this triangle
            const uint16_t iv0 = indices[ iTriangle * 3 + 0 ];
            const uint16_t iv1 = indices[ iTriangle * 3 + 1 ];
            const uint16_t iv2 = indices[ iTriangle * 3 + 2 ];

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
                newIndices.push_back( indexToAdd );
            }
        }

        indices = std::move( newIndices );
    }

    // Now that we've completed subdivision, create the final vertices with normals and texture coordinates
    std::vector<GeometryVertexData> vertices;
    vertices.reserve( vertexPositions.size( ) );

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

        GeometryVertexData vertex;
        XMStoreFloat3( reinterpret_cast<XMFLOAT3 *>( &vertex.Position ), pos );
        XMStoreFloat3( reinterpret_cast<XMFLOAT3 *>( &vertex.Normal ), normal );
        vertex.TextureCoordinate.U = 1.0f - u;
        vertex.TextureCoordinate.V = v;
        vertices.push_back( vertex );
    }

    // There are a couple of fixes to do. One is a texture coordinate wraparound fixup.
    const size_t preFixupVertexCount = vertices.size( );
    for ( size_t i = 0; i < preFixupVertexCount; ++i )
    {
        // This vertex is on the prime meridian if position.x and texture coordinates are both zero (allowing for small epsilon).
        const bool isOnPrimeMeridian =
            XMVector2NearEqual( XMVectorSet( vertices[ i ].Position.X, vertices[ i ].TextureCoordinate.U, 0.0f, 0.0f ), XMVectorZero( ), XMVectorSplatEpsilon( ) );

        if ( isOnPrimeMeridian )
        {
            size_t newIndex = vertices.size( ); // the index of this vertex that we're about to add
            CheckIndexOverflow( newIndex );

            // copy this vertex, correct the texture coordinate, and add the vertex
            GeometryVertexData v  = vertices[ i ];
            v.TextureCoordinate.U = 1.0f;
            vertices.push_back( v );

            // Now find all the triangles which contain this vertex and update them if necessary
            for ( size_t j = 0; j < indices.size( ); j += 3 )
            {
                uint32_t *triIndex0 = &indices[ j + 0 ];
                uint32_t *triIndex1 = &indices[ j + 1 ];
                uint32_t *triIndex2 = &indices[ j + 2 ];

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

                const GeometryVertexData &v0 = vertices[ *triIndex0 ];
                const GeometryVertexData &v1 = vertices[ *triIndex1 ];
                const GeometryVertexData &v2 = vertices[ *triIndex2 ];

                // check the other two vertices to see if we might need to fix this triangle

                if ( abs( v0.TextureCoordinate.U - v1.TextureCoordinate.U ) > 0.5f || abs( v0.TextureCoordinate.U - v2.TextureCoordinate.U ) > 0.5f )
                {
                    // yep; replace the specified index to point to the new, corrected vertex
                    *triIndex0 = static_cast<uint16_t>( newIndex );
                }
            }
        }
    }

    // And one last fix we need to do: the poles.
    auto const fixPole = [ & ]( const size_t poleIndex )
    {
        const auto &poleVertex            = vertices[ poleIndex ];
        bool        overwrittenPoleVertex = false; // overwriting the original pole vertex saves us one vertex

        for ( size_t i = 0; i < indices.size( ); i += 3 )
        {
            // These pointers point to the three indices which make up this triangle.
            uint32_t *pPoleIndex;
            uint32_t *pOtherIndex0;
            uint32_t *pOtherIndex1;
            if ( indices[ i + 0 ] == poleIndex )
            {
                pPoleIndex   = &indices[ i + 0 ];
                pOtherIndex0 = &indices[ i + 1 ];
                pOtherIndex1 = &indices[ i + 2 ];
            }
            else if ( indices[ i + 1 ] == poleIndex )
            {
                pPoleIndex   = &indices[ i + 1 ];
                pOtherIndex0 = &indices[ i + 2 ];
                pOtherIndex1 = &indices[ i + 0 ];
            }
            else if ( indices[ i + 2 ] == poleIndex )
            {
                pPoleIndex   = &indices[ i + 2 ];
                pOtherIndex0 = &indices[ i + 0 ];
                pOtherIndex1 = &indices[ i + 1 ];
            }
            else
            {
                continue;
            }

            const auto &otherVertex0 = vertices[ *pOtherIndex0 ];
            const auto &otherVertex1 = vertices[ *pOtherIndex1 ];

            // Calculate the texture coordinates for the new pole vertex, add it to the vertices and update the index
            GeometryVertexData newPoleVertex  = poleVertex;
            newPoleVertex.TextureCoordinate.U = ( otherVertex0.TextureCoordinate.U + otherVertex1.TextureCoordinate.U ) / 2;
            newPoleVertex.TextureCoordinate.V = poleVertex.TextureCoordinate.V;

            if ( !overwrittenPoleVertex )
            {
                vertices[ poleIndex ] = newPoleVertex;
                overwrittenPoleVertex = true;
            }
            else
            {
                CheckIndexOverflow( vertices.size( ) );

                *pPoleIndex = static_cast<uint16_t>( vertices.size( ) );
                vertices.push_back( newPoleVertex );
            }
        }
    };

    fixPole( northPoleIndex );
    fixPole( southPoleIndex );

    // Calculate final sizes and allocate in arena
    const size_t finalVertexCount = vertices.size( );
    const size_t finalIndexCount  = indices.size( );
    const size_t arenaSize        = sizeof( GeometryVertexData ) * finalVertexCount + sizeof( uint32_t ) * finalIndexCount + 1024;
    result->_Arena.EnsureCapacity( arenaSize );

    // Allocate arrays in arena
    result->Vertices.Elements    = DZArenaAllocator<GeometryVertexData>::AllocateAndConstruct( result->_Arena, finalVertexCount );
    result->Vertices.NumElements = static_cast<uint32_t>( finalVertexCount );
    result->Indices.Elements     = DZArenaAllocator<uint32_t>::Allocate( result->_Arena, finalIndexCount );
    result->Indices.NumElements  = static_cast<uint32_t>( finalIndexCount );

    // Copy data from vectors to arena arrays
    for ( size_t i = 0; i < finalVertexCount; ++i )
    {
        result->Vertices.Elements[ i ] = vertices[ i ];
    }
    for ( size_t i = 0; i < finalIndexCount; ++i )
    {
        result->Indices.Elements[ i ] = indices[ i ];
    }

    // Build RH above
    if ( !rightHanded )
    {
        ReverseWinding( *result );
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
    const float angle = static_cast<float>( i ) * XM_2PI / static_cast<float>( tessellation ) + XM_PIDIV2;
    float       dx, dz;

    XMScalarSinCos( &dx, &dz, angle );

    const XMVECTORF32 v = { { { dx, 0, dz, 0 } } };
    return v;
}

// Helper creates a triangle fan to close the end of a cylinder / cone
void CreateCylinderCap( GeometryVertexData *vertices, size_t &vertexIndex, uint32_t *indices, size_t &indexIndex, const size_t tessellation, const float height, const float radius,
                        const bool isTop )
{
    const size_t vbase = vertexIndex;

    // Create cap indices.
    for ( size_t i = 0; i < tessellation - 2; i++ )
    {
        size_t i1 = ( i + 1 ) % tessellation;
        size_t i2 = ( i + 2 ) % tessellation;

        if ( isTop )
        {
            std::swap( i1, i2 );
        }

        SetIndex( indices, indexIndex++, vbase );
        SetIndex( indices, indexIndex++, vbase + i1 );
        SetIndex( indices, indexIndex++, vbase + i2 );
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
        const XMVECTOR circleVector      = GetCircleVector( i, tessellation );
        const XMVECTOR position          = XMVectorAdd( XMVectorScale( circleVector, radius ), XMVectorScale( normal, height ) );
        const XMVECTOR textureCoordinate = XMVectorMultiplyAdd( XMVectorSwizzle<0, 2, 3, 3>( circleVector ), textureScale, g_XMOneHalf );

        SetVertex( vertices, vertexIndex++, position, normal, textureCoordinate );
    }
}

GeometryData *Geometry::BuildCylinder( const CylinderDesc &desc )
{
    const float  diameter     = desc.Diameter;
    float        height       = desc.Height;
    const size_t tessellation = desc.Tessellation;
    const bool   rightHanded  = ( desc.BuildDesc & BuildDesc::RightHanded ) == BuildDesc::RightHanded;

    if ( tessellation < 3 )
        throw std::invalid_argument( "tesselation parameter must be at least 3" );

    auto *result = new GeometryData( );

    // Calculate exact sizes
    const size_t sideVertexCount = ( tessellation + 1 ) * 2;
    const size_t capVertexCount  = tessellation * 2; // 2 caps
    const size_t vertexCount     = sideVertexCount + capVertexCount;
    const size_t sideIndexCount  = tessellation * 6;
    const size_t capIndexCount   = ( tessellation - 2 ) * 3 * 2; // 2 caps
    const size_t indexCount      = sideIndexCount + capIndexCount;
    const size_t arenaSize       = sizeof( GeometryVertexData ) * vertexCount + sizeof( uint32_t ) * indexCount + 1024;
    result->_Arena.EnsureCapacity( arenaSize );

    // Allocate arrays
    result->Vertices.Elements    = DZArenaAllocator<GeometryVertexData>::AllocateAndConstruct( result->_Arena, vertexCount );
    result->Vertices.NumElements = static_cast<uint32_t>( vertexCount );
    result->Indices.Elements     = DZArenaAllocator<uint32_t>::Allocate( result->_Arena, indexCount );
    result->Indices.NumElements  = static_cast<uint32_t>( indexCount );

    height /= 2;

    const XMVECTOR topOffset = XMVectorScale( g_XMIdentityR1, height );

    const float  radius = diameter / 2;
    const size_t stride = tessellation + 1;

    size_t vertexIndex = 0;
    size_t indexIndex  = 0;

    // Create a ring of triangles around the outside of the cylinder.
    for ( size_t i = 0; i <= tessellation; i++ )
    {
        const XMVECTOR normal = GetCircleVector( i, tessellation );

        const XMVECTOR sideOffset = XMVectorScale( normal, radius );

        const float u = static_cast<float>( i ) / static_cast<float>( tessellation );

        const XMVECTOR textureCoordinate = XMLoadFloat( &u );

        SetVertex( result->Vertices.Elements, vertexIndex++, XMVectorAdd( sideOffset, topOffset ), normal, textureCoordinate );
        SetVertex( result->Vertices.Elements, vertexIndex++, XMVectorSubtract( sideOffset, topOffset ), normal, XMVectorAdd( textureCoordinate, g_XMIdentityR1 ) );

        if ( i < tessellation )
        {
            SetIndex( result->Indices.Elements, indexIndex++, i * 2 );
            SetIndex( result->Indices.Elements, indexIndex++, ( i * 2 + 2 ) % ( stride * 2 ) );
            SetIndex( result->Indices.Elements, indexIndex++, i * 2 + 1 );

            SetIndex( result->Indices.Elements, indexIndex++, i * 2 + 1 );
            SetIndex( result->Indices.Elements, indexIndex++, ( i * 2 + 2 ) % ( stride * 2 ) );
            SetIndex( result->Indices.Elements, indexIndex++, ( i * 2 + 3 ) % ( stride * 2 ) );
        }
    }

    // Create flat triangle fan caps to seal the top and bottom.
    CreateCylinderCap( result->Vertices.Elements, vertexIndex, result->Indices.Elements, indexIndex, tessellation, height, radius, true );
    CreateCylinderCap( result->Vertices.Elements, vertexIndex, result->Indices.Elements, indexIndex, tessellation, height, radius, false );

    // Build RH above
    if ( !rightHanded )
    {
        ReverseWinding( *result );
    }

    return result;
}

// Creates a cone primitive.
GeometryData *Geometry::BuildCone( const ConeDesc &desc )
{
    const float  diameter     = desc.Diameter;
    float        height       = desc.Height;
    const size_t tessellation = desc.Tessellation;
    const bool   rightHanded  = ( desc.BuildDesc & BuildDesc::RightHanded ) == BuildDesc::RightHanded;

    if ( tessellation < 3 )
        throw std::invalid_argument( "tesselation parameter must be at least 3" );

    auto *result = new GeometryData( );

    // Calculate exact sizes
    const size_t sideVertexCount = ( tessellation + 1 ) * 2;
    const size_t capVertexCount  = tessellation; // 1 bottom cap
    const size_t vertexCount     = sideVertexCount + capVertexCount;
    const size_t sideIndexCount  = tessellation * 3;
    const size_t capIndexCount   = ( tessellation - 2 ) * 3; // 1 cap
    const size_t indexCount      = sideIndexCount + capIndexCount;
    const size_t arenaSize       = sizeof( GeometryVertexData ) * vertexCount + sizeof( uint32_t ) * indexCount + 1024;
    result->_Arena.EnsureCapacity( arenaSize );

    // Allocate arrays
    result->Vertices.Elements    = DZArenaAllocator<GeometryVertexData>::AllocateAndConstruct( result->_Arena, vertexCount );
    result->Vertices.NumElements = static_cast<uint32_t>( vertexCount );
    result->Indices.Elements     = DZArenaAllocator<uint32_t>::Allocate( result->_Arena, indexCount );
    result->Indices.NumElements  = static_cast<uint32_t>( indexCount );

    height /= 2;

    const XMVECTOR topOffset = XMVectorScale( g_XMIdentityR1, height );

    const float  radius = diameter / 2;
    const size_t stride = tessellation + 1;

    size_t vertexIndex = 0;
    size_t indexIndex  = 0;

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
        SetVertex( result->Vertices.Elements, vertexIndex++, topOffset, normal, g_XMZero );
        SetVertex( result->Vertices.Elements, vertexIndex++, pt, normal, XMVectorAdd( textureCoordinate, g_XMIdentityR1 ) );

        if ( i < tessellation )
        {
            SetIndex( result->Indices.Elements, indexIndex++, i * 2 );
            SetIndex( result->Indices.Elements, indexIndex++, ( i * 2 + 3 ) % ( stride * 2 ) );
            SetIndex( result->Indices.Elements, indexIndex++, ( i * 2 + 1 ) % ( stride * 2 ) );
        }
    }

    // Create flat triangle fan caps to seal the bottom.
    CreateCylinderCap( result->Vertices.Elements, vertexIndex, result->Indices.Elements, indexIndex, tessellation, height, radius, false );

    // Build RH above
    if ( !rightHanded )
    {
        ReverseWinding( *result );
    }

    return result;
}

//--------------------------------------------------------------------------------------
// Torus
//--------------------------------------------------------------------------------------
GeometryData *Geometry::BuildTorus( const TorusDesc &desc )
{
    const float  diameter     = desc.Diameter;
    const float  thickness    = desc.Thickness;
    const size_t tessellation = desc.Tessellation;
    const bool   rightHanded  = ( desc.BuildDesc & BuildDesc::RightHanded ) == BuildDesc::RightHanded;

    if ( tessellation < 3 )
        throw std::invalid_argument( "tesselation parameter must be at least 3" );

    auto *result = new GeometryData( );

    const size_t stride = tessellation + 1;
    // Calculate sizes
    const size_t vertexCount = stride * stride;
    const size_t indexCount  = tessellation * tessellation * 6;
    const size_t arenaSize   = sizeof( GeometryVertexData ) * vertexCount + sizeof( uint32_t ) * indexCount + 1024;
    result->_Arena.EnsureCapacity( arenaSize );

    // Allocate arrays
    result->Vertices.Elements    = DZArenaAllocator<GeometryVertexData>::AllocateAndConstruct( result->_Arena, vertexCount );
    result->Vertices.NumElements = static_cast<uint32_t>( vertexCount );
    result->Indices.Elements     = DZArenaAllocator<uint32_t>::Allocate( result->_Arena, indexCount );
    result->Indices.NumElements  = static_cast<uint32_t>( indexCount );

    size_t vertexIndex = 0;
    size_t indexIndex  = 0;

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

            SetVertex( result->Vertices.Elements, vertexIndex++, position, normal, textureCoordinate );

            // And create indices for two triangles.
            if ( i < tessellation && j < tessellation )
            {
                const size_t nextI = ( i + 1 ) % stride;
                const size_t nextJ = ( j + 1 ) % stride;

                SetIndex( result->Indices.Elements, indexIndex++, i * stride + j );
                SetIndex( result->Indices.Elements, indexIndex++, i * stride + nextJ );
                SetIndex( result->Indices.Elements, indexIndex++, nextI * stride + j );

                SetIndex( result->Indices.Elements, indexIndex++, i * stride + nextJ );
                SetIndex( result->Indices.Elements, indexIndex++, nextI * stride + nextJ );
                SetIndex( result->Indices.Elements, indexIndex++, nextI * stride + j );
            }
        }
    }

    // Build RH above
    if ( !rightHanded )
    {
        ReverseWinding( *result );
    }

    return result;
}

//--------------------------------------------------------------------------------------
// Tetrahedron
//--------------------------------------------------------------------------------------
GeometryData *Geometry::BuildTetrahedron( const TetrahedronDesc &tetrahedronDesc )
{
    const float size        = tetrahedronDesc.Size;
    const bool  rightHanded = ( tetrahedronDesc.BuildDesc & BuildDesc::RightHanded ) == BuildDesc::RightHanded;

    auto *result = new GeometryData( );

    static constexpr XMVECTORF32 verts[ 4 ] = { { { { 0.f, 0.f, 1.f, 0 } } },
                                                { { { 2.f * SQRT2 / 3.f, 0.f, -1.f / 3.f, 0 } } },
                                                { { { -SQRT2 / 3.f, SQRT6 / 3.f, -1.f / 3.f, 0 } } },
                                                { { { -SQRT2 / 3.f, -SQRT6 / 3.f, -1.f / 3.f, 0 } } } };

    static const uint32_t faces[ 4 * 3 ] = {
        0, 1, 2, 0, 2, 3, 0, 3, 1, 1, 3, 2,
    };

    // Calculate sizes
    constexpr size_t vertexCount = 4 * 3; // 4 faces * 3 vertices per face
    constexpr size_t indexCount  = 4 * 3; // 4 faces * 3 indices per face
    constexpr size_t arenaSize   = sizeof( GeometryVertexData ) * vertexCount + sizeof( uint32_t ) * indexCount + 1024;
    result->_Arena.EnsureCapacity( arenaSize );

    // Allocate arrays
    result->Vertices.Elements    = DZArenaAllocator<GeometryVertexData>::AllocateAndConstruct( result->_Arena, vertexCount );
    result->Vertices.NumElements = static_cast<uint32_t>( vertexCount );
    result->Indices.Elements     = DZArenaAllocator<uint32_t>::Allocate( result->_Arena, indexCount );
    result->Indices.NumElements  = static_cast<uint32_t>( indexCount );

    size_t vertexIndex = 0;
    size_t indexIndex  = 0;

    for ( size_t j = 0; j < std::size( faces ); j += 3 )
    {
        const uint32_t v0 = faces[ j ];
        const uint32_t v1 = faces[ j + 1 ];
        const uint32_t v2 = faces[ j + 2 ];

        XMVECTOR normal = XMVector3Cross( XMVectorSubtract( verts[ v1 ].v, verts[ v0 ].v ), XMVectorSubtract( verts[ v2 ].v, verts[ v0 ].v ) );
        normal          = XMVector3Normalize( normal );

        const size_t base = vertexIndex;
        SetIndex( result->Indices.Elements, indexIndex++, base );
        SetIndex( result->Indices.Elements, indexIndex++, base + 1 );
        SetIndex( result->Indices.Elements, indexIndex++, base + 2 );

        // Duplicate vertices to use face normals
        XMVECTOR position = XMVectorScale( verts[ v0 ], size );
        SetVertex( result->Vertices.Elements, vertexIndex++, position, normal, g_XMZero /* 0, 0 */ );

        position = XMVectorScale( verts[ v1 ], size );
        SetVertex( result->Vertices.Elements, vertexIndex++, position, normal, g_XMIdentityR0 /* 1, 0 */ );

        position = XMVectorScale( verts[ v2 ], size );
        SetVertex( result->Vertices.Elements, vertexIndex++, position, normal, g_XMIdentityR1 /* 0, 1 */ );
    }

    // Built LH above
    if ( rightHanded )
    {
        ReverseWinding( *result );
    }

    assert( result->Vertices.NumElements == 4 * 3 );
    assert( result->Indices.NumElements == 4 * 3 );
    return result;
}

//--------------------------------------------------------------------------------------
// Octahedron
//--------------------------------------------------------------------------------------
GeometryData *Geometry::BuildOctahedron( const OctahedronDesc &octahedronDesc )
{
    const float size        = octahedronDesc.Size;
    const bool  rightHanded = ( octahedronDesc.BuildDesc & BuildDesc::RightHanded ) == BuildDesc::RightHanded;

    auto *result = new GeometryData( );

    static const XMVECTORF32 verts[ 6 ] = { { { { 1, 0, 0, 0 } } },  { { { -1, 0, 0, 0 } } }, { { { 0, 1, 0, 0 } } },
                                            { { { 0, -1, 0, 0 } } }, { { { 0, 0, 1, 0 } } },  { { { 0, 0, -1, 0 } } } };

    static const uint32_t faces[ 8 * 3 ] = { 4, 0, 2, 4, 2, 1, 4, 1, 3, 4, 3, 0, 5, 2, 0, 5, 1, 2, 5, 3, 1, 5, 0, 3 };

    // Calculate sizes
    constexpr size_t vertexCount = 8 * 3; // 8 faces * 3 vertices per face
    constexpr size_t indexCount  = 8 * 3; // 8 faces * 3 indices per face
    constexpr size_t arenaSize   = sizeof( GeometryVertexData ) * vertexCount + sizeof( uint32_t ) * indexCount + 1024;
    result->_Arena.EnsureCapacity( arenaSize );

    // Allocate arrays
    result->Vertices.Elements    = DZArenaAllocator<GeometryVertexData>::AllocateAndConstruct( result->_Arena, vertexCount );
    result->Vertices.NumElements = static_cast<uint32_t>( vertexCount );
    result->Indices.Elements     = DZArenaAllocator<uint32_t>::Allocate( result->_Arena, indexCount );
    result->Indices.NumElements  = static_cast<uint32_t>( indexCount );

    size_t vertexIndex = 0;
    size_t indexIndex  = 0;

    for ( size_t j = 0; j < std::size( faces ); j += 3 )
    {
        const uint32_t v0 = faces[ j ];
        const uint32_t v1 = faces[ j + 1 ];
        const uint32_t v2 = faces[ j + 2 ];

        XMVECTOR normal = XMVector3Cross( XMVectorSubtract( verts[ v1 ].v, verts[ v0 ].v ), XMVectorSubtract( verts[ v2 ].v, verts[ v0 ].v ) );
        normal          = XMVector3Normalize( normal );

        const size_t base = vertexIndex;
        SetIndex( result->Indices.Elements, indexIndex++, base );
        SetIndex( result->Indices.Elements, indexIndex++, base + 1 );
        SetIndex( result->Indices.Elements, indexIndex++, base + 2 );

        // Duplicate vertices to use face normals
        XMVECTOR position = XMVectorScale( verts[ v0 ], size );
        SetVertex( result->Vertices.Elements, vertexIndex++, position, normal, g_XMZero /* 0, 0 */ );

        position = XMVectorScale( verts[ v1 ], size );
        SetVertex( result->Vertices.Elements, vertexIndex++, position, normal, g_XMIdentityR0 /* 1, 0 */ );

        position = XMVectorScale( verts[ v2 ], size );
        SetVertex( result->Vertices.Elements, vertexIndex++, position, normal, g_XMIdentityR1 /* 0, 1*/ );
    }

    // Built LH above
    if ( rightHanded )
    {
        ReverseWinding( *result );
    }

    assert( result->Vertices.NumElements == 8 * 3 );
    assert( result->Indices.NumElements == 8 * 3 );
    return result;
}

//--------------------------------------------------------------------------------------
// Dodecahedron
//--------------------------------------------------------------------------------------
GeometryData *Geometry::BuildDodecahedron( const DodecahedronDesc &dodecahedronDesc )
{
    const float size        = dodecahedronDesc.Size;
    const bool  rightHanded = ( dodecahedronDesc.BuildDesc & BuildDesc::RightHanded ) == BuildDesc::RightHanded;

    auto *result = new GeometryData( );

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

    // Calculate sizes
    constexpr size_t vertexCount = 12 * 5;     // 12 faces * 5 vertices per face
    constexpr size_t indexCount  = 12 * 3 * 3; // 12 faces * 3 triangles * 3 indices
    constexpr size_t arenaSize   = sizeof( GeometryVertexData ) * vertexCount + sizeof( uint32_t ) * indexCount + 1024;
    result->_Arena.EnsureCapacity( arenaSize );

    // Allocate arrays
    result->Vertices.Elements    = DZArenaAllocator<GeometryVertexData>::AllocateAndConstruct( result->_Arena, vertexCount );
    result->Vertices.NumElements = static_cast<uint32_t>( vertexCount );
    result->Indices.Elements     = DZArenaAllocator<uint32_t>::Allocate( result->_Arena, indexCount );
    result->Indices.NumElements  = static_cast<uint32_t>( indexCount );

    size_t vertexIndex = 0;
    size_t indexIndex  = 0;
    size_t t           = 0;

    for ( size_t j = 0; j < std::size( faces ); j += 5, ++t )
    {
        const uint32_t v0 = faces[ j ];
        const uint32_t v1 = faces[ j + 1 ];
        const uint32_t v2 = faces[ j + 2 ];
        const uint32_t v3 = faces[ j + 3 ];
        const uint32_t v4 = faces[ j + 4 ];

        XMVECTOR normal = XMVector3Cross( XMVectorSubtract( verts[ v1 ].v, verts[ v0 ].v ), XMVectorSubtract( verts[ v2 ].v, verts[ v0 ].v ) );
        normal          = XMVector3Normalize( normal );

        const size_t base = vertexIndex;

        SetIndex( result->Indices.Elements, indexIndex++, base );
        SetIndex( result->Indices.Elements, indexIndex++, base + 1 );
        SetIndex( result->Indices.Elements, indexIndex++, base + 2 );

        SetIndex( result->Indices.Elements, indexIndex++, base );
        SetIndex( result->Indices.Elements, indexIndex++, base + 2 );
        SetIndex( result->Indices.Elements, indexIndex++, base + 3 );

        SetIndex( result->Indices.Elements, indexIndex++, base );
        SetIndex( result->Indices.Elements, indexIndex++, base + 3 );
        SetIndex( result->Indices.Elements, indexIndex++, base + 4 );

        // Duplicate vertices to use face normals
        XMVECTOR position = XMVectorScale( verts[ v0 ], size );
        SetVertex( result->Vertices.Elements, vertexIndex++, position, normal, textureCoordinates[ textureIndex[ t ][ 0 ] ] );

        position = XMVectorScale( verts[ v1 ], size );
        SetVertex( result->Vertices.Elements, vertexIndex++, position, normal, textureCoordinates[ textureIndex[ t ][ 1 ] ] );

        position = XMVectorScale( verts[ v2 ], size );
        SetVertex( result->Vertices.Elements, vertexIndex++, position, normal, textureCoordinates[ textureIndex[ t ][ 2 ] ] );

        position = XMVectorScale( verts[ v3 ], size );
        SetVertex( result->Vertices.Elements, vertexIndex++, position, normal, textureCoordinates[ textureIndex[ t ][ 3 ] ] );

        position = XMVectorScale( verts[ v4 ], size );
        SetVertex( result->Vertices.Elements, vertexIndex++, position, normal, textureCoordinates[ textureIndex[ t ][ 4 ] ] );
    }

    // Built LH above
    if ( rightHanded )
    {
        ReverseWinding( *result );
    }

    assert( result->Vertices.NumElements == 12 * 5 );
    assert( result->Indices.NumElements == 12 * 3 * 3 );

    return result;
}

//--------------------------------------------------------------------------------------
// Icosahedron
//--------------------------------------------------------------------------------------
GeometryData *Geometry::BuildIcosahedron( const IcosahedronDesc &desc )
{
    const float size        = desc.Size;
    const bool  rightHanded = ( desc.BuildDesc & BuildDesc::RightHanded ) == BuildDesc::RightHanded;

    auto *result = new GeometryData( );

    constexpr float t  = 1.618033988749894848205f; // (1 + sqrt(5)) / 2
    constexpr float t2 = 1.519544995837552493271f; // sqrt( 1 + sqr( (1 + sqrt(5)) / 2 ) )

    static const XMVECTORF32 verts[ 12 ] = { { { { t / t2, 1.f / t2, 0, 0 } } },   { { { -t / t2, 1.f / t2, 0, 0 } } },  { { { t / t2, -1.f / t2, 0, 0 } } },
                                             { { { -t / t2, -1.f / t2, 0, 0 } } }, { { { 1.f / t2, 0, t / t2, 0 } } },   { { { 1.f / t2, 0, -t / t2, 0 } } },
                                             { { { -1.f / t2, 0, t / t2, 0 } } },  { { { -1.f / t2, 0, -t / t2, 0 } } }, { { { 0, t / t2, 1.f / t2, 0 } } },
                                             { { { 0, -t / t2, 1.f / t2, 0 } } },  { { { 0, t / t2, -1.f / t2, 0 } } },  { { { 0, -t / t2, -1.f / t2, 0 } } } };

    static const uint32_t faces[ 20 * 3 ] = { 0, 8, 4,  0, 5,  10, 2, 4, 9, 2, 11, 5, 1, 6, 8, 1, 10, 7, 3, 9, 6, 3, 7, 11, 0,  10, 8, 1,  8, 10,
                                              2, 9, 11, 3, 11, 9,  4, 2, 0, 5, 0,  2, 6, 1, 3, 7, 3,  1, 8, 6, 4, 9, 4, 6,  10, 5,  7, 11, 7, 5 };

    // Calculate sizes
    constexpr size_t vertexCount = 20 * 3; // 20 faces * 3 vertices per face
    constexpr size_t indexCount  = 20 * 3; // 20 faces * 3 indices per face
    constexpr size_t arenaSize   = sizeof( GeometryVertexData ) * vertexCount + sizeof( uint32_t ) * indexCount + 1024;
    result->_Arena.EnsureCapacity( arenaSize );

    // Allocate arrays
    result->Vertices.Elements    = DZArenaAllocator<GeometryVertexData>::AllocateAndConstruct( result->_Arena, vertexCount );
    result->Vertices.NumElements = static_cast<uint32_t>( vertexCount );
    result->Indices.Elements     = DZArenaAllocator<uint32_t>::Allocate( result->_Arena, indexCount );
    result->Indices.NumElements  = static_cast<uint32_t>( indexCount );

    size_t vertexIndex = 0;
    size_t indexIndex  = 0;

    for ( size_t j = 0; j < std::size( faces ); j += 3 )
    {
        const uint32_t v0 = faces[ j ];
        const uint32_t v1 = faces[ j + 1 ];
        const uint32_t v2 = faces[ j + 2 ];

        XMVECTOR normal = XMVector3Cross( XMVectorSubtract( verts[ v1 ].v, verts[ v0 ].v ), XMVectorSubtract( verts[ v2 ].v, verts[ v0 ].v ) );
        normal          = XMVector3Normalize( normal );

        const size_t base = vertexIndex;
        SetIndex( result->Indices.Elements, indexIndex++, base );
        SetIndex( result->Indices.Elements, indexIndex++, base + 1 );
        SetIndex( result->Indices.Elements, indexIndex++, base + 2 );

        // Duplicate vertices to use face normals
        XMVECTOR position = XMVectorScale( verts[ v0 ], size );
        SetVertex( result->Vertices.Elements, vertexIndex++, position, normal, g_XMZero /* 0, 0 */ );

        position = XMVectorScale( verts[ v1 ], size );
        SetVertex( result->Vertices.Elements, vertexIndex++, position, normal, g_XMIdentityR0 /* 1, 0 */ );

        position = XMVectorScale( verts[ v2 ], size );
        SetVertex( result->Vertices.Elements, vertexIndex++, position, normal, g_XMIdentityR1 /* 0, 1 */ );
    }

    // Built LH above
    if ( rightHanded )
    {
        ReverseWinding( *result );
    }

    assert( result->Vertices.NumElements == 20 * 3 );
    assert( result->Indices.NumElements == 20 * 3 );
    return result;
}
