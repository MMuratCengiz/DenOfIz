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
#include <cstdlib>
#include <cstring>
#include <map>
#include <stdexcept>
#include <vector>
#include "DenOfIzGraphics/Data/BatchResourceCopy.h"

using namespace DirectX;

constexpr float SQRT2 = 1.41421356237309504880f;
constexpr float SQRT3 = 1.73205080756887729352f;
constexpr float SQRT6 = 2.44948974278317809820f;

typedef struct DenOfIz_GeometryData_T
{
    std::vector<DenOfIz_GeometryVertexData> Vertices;
    std::vector<uint32_t>                   Indices;
    DenOfIz_GeometryVertexDataArray         VerticesArray;
    DenOfIz_UInt32Array                     IndicesArray;

    void UpdateArrays( )
    {
        VerticesArray.Elements    = Vertices.empty( ) ? nullptr : Vertices.data( );
        VerticesArray.NumElements = static_cast<uint32_t>( Vertices.size( ) );
        IndicesArray.Elements     = Indices.empty( ) ? nullptr : Indices.data( );
        IndicesArray.NumElements  = static_cast<uint32_t>( Indices.size( ) );
    }
} DenOfIz_GeometryData_T;

inline void CheckIndexOverflow( const size_t value )
{
    if ( value >= 65535u )
    {
        throw std::out_of_range( "Index value out of range: cannot tessellate primitive so finely" );
    }
}

void SetVertex( DenOfIz_GeometryVertexData *vertices, const size_t index, FXMVECTOR position, FXMVECTOR normal, FXMVECTOR textureCoordinate )
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

inline void ReverseWinding( DenOfIz_GeometryData_T *data )
{
    for ( uint32_t i = 0; i < data->IndicesArray.NumElements; i += 3 )
    {
        const uint32_t temp                  = data->IndicesArray.Elements[ i ];
        data->IndicesArray.Elements[ i ]     = data->IndicesArray.Elements[ i + 2 ];
        data->IndicesArray.Elements[ i + 2 ] = temp;
    }

    for ( uint32_t i = 0; i < data->VerticesArray.NumElements; ++i )
    {
        data->VerticesArray.Elements[ i ].TextureCoordinate.U = 1.f - data->VerticesArray.Elements[ i ].TextureCoordinate.U;
    }
}

inline void InvertNormals( DenOfIz_GeometryData_T *data )
{
    for ( uint32_t i = 0; i < data->VerticesArray.NumElements; ++i )
    {
        auto &it    = data->VerticesArray.Elements[ i ];
        it.Normal.X = -it.Normal.X;
        it.Normal.Y = -it.Normal.Y;
        it.Normal.Z = -it.Normal.Z;
    }
}

void DenOfIz_GeometryData_GetVertexCount( DenOfIz_GeometryData geometryData, uint32_t *outCount )
{
    DenOfIz_GeometryData_T *data = DENOFIZ_FROM_HANDLE( DenOfIz_GeometryData_T, geometryData );
    *outCount                    = data->VerticesArray.NumElements;
}

void DenOfIz_GeometryData_GetIndexCount( DenOfIz_GeometryData geometryData, uint32_t *outCount )
{
    DenOfIz_GeometryData_T *data = DENOFIZ_FROM_HANDLE( DenOfIz_GeometryData_T, geometryData );
    *outCount                    = data->IndicesArray.NumElements;
}

void DenOfIz_GeometryData_GetVertexData( DenOfIz_GeometryData geometryData, void *outData )
{
    DenOfIz_GeometryData_T *data = DENOFIZ_FROM_HANDLE( DenOfIz_GeometryData_T, geometryData );
    if ( outData != nullptr && data->VerticesArray.Elements != nullptr )
    {
        memcpy( outData, data->VerticesArray.Elements, data->VerticesArray.NumElements * sizeof( DenOfIz_GeometryVertexData ) );
    }
}

void DenOfIz_GeometryData_GetIndexData( DenOfIz_GeometryData geometryData, void *outData )
{
    DenOfIz_GeometryData_T *data = DENOFIZ_FROM_HANDLE( DenOfIz_GeometryData_T, geometryData );
    if ( outData != nullptr && data->IndicesArray.Elements != nullptr )
    {
        memcpy( outData, data->IndicesArray.Elements, data->IndicesArray.NumElements * sizeof( uint32_t ) );
    }
}

void DenOfIz_GeometryData_Destroy( DenOfIz_GeometryData geometryData )
{
    if ( !DENOFIZ_HANDLE_IS_VALID( geometryData ) )
    {
        return;
    }

    DenOfIz_GeometryData_T *data = DENOFIZ_FROM_HANDLE( DenOfIz_GeometryData_T, geometryData );
    delete data;
}

void DenOfIz_Geometry_BuildQuadXY( const DenOfIz_QuadDesc *quadDesc, DenOfIz_GeometryData *outGeometryData )
{
    const bool rightHanded   = ( quadDesc->BuildDesc & DENOFIZ_BUILD_DESC_RIGHT_HANDED ) == DENOFIZ_BUILD_DESC_RIGHT_HANDED;
    const bool invertNormals = ( quadDesc->BuildDesc & DENOFIZ_BUILD_DESC_INVERT_NORMALS ) == DENOFIZ_BUILD_DESC_INVERT_NORMALS;

    auto *result = new DenOfIz_GeometryData_T( );

    constexpr size_t vertexCount = 4;
    constexpr size_t indexCount  = 6;

    result->Vertices.resize( vertexCount );
    result->Indices.resize( indexCount );
    result->UpdateArrays( );

    const float halfWidth  = quadDesc->Width / 2.0f;
    const float halfHeight = quadDesc->Height / 2.0f;

    const XMVECTOR positions[ 4 ] = { XMVectorSet( -halfWidth, halfHeight, 0.0f, 0.0f ), XMVectorSet( -halfWidth, -halfHeight, 0.0f, 0.0f ),
                                      XMVectorSet( halfWidth, -halfHeight, 0.0f, 0.0f ), XMVectorSet( halfWidth, halfHeight, 0.0f, 0.0f ) };

    const XMVECTOR texCoords[ 4 ] = { XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f ), XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ), XMVectorSet( 1.0f, 1.0f, 0.0f, 0.0f ),
                                      XMVectorSet( 1.0f, 0.0f, 0.0f, 0.0f ) };

    const XMVECTOR normal = rightHanded ? g_XMIdentityR2 : g_XMNegIdentityR2;

    for ( size_t i = 0; i < 4; ++i )
    {
        SetVertex( result->VerticesArray.Elements, i, positions[ i ], normal, texCoords[ i ] );
    }

    SetIndex( result->IndicesArray.Elements, 0, 0 );
    SetIndex( result->IndicesArray.Elements, 1, 1 );
    SetIndex( result->IndicesArray.Elements, 2, 2 );
    SetIndex( result->IndicesArray.Elements, 3, 0 );
    SetIndex( result->IndicesArray.Elements, 4, 2 );
    SetIndex( result->IndicesArray.Elements, 5, 3 );

    if ( !rightHanded )
    {
        ReverseWinding( result );
    }

    if ( invertNormals )
    {
        InvertNormals( result );
    }

    *outGeometryData = DENOFIZ_TO_HANDLE( result );
}

void DenOfIz_Geometry_BuildQuadXZ( const DenOfIz_QuadDesc *quadDesc, DenOfIz_GeometryData *outGeometryData )
{
    const bool rightHanded   = ( quadDesc->BuildDesc & DENOFIZ_BUILD_DESC_RIGHT_HANDED ) == DENOFIZ_BUILD_DESC_RIGHT_HANDED;
    const bool invertNormals = ( quadDesc->BuildDesc & DENOFIZ_BUILD_DESC_INVERT_NORMALS ) == DENOFIZ_BUILD_DESC_INVERT_NORMALS;

    auto *result = new DenOfIz_GeometryData_T( );

    constexpr size_t vertexCount = 4;
    constexpr size_t indexCount  = 6;

    result->Vertices.resize( vertexCount );
    result->Indices.resize( indexCount );
    result->UpdateArrays( );

    const float halfWidth = quadDesc->Width / 2.0f;
    const float halfDepth = quadDesc->Height / 2.0f;

    const XMVECTOR positions[ 4 ] = { XMVectorSet( -halfWidth, 0.0f, halfDepth, 0.0f ), XMVectorSet( -halfWidth, 0.0f, -halfDepth, 0.0f ),
                                      XMVectorSet( halfWidth, 0.0f, -halfDepth, 0.0f ), XMVectorSet( halfWidth, 0.0f, halfDepth, 0.0f ) };

    const XMVECTOR texCoords[ 4 ] = { XMVectorSet( 0.0f, 0.0f, 0.0f, 0.0f ), XMVectorSet( 0.0f, 1.0f, 0.0f, 0.0f ), XMVectorSet( 1.0f, 1.0f, 0.0f, 0.0f ),
                                      XMVectorSet( 1.0f, 0.0f, 0.0f, 0.0f ) };

    const XMVECTOR normal = g_XMIdentityR1;

    for ( size_t i = 0; i < 4; ++i )
    {
        SetVertex( result->VerticesArray.Elements, i, positions[ i ], normal, texCoords[ i ] );
    }

    SetIndex( result->IndicesArray.Elements, 0, 0 );
    SetIndex( result->IndicesArray.Elements, 1, 1 );
    SetIndex( result->IndicesArray.Elements, 2, 2 );
    SetIndex( result->IndicesArray.Elements, 3, 0 );
    SetIndex( result->IndicesArray.Elements, 4, 2 );
    SetIndex( result->IndicesArray.Elements, 5, 3 );

    if ( !rightHanded )
    {
        ReverseWinding( result );
    }

    if ( invertNormals )
    {
        InvertNormals( result );
    }

    *outGeometryData = DENOFIZ_TO_HANDLE( result );
}

void DenOfIz_Geometry_BuildBox( const DenOfIz_BoxDesc *desc, DenOfIz_GeometryData *outGeometryData )
{
    const XMFLOAT3 size( desc->Width, desc->Height, desc->Depth );
    const bool     rightHanded   = ( desc->BuildDesc & DENOFIZ_BUILD_DESC_RIGHT_HANDED ) == DENOFIZ_BUILD_DESC_RIGHT_HANDED;
    const bool     invertNormals = ( desc->BuildDesc & DENOFIZ_BUILD_DESC_INVERT_NORMALS ) == DENOFIZ_BUILD_DESC_INVERT_NORMALS;

    auto *result = new DenOfIz_GeometryData_T( );

    constexpr int    FaceCount   = 6;
    constexpr size_t vertexCount = 24;
    constexpr size_t indexCount  = 36;
    result->Vertices.resize( vertexCount );
    result->Indices.resize( indexCount );
    result->UpdateArrays( );

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

    for ( int i = 0; i < FaceCount; i++ )
    {
        const XMVECTOR normal = faceNormals[ i ];
        const XMVECTOR basis  = i >= 4 ? g_XMIdentityR2 : g_XMIdentityR1;
        const XMVECTOR side1  = XMVector3Cross( normal, basis );
        const XMVECTOR side2  = XMVector3Cross( normal, side1 );
        const size_t   vbase  = vertexIndex;
        SetIndex( result->IndicesArray.Elements, indexIndex++, vbase + 0 );
        SetIndex( result->IndicesArray.Elements, indexIndex++, vbase + 1 );
        SetIndex( result->IndicesArray.Elements, indexIndex++, vbase + 2 );
        SetIndex( result->IndicesArray.Elements, indexIndex++, vbase + 0 );
        SetIndex( result->IndicesArray.Elements, indexIndex++, vbase + 2 );
        SetIndex( result->IndicesArray.Elements, indexIndex++, vbase + 3 );

        SetVertex( result->VerticesArray.Elements, vertexIndex++, XMVectorMultiply( XMVectorSubtract( XMVectorSubtract( normal, side1 ), side2 ), tsize ), normal,
                   textureCoordinates[ 0 ] );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, XMVectorMultiply( XMVectorAdd( XMVectorSubtract( normal, side1 ), side2 ), tsize ), normal,
                   textureCoordinates[ 1 ] );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, XMVectorMultiply( XMVectorAdd( normal, XMVectorAdd( side1, side2 ) ), tsize ), normal, textureCoordinates[ 2 ] );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, XMVectorMultiply( XMVectorSubtract( XMVectorAdd( normal, side1 ), side2 ), tsize ), normal,
                   textureCoordinates[ 3 ] );
    }

    if ( !rightHanded )
    {
        ReverseWinding( result );
    }

    if ( invertNormals )
    {
        InvertNormals( result );
    }

    *outGeometryData = DENOFIZ_TO_HANDLE( result );
}

void DenOfIz_Geometry_BuildSphere( const DenOfIz_SphereDesc *desc, DenOfIz_GeometryData *outGeometryData )
{
    const float  diameter      = desc->Diameter;
    const size_t tessellation  = desc->Tessellation;
    const bool   rightHanded   = ( desc->BuildDesc & DENOFIZ_BUILD_DESC_RIGHT_HANDED ) == DENOFIZ_BUILD_DESC_RIGHT_HANDED;
    const bool   invertNormals = ( desc->BuildDesc & DENOFIZ_BUILD_DESC_INVERT_NORMALS ) == DENOFIZ_BUILD_DESC_INVERT_NORMALS;

    if ( tessellation < 3 )
    {
        throw std::invalid_argument( "tesselation parameter must be at least 3" );
    }

    auto *result = new DenOfIz_GeometryData_T( );

    const size_t verticalSegments   = tessellation;
    const size_t horizontalSegments = tessellation * 2;
    const size_t vertexCount        = ( verticalSegments + 1 ) * ( horizontalSegments + 1 );
    const size_t indexCount         = verticalSegments * ( horizontalSegments + 1 ) * 6;

    result->Vertices.resize( vertexCount );
    result->Indices.resize( indexCount );
    result->UpdateArrays( );

    const float radius      = diameter / 2;
    size_t      vertexIndex = 0;

    for ( size_t i = 0; i <= verticalSegments; i++ )
    {
        const float v        = 1 - static_cast<float>( i ) / static_cast<float>( verticalSegments );
        const float latitude = static_cast<float>( i ) * XM_PI / static_cast<float>( verticalSegments ) - XM_PIDIV2;
        float       dy, dxz;
        XMScalarSinCos( &dy, &dxz, latitude );

        for ( size_t j = 0; j <= horizontalSegments; j++ )
        {
            const float u         = static_cast<float>( j ) / static_cast<float>( horizontalSegments );
            const float longitude = static_cast<float>( j ) * XM_2PI / static_cast<float>( horizontalSegments );
            float       dx, dz;
            XMScalarSinCos( &dx, &dz, longitude );
            dx *= dxz;
            dz *= dxz;
            const XMVECTOR normal            = XMVectorSet( dx, dy, dz, 0 );
            const XMVECTOR textureCoordinate = XMVectorSet( u, v, 0, 0 );
            SetVertex( result->VerticesArray.Elements, vertexIndex++, XMVectorScale( normal, radius ), normal, textureCoordinate );
        }
    }

    const size_t stride     = horizontalSegments + 1;
    size_t       indexIndex = 0;

    for ( size_t i = 0; i < verticalSegments; i++ )
    {
        for ( size_t j = 0; j <= horizontalSegments; j++ )
        {
            const size_t nextI = i + 1;
            const size_t nextJ = ( j + 1 ) % stride;
            SetIndex( result->IndicesArray.Elements, indexIndex++, i * stride + j );
            SetIndex( result->IndicesArray.Elements, indexIndex++, nextI * stride + j );
            SetIndex( result->IndicesArray.Elements, indexIndex++, i * stride + nextJ );
            SetIndex( result->IndicesArray.Elements, indexIndex++, i * stride + nextJ );
            SetIndex( result->IndicesArray.Elements, indexIndex++, nextI * stride + j );
            SetIndex( result->IndicesArray.Elements, indexIndex++, nextI * stride + nextJ );
        }
    }

    if ( !rightHanded )
    {
        ReverseWinding( result );
    }

    if ( invertNormals )
    {
        InvertNormals( result );
    }

    *outGeometryData = DENOFIZ_TO_HANDLE( result );
}

void DenOfIz_Geometry_BuildGeoSphere( const DenOfIz_GeoSphereDesc *desc, DenOfIz_GeometryData *outGeometryData )
{
    float  diameter     = desc->Diameter;
    size_t tessellation = desc->Tessellation;
    bool   rightHanded  = ( desc->BuildDesc & DENOFIZ_BUILD_DESC_RIGHT_HANDED ) == DENOFIZ_BUILD_DESC_RIGHT_HANDED;
    auto  *result       = new DenOfIz_GeometryData_T( );

    using UndirectedEdge = std::pair<uint16_t, uint16_t>;

    auto makeUndirectedEdge = []( const uint16_t a, const uint16_t b ) noexcept { return std::make_pair( std::max( a, b ), std::min( a, b ) ); };

    using EdgeSubdivisionMap = std::map<UndirectedEdge, uint16_t>;

    static constexpr XMFLOAT3 OctahedronVertices[] = {
        XMFLOAT3( 0, 1, 0 ), XMFLOAT3( 0, 0, -1 ), XMFLOAT3( 1, 0, 0 ), XMFLOAT3( 0, 0, 1 ), XMFLOAT3( -1, 0, 0 ), XMFLOAT3( 0, -1, 0 ),
    };
    static const uint16_t OctahedronIndices[] = {
        0, 1, 2, 0, 2, 3, 0, 3, 4, 0, 4, 1, 5, 1, 4, 5, 4, 3, 5, 3, 2, 5, 2, 1,
    };

    const float radius = diameter / 2.0f;

    std::vector<XMFLOAT3> vertexPositions( std::begin( OctahedronVertices ), std::end( OctahedronVertices ) );
    std::vector<uint32_t> indices;
    indices.reserve( 1024 );

    for ( auto &octahedronIndex : OctahedronIndices )
    {
        indices.push_back( octahedronIndex );
    }

    constexpr uint16_t northPoleIndex = 0;
    constexpr uint16_t southPoleIndex = 5;

    for ( size_t iSubdivision = 0; iSubdivision < tessellation; ++iSubdivision )
    {
        EdgeSubdivisionMap    subdividedEdges;
        std::vector<uint32_t> newIndices;
        newIndices.reserve( indices.size( ) * 4 );

        const size_t triangleCount = indices.size( ) / 3;
        for ( size_t iTriangle = 0; iTriangle < triangleCount; ++iTriangle )
        {
            const uint16_t iv0 = indices[ iTriangle * 3 + 0 ];
            const uint16_t iv1 = indices[ iTriangle * 3 + 1 ];
            const uint16_t iv2 = indices[ iTriangle * 3 + 2 ];

            XMFLOAT3 v01{ };
            XMFLOAT3 v12{ };
            XMFLOAT3 v20{ };
            uint16_t iv01;
            uint16_t iv12;
            uint16_t iv20;

            auto const divideEdge = [ & ]( const uint16_t i0, const uint16_t i1, XMFLOAT3 &outVertex, uint16_t &outIndex )
            {
                const UndirectedEdge edge = makeUndirectedEdge( i0, i1 );

                if ( const auto it = subdividedEdges.find( edge ); it != subdividedEdges.end( ) )
                {
                    outIndex  = it->second;
                    outVertex = vertexPositions[ outIndex ];
                }
                else
                {
                    XMStoreFloat3( &outVertex, XMVectorScale( XMVectorAdd( XMLoadFloat3( &vertexPositions[ i0 ] ), XMLoadFloat3( &vertexPositions[ i1 ] ) ), 0.5f ) );
                    outIndex = static_cast<uint16_t>( vertexPositions.size( ) );
                    CheckIndexOverflow( outIndex );
                    vertexPositions.push_back( outVertex );
                    auto entry = std::make_pair( edge, outIndex );
                    subdividedEdges.insert( entry );
                }
            };

            divideEdge( iv0, iv1, v01, iv01 );
            divideEdge( iv1, iv2, v12, iv12 );
            divideEdge( iv0, iv2, v20, iv20 );

            const uint16_t indicesToAdd[] = {
                iv0, iv01, iv20, iv20, iv12, iv2, iv20, iv01, iv12, iv01, iv1, iv12,
            };

            for ( const auto &indexToAdd : indicesToAdd )
            {
                newIndices.push_back( indexToAdd );
            }
        }

        indices = std::move( newIndices );
    }

    std::vector<DenOfIz_GeometryVertexData> vertices;
    vertices.reserve( vertexPositions.size( ) );

    for ( const auto &it : vertexPositions )
    {
        auto const normal = XMVector3Normalize( XMLoadFloat3( &it ) );
        auto const pos    = XMVectorScale( normal, radius );

        XMFLOAT3 normalFloat3{ };
        XMStoreFloat3( &normalFloat3, normal );

        const float longitude = atan2f( normalFloat3.x, -normalFloat3.z );
        const float latitude  = acosf( normalFloat3.y );
        const float u         = longitude / XM_2PI + 0.5f;
        const float v         = latitude / XM_PI;

        DenOfIz_GeometryVertexData vertex;
        XMStoreFloat3( reinterpret_cast<XMFLOAT3 *>( &vertex.Position ), pos );
        XMStoreFloat3( reinterpret_cast<XMFLOAT3 *>( &vertex.Normal ), normal );
        vertex.TextureCoordinate.U = 1.0f - u;
        vertex.TextureCoordinate.V = v;
        vertices.push_back( vertex );
    }

    const size_t preFixupVertexCount = vertices.size( );
    for ( size_t i = 0; i < preFixupVertexCount; ++i )
    {
        const bool isOnPrimeMeridian =
            XMVector2NearEqual( XMVectorSet( vertices[ i ].Position.X, vertices[ i ].TextureCoordinate.U, 0.0f, 0.0f ), XMVectorZero( ), XMVectorSplatEpsilon( ) );

        if ( isOnPrimeMeridian )
        {
            size_t newIndex = vertices.size( );
            CheckIndexOverflow( newIndex );
            DenOfIz_GeometryVertexData v = vertices[ i ];
            v.TextureCoordinate.U        = 1.0f;
            vertices.push_back( v );

            for ( size_t j = 0; j < indices.size( ); j += 3 )
            {
                uint32_t *triIndex0 = &indices[ j + 0 ];
                uint32_t *triIndex1 = &indices[ j + 1 ];
                uint32_t *triIndex2 = &indices[ j + 2 ];

                if ( *triIndex0 == i )
                {
                }
                else if ( *triIndex1 == i )
                {
                    std::swap( triIndex0, triIndex1 );
                }
                else if ( *triIndex2 == i )
                {
                    std::swap( triIndex0, triIndex2 );
                }
                else
                {
                    continue;
                }

                const DenOfIz_GeometryVertexData &v0 = vertices[ *triIndex0 ];
                const DenOfIz_GeometryVertexData &v1 = vertices[ *triIndex1 ];
                const DenOfIz_GeometryVertexData &v2 = vertices[ *triIndex2 ];

                if ( abs( v0.TextureCoordinate.U - v1.TextureCoordinate.U ) > 0.5f || abs( v0.TextureCoordinate.U - v2.TextureCoordinate.U ) > 0.5f )
                {
                    *triIndex0 = static_cast<uint16_t>( newIndex );
                }
            }
        }
    }

    auto const fixPole = [ & ]( const size_t poleIndex )
    {
        const auto &poleVertex            = vertices[ poleIndex ];
        bool        overwrittenPoleVertex = false;

        for ( size_t i = 0; i < indices.size( ); i += 3 )
        {
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

            DenOfIz_GeometryVertexData newPoleVertex = poleVertex;
            newPoleVertex.TextureCoordinate.U        = ( otherVertex0.TextureCoordinate.U + otherVertex1.TextureCoordinate.U ) / 2;
            newPoleVertex.TextureCoordinate.V        = poleVertex.TextureCoordinate.V;

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

    const size_t finalVertexCount = vertices.size( );
    const size_t finalIndexCount  = indices.size( );

    result->Vertices.resize( finalVertexCount );
    result->Indices.resize( finalIndexCount );
    result->UpdateArrays( );

    for ( size_t i = 0; i < finalVertexCount; ++i )
    {
        result->VerticesArray.Elements[ i ] = vertices[ i ];
    }
    for ( size_t i = 0; i < finalIndexCount; ++i )
    {
        result->IndicesArray.Elements[ i ] = indices[ i ];
    }

    if ( !rightHanded )
    {
        ReverseWinding( result );
    }

    *outGeometryData = DENOFIZ_TO_HANDLE( result );
}

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

void CreateCylinderCap( DenOfIz_GeometryVertexData *vertices, size_t &vertexIndex, uint32_t *indices, size_t &indexIndex, const size_t tessellation, const float height,
                        const float radius, const bool isTop )
{
    const size_t vbase = vertexIndex;

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

    XMVECTOR normal       = g_XMIdentityR1;
    XMVECTOR textureScale = g_XMNegativeOneHalf;

    if ( !isTop )
    {
        normal       = XMVectorNegate( normal );
        textureScale = XMVectorMultiply( textureScale, g_XMNegateX );
    }

    for ( size_t i = 0; i < tessellation; i++ )
    {
        const XMVECTOR circleVector      = GetCircleVector( i, tessellation );
        const XMVECTOR position          = XMVectorAdd( XMVectorScale( circleVector, radius ), XMVectorScale( normal, height ) );
        const XMVECTOR textureCoordinate = XMVectorMultiplyAdd( XMVectorSwizzle<0, 2, 3, 3>( circleVector ), textureScale, g_XMOneHalf );
        SetVertex( vertices, vertexIndex++, position, normal, textureCoordinate );
    }
}

void DenOfIz_Geometry_BuildCylinder( const DenOfIz_CylinderDesc *desc, DenOfIz_GeometryData *outGeometryData )
{
    const float  diameter     = desc->Diameter;
    float        height       = desc->Height;
    const size_t tessellation = desc->Tessellation;
    const bool   rightHanded  = ( desc->BuildDesc & DENOFIZ_BUILD_DESC_RIGHT_HANDED ) == DENOFIZ_BUILD_DESC_RIGHT_HANDED;

    if ( tessellation < 3 )
    {
        throw std::invalid_argument( "tesselation parameter must be at least 3" );
    }

    auto *result = new DenOfIz_GeometryData_T( );

    const size_t sideVertexCount = ( tessellation + 1 ) * 2;
    const size_t capVertexCount  = tessellation * 2;
    const size_t vertexCount     = sideVertexCount + capVertexCount;
    const size_t sideIndexCount  = tessellation * 6;
    const size_t capIndexCount   = ( tessellation - 2 ) * 3 * 2;
    const size_t indexCount      = sideIndexCount + capIndexCount;

    result->Vertices.resize( vertexCount );
    result->Indices.resize( indexCount );
    result->UpdateArrays( );

    height /= 2;

    const XMVECTOR topOffset = XMVectorScale( g_XMIdentityR1, height );
    const float    radius    = diameter / 2;
    const size_t   stride    = tessellation + 1;

    size_t vertexIndex = 0;
    size_t indexIndex  = 0;

    for ( size_t i = 0; i <= tessellation; i++ )
    {
        const XMVECTOR normal            = GetCircleVector( i, tessellation );
        const XMVECTOR sideOffset        = XMVectorScale( normal, radius );
        const float    u                 = static_cast<float>( i ) / static_cast<float>( tessellation );
        const XMVECTOR textureCoordinate = XMLoadFloat( &u );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, XMVectorAdd( sideOffset, topOffset ), normal, textureCoordinate );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, XMVectorSubtract( sideOffset, topOffset ), normal, XMVectorAdd( textureCoordinate, g_XMIdentityR1 ) );

        if ( i < tessellation )
        {
            SetIndex( result->IndicesArray.Elements, indexIndex++, i * 2 );
            SetIndex( result->IndicesArray.Elements, indexIndex++, ( i * 2 + 2 ) % ( stride * 2 ) );
            SetIndex( result->IndicesArray.Elements, indexIndex++, i * 2 + 1 );
            SetIndex( result->IndicesArray.Elements, indexIndex++, i * 2 + 1 );
            SetIndex( result->IndicesArray.Elements, indexIndex++, ( i * 2 + 2 ) % ( stride * 2 ) );
            SetIndex( result->IndicesArray.Elements, indexIndex++, ( i * 2 + 3 ) % ( stride * 2 ) );
        }
    }

    CreateCylinderCap( result->VerticesArray.Elements, vertexIndex, result->IndicesArray.Elements, indexIndex, tessellation, height, radius, true );
    CreateCylinderCap( result->VerticesArray.Elements, vertexIndex, result->IndicesArray.Elements, indexIndex, tessellation, height, radius, false );

    if ( !rightHanded )
    {
        ReverseWinding( result );
    }

    *outGeometryData = DENOFIZ_TO_HANDLE( result );
}

void DenOfIz_Geometry_BuildCone( const DenOfIz_ConeDesc *desc, DenOfIz_GeometryData *outGeometryData )
{
    const float  diameter     = desc->Diameter;
    float        height       = desc->Height;
    const size_t tessellation = desc->Tessellation;
    const bool   rightHanded  = ( desc->BuildDesc & DENOFIZ_BUILD_DESC_RIGHT_HANDED ) == DENOFIZ_BUILD_DESC_RIGHT_HANDED;

    if ( tessellation < 3 )
    {
        throw std::invalid_argument( "tesselation parameter must be at least 3" );
    }

    auto *result = new DenOfIz_GeometryData_T( );

    const size_t sideVertexCount = ( tessellation + 1 ) * 2;
    const size_t capVertexCount  = tessellation;
    const size_t vertexCount     = sideVertexCount + capVertexCount;
    const size_t sideIndexCount  = tessellation * 3;
    const size_t capIndexCount   = ( tessellation - 2 ) * 3;
    const size_t indexCount      = sideIndexCount + capIndexCount;

    result->Vertices.resize( vertexCount );
    result->Indices.resize( indexCount );
    result->UpdateArrays( );

    height /= 2;

    const XMVECTOR topOffset = XMVectorScale( g_XMIdentityR1, height );
    const float    radius    = diameter / 2;
    const size_t   stride    = tessellation + 1;

    size_t vertexIndex = 0;
    size_t indexIndex  = 0;

    for ( size_t i = 0; i <= tessellation; i++ )
    {
        const XMVECTOR circle            = GetCircleVector( i, tessellation );
        const XMVECTOR sideOffset        = XMVectorScale( circle, radius );
        const float    u                 = static_cast<float>( i ) / static_cast<float>( tessellation );
        const XMVECTOR textureCoordinate = XMLoadFloat( &u );
        const XMVECTOR pt                = XMVectorSubtract( sideOffset, topOffset );
        XMVECTOR       normal            = XMVector3Cross( GetCircleTangent( i, tessellation ), XMVectorSubtract( topOffset, pt ) );
        normal                           = XMVector3Normalize( normal );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, topOffset, normal, g_XMZero );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, pt, normal, XMVectorAdd( textureCoordinate, g_XMIdentityR1 ) );

        if ( i < tessellation )
        {
            SetIndex( result->IndicesArray.Elements, indexIndex++, i * 2 );
            SetIndex( result->IndicesArray.Elements, indexIndex++, ( i * 2 + 3 ) % ( stride * 2 ) );
            SetIndex( result->IndicesArray.Elements, indexIndex++, ( i * 2 + 1 ) % ( stride * 2 ) );
        }
    }

    CreateCylinderCap( result->VerticesArray.Elements, vertexIndex, result->IndicesArray.Elements, indexIndex, tessellation, height, radius, false );

    if ( !rightHanded )
    {
        ReverseWinding( result );
    }

    *outGeometryData = DENOFIZ_TO_HANDLE( result );
}

void DenOfIz_Geometry_BuildTorus( const DenOfIz_TorusDesc *desc, DenOfIz_GeometryData *outGeometryData )
{
    const float  diameter     = desc->Diameter;
    const float  thickness    = desc->Thickness;
    const size_t tessellation = desc->Tessellation;
    const bool   rightHanded  = ( desc->BuildDesc & DENOFIZ_BUILD_DESC_RIGHT_HANDED ) == DENOFIZ_BUILD_DESC_RIGHT_HANDED;

    if ( tessellation < 3 )
    {
        throw std::invalid_argument( "tesselation parameter must be at least 3" );
    }

    auto *result = new DenOfIz_GeometryData_T( );

    const size_t stride      = tessellation + 1;
    const size_t vertexCount = stride * stride;
    const size_t indexCount  = tessellation * tessellation * 6;

    result->Vertices.resize( vertexCount );
    result->Indices.resize( indexCount );
    result->UpdateArrays( );

    size_t vertexIndex = 0;
    size_t indexIndex  = 0;

    for ( size_t i = 0; i <= tessellation; i++ )
    {
        const float    u          = static_cast<float>( i ) / static_cast<float>( tessellation );
        const float    outerAngle = static_cast<float>( i ) * XM_2PI / static_cast<float>( tessellation ) - XM_PIDIV2;
        const XMMATRIX transform  = XMMatrixTranslation( diameter / 2, 0, 0 ) * XMMatrixRotationY( outerAngle );

        for ( size_t j = 0; j <= tessellation; j++ )
        {
            const float v          = 1 - static_cast<float>( j ) / static_cast<float>( tessellation );
            const float innerAngle = static_cast<float>( j ) * XM_2PI / static_cast<float>( tessellation ) + XM_PI;
            float       dx, dy;
            XMScalarSinCos( &dy, &dx, innerAngle );
            XMVECTOR       normal            = XMVectorSet( dx, dy, 0, 0 );
            XMVECTOR       position          = XMVectorScale( normal, thickness / 2 );
            const XMVECTOR textureCoordinate = XMVectorSet( u, v, 0, 0 );
            position                         = XMVector3Transform( position, transform );
            normal                           = XMVector3TransformNormal( normal, transform );
            SetVertex( result->VerticesArray.Elements, vertexIndex++, position, normal, textureCoordinate );

            if ( i < tessellation && j < tessellation )
            {
                const size_t nextI = ( i + 1 ) % stride;
                const size_t nextJ = ( j + 1 ) % stride;
                SetIndex( result->IndicesArray.Elements, indexIndex++, i * stride + j );
                SetIndex( result->IndicesArray.Elements, indexIndex++, i * stride + nextJ );
                SetIndex( result->IndicesArray.Elements, indexIndex++, nextI * stride + j );
                SetIndex( result->IndicesArray.Elements, indexIndex++, i * stride + nextJ );
                SetIndex( result->IndicesArray.Elements, indexIndex++, nextI * stride + nextJ );
                SetIndex( result->IndicesArray.Elements, indexIndex++, nextI * stride + j );
            }
        }
    }

    if ( !rightHanded )
    {
        ReverseWinding( result );
    }

    *outGeometryData = DENOFIZ_TO_HANDLE( result );
}

void DenOfIz_Geometry_BuildTetrahedron( const DenOfIz_TetrahedronDesc *tetrahedronDesc, DenOfIz_GeometryData *outGeometryData )
{
    const float size        = tetrahedronDesc->Size;
    const bool  rightHanded = ( tetrahedronDesc->BuildDesc & DENOFIZ_BUILD_DESC_RIGHT_HANDED ) == DENOFIZ_BUILD_DESC_RIGHT_HANDED;

    auto *result = new DenOfIz_GeometryData_T( );

    static constexpr XMVECTORF32 verts[ 4 ] = { { { { 0.f, 0.f, 1.f, 0 } } },
                                                { { { 2.f * SQRT2 / 3.f, 0.f, -1.f / 3.f, 0 } } },
                                                { { { -SQRT2 / 3.f, SQRT6 / 3.f, -1.f / 3.f, 0 } } },
                                                { { { -SQRT2 / 3.f, -SQRT6 / 3.f, -1.f / 3.f, 0 } } } };

    static const uint32_t faces[ 4 * 3 ] = {
        0, 1, 2, 0, 2, 3, 0, 3, 1, 1, 3, 2,
    };

    constexpr size_t vertexCount = 4 * 3;
    constexpr size_t indexCount  = 4 * 3;
    result->Vertices.resize( vertexCount );
    result->Indices.resize( indexCount );
    result->UpdateArrays( );

    size_t vertexIndex = 0;
    size_t indexIndex  = 0;

    for ( size_t j = 0; j < std::size( faces ); j += 3 )
    {
        const uint32_t v0     = faces[ j ];
        const uint32_t v1     = faces[ j + 1 ];
        const uint32_t v2     = faces[ j + 2 ];
        XMVECTOR       normal = XMVector3Cross( XMVectorSubtract( verts[ v1 ].v, verts[ v0 ].v ), XMVectorSubtract( verts[ v2 ].v, verts[ v0 ].v ) );
        normal                = XMVector3Normalize( normal );
        const size_t base     = vertexIndex;
        SetIndex( result->IndicesArray.Elements, indexIndex++, base );
        SetIndex( result->IndicesArray.Elements, indexIndex++, base + 1 );
        SetIndex( result->IndicesArray.Elements, indexIndex++, base + 2 );
        XMVECTOR position = XMVectorScale( verts[ v0 ], size );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, position, normal, g_XMZero );
        position = XMVectorScale( verts[ v1 ], size );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, position, normal, g_XMIdentityR0 );
        position = XMVectorScale( verts[ v2 ], size );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, position, normal, g_XMIdentityR1 );
    }

    if ( rightHanded )
    {
        ReverseWinding( result );
    }

    *outGeometryData = DENOFIZ_TO_HANDLE( result );
}

void DenOfIz_Geometry_BuildOctahedron( const DenOfIz_OctahedronDesc *octahedronDesc, DenOfIz_GeometryData *outGeometryData )
{
    const float size        = octahedronDesc->Size;
    const bool  rightHanded = ( octahedronDesc->BuildDesc & DENOFIZ_BUILD_DESC_RIGHT_HANDED ) == DENOFIZ_BUILD_DESC_RIGHT_HANDED;

    auto *result = new DenOfIz_GeometryData_T( );

    static const XMVECTORF32 verts[ 6 ] = { { { { 1, 0, 0, 0 } } },  { { { -1, 0, 0, 0 } } }, { { { 0, 1, 0, 0 } } },
                                            { { { 0, -1, 0, 0 } } }, { { { 0, 0, 1, 0 } } },  { { { 0, 0, -1, 0 } } } };

    static const uint32_t faces[ 8 * 3 ] = { 4, 0, 2, 4, 2, 1, 4, 1, 3, 4, 3, 0, 5, 2, 0, 5, 1, 2, 5, 3, 1, 5, 0, 3 };

    constexpr size_t vertexCount = 8 * 3;
    constexpr size_t indexCount  = 8 * 3;
    result->Vertices.resize( vertexCount );
    result->Indices.resize( indexCount );
    result->UpdateArrays( );

    size_t vertexIndex = 0;
    size_t indexIndex  = 0;

    for ( size_t j = 0; j < std::size( faces ); j += 3 )
    {
        const uint32_t v0     = faces[ j ];
        const uint32_t v1     = faces[ j + 1 ];
        const uint32_t v2     = faces[ j + 2 ];
        XMVECTOR       normal = XMVector3Cross( XMVectorSubtract( verts[ v1 ].v, verts[ v0 ].v ), XMVectorSubtract( verts[ v2 ].v, verts[ v0 ].v ) );
        normal                = XMVector3Normalize( normal );
        const size_t base     = vertexIndex;
        SetIndex( result->IndicesArray.Elements, indexIndex++, base );
        SetIndex( result->IndicesArray.Elements, indexIndex++, base + 1 );
        SetIndex( result->IndicesArray.Elements, indexIndex++, base + 2 );
        XMVECTOR position = XMVectorScale( verts[ v0 ], size );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, position, normal, g_XMZero );
        position = XMVectorScale( verts[ v1 ], size );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, position, normal, g_XMIdentityR0 );
        position = XMVectorScale( verts[ v2 ], size );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, position, normal, g_XMIdentityR1 );
    }

    if ( rightHanded )
    {
        ReverseWinding( result );
    }

    *outGeometryData = DENOFIZ_TO_HANDLE( result );
}

void DenOfIz_Geometry_BuildDodecahedron( const DenOfIz_DodecahedronDesc *dodecahedronDesc, DenOfIz_GeometryData *outGeometryData )
{
    const float size        = dodecahedronDesc->Size;
    const bool  rightHanded = ( dodecahedronDesc->BuildDesc & DENOFIZ_BUILD_DESC_RIGHT_HANDED ) == DENOFIZ_BUILD_DESC_RIGHT_HANDED;

    auto *result = new DenOfIz_GeometryData_T( );

    constexpr float a = 1.f / SQRT3;
    constexpr float b = 0.356822089773089931942f;
    constexpr float c = 0.934172358962715696451f;

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

    constexpr size_t vertexCount = 12 * 5;
    constexpr size_t indexCount  = 12 * 3 * 3;
    result->Vertices.resize( vertexCount );
    result->Indices.resize( indexCount );
    result->UpdateArrays( );

    size_t vertexIndex = 0;
    size_t indexIndex  = 0;
    size_t t           = 0;

    for ( size_t j = 0; j < std::size( faces ); j += 5, ++t )
    {
        const uint32_t v0     = faces[ j ];
        const uint32_t v1     = faces[ j + 1 ];
        const uint32_t v2     = faces[ j + 2 ];
        const uint32_t v3     = faces[ j + 3 ];
        const uint32_t v4     = faces[ j + 4 ];
        XMVECTOR       normal = XMVector3Cross( XMVectorSubtract( verts[ v1 ].v, verts[ v0 ].v ), XMVectorSubtract( verts[ v2 ].v, verts[ v0 ].v ) );
        normal                = XMVector3Normalize( normal );
        const size_t base     = vertexIndex;
        SetIndex( result->IndicesArray.Elements, indexIndex++, base );
        SetIndex( result->IndicesArray.Elements, indexIndex++, base + 1 );
        SetIndex( result->IndicesArray.Elements, indexIndex++, base + 2 );
        SetIndex( result->IndicesArray.Elements, indexIndex++, base );
        SetIndex( result->IndicesArray.Elements, indexIndex++, base + 2 );
        SetIndex( result->IndicesArray.Elements, indexIndex++, base + 3 );
        SetIndex( result->IndicesArray.Elements, indexIndex++, base );
        SetIndex( result->IndicesArray.Elements, indexIndex++, base + 3 );
        SetIndex( result->IndicesArray.Elements, indexIndex++, base + 4 );
        XMVECTOR position = XMVectorScale( verts[ v0 ], size );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, position, normal, textureCoordinates[ textureIndex[ t ][ 0 ] ] );
        position = XMVectorScale( verts[ v1 ], size );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, position, normal, textureCoordinates[ textureIndex[ t ][ 1 ] ] );
        position = XMVectorScale( verts[ v2 ], size );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, position, normal, textureCoordinates[ textureIndex[ t ][ 2 ] ] );
        position = XMVectorScale( verts[ v3 ], size );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, position, normal, textureCoordinates[ textureIndex[ t ][ 3 ] ] );
        position = XMVectorScale( verts[ v4 ], size );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, position, normal, textureCoordinates[ textureIndex[ t ][ 4 ] ] );
    }

    if ( rightHanded )
    {
        ReverseWinding( result );
    }

    *outGeometryData = DENOFIZ_TO_HANDLE( result );
}

void DenOfIz_Geometry_BuildIcosahedron( const DenOfIz_IcosahedronDesc *desc, DenOfIz_GeometryData *outGeometryData )
{
    const float size        = desc->Size;
    const bool  rightHanded = ( desc->BuildDesc & DENOFIZ_BUILD_DESC_RIGHT_HANDED ) == DENOFIZ_BUILD_DESC_RIGHT_HANDED;

    auto *result = new DenOfIz_GeometryData_T( );

    constexpr float t  = 1.618033988749894848205f;
    constexpr float t2 = 1.519544995837552493271f;

    static const XMVECTORF32 verts[ 12 ] = { { { { t / t2, 1.f / t2, 0, 0 } } },   { { { -t / t2, 1.f / t2, 0, 0 } } },  { { { t / t2, -1.f / t2, 0, 0 } } },
                                             { { { -t / t2, -1.f / t2, 0, 0 } } }, { { { 1.f / t2, 0, t / t2, 0 } } },   { { { 1.f / t2, 0, -t / t2, 0 } } },
                                             { { { -1.f / t2, 0, t / t2, 0 } } },  { { { -1.f / t2, 0, -t / t2, 0 } } }, { { { 0, t / t2, 1.f / t2, 0 } } },
                                             { { { 0, -t / t2, 1.f / t2, 0 } } },  { { { 0, t / t2, -1.f / t2, 0 } } },  { { { 0, -t / t2, -1.f / t2, 0 } } } };

    static const uint32_t faces[ 20 * 3 ] = { 0, 8, 4,  0, 5,  10, 2, 4, 9, 2, 11, 5, 1, 6, 8, 1, 10, 7, 3, 9, 6, 3, 7, 11, 0,  10, 8, 1,  8, 10,
                                              2, 9, 11, 3, 11, 9,  4, 2, 0, 5, 0,  2, 6, 1, 3, 7, 3,  1, 8, 6, 4, 9, 4, 6,  10, 5,  7, 11, 7, 5 };

    constexpr size_t vertexCount = 20 * 3;
    constexpr size_t indexCount  = 20 * 3;
    result->Vertices.resize( vertexCount );
    result->Indices.resize( indexCount );
    result->UpdateArrays( );

    size_t vertexIndex = 0;
    size_t indexIndex  = 0;

    for ( size_t j = 0; j < std::size( faces ); j += 3 )
    {
        const uint32_t v0     = faces[ j ];
        const uint32_t v1     = faces[ j + 1 ];
        const uint32_t v2     = faces[ j + 2 ];
        XMVECTOR       normal = XMVector3Cross( XMVectorSubtract( verts[ v1 ].v, verts[ v0 ].v ), XMVectorSubtract( verts[ v2 ].v, verts[ v0 ].v ) );
        normal                = XMVector3Normalize( normal );
        const size_t base     = vertexIndex;
        SetIndex( result->IndicesArray.Elements, indexIndex++, base );
        SetIndex( result->IndicesArray.Elements, indexIndex++, base + 1 );
        SetIndex( result->IndicesArray.Elements, indexIndex++, base + 2 );
        XMVECTOR position = XMVectorScale( verts[ v0 ], size );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, position, normal, g_XMZero );
        position = XMVectorScale( verts[ v1 ], size );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, position, normal, g_XMIdentityR0 );
        position = XMVectorScale( verts[ v2 ], size );
        SetVertex( result->VerticesArray.Elements, vertexIndex++, position, normal, g_XMIdentityR1 );
    }

    if ( rightHanded )
    {
        ReverseWinding( result );
    }

    *outGeometryData = DENOFIZ_TO_HANDLE( result );
}
