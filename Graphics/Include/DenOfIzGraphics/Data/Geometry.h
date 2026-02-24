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
// File: Geometry.h
//
// Copyright (c) Microsoft Corporation.
// Licensed under the MIT License.
//
// http://go.microsoft.com/fwlink/?LinkId=248929
// http://go.microsoft.com/fwlink/?LinkID=615561
//--------------------------------------------------------------------------------------

#pragma once

#include <stdint.h>
#include "DenOfIzGraphics/Handle.h"
#include "DenOfIzGraphics/Utilities/Common_Arrays.h"
#include "DenOfIzGraphics/Utilities/Common_Macro.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum DenOfIz_BuildDesc
    {
        DENOFIZ_BUILD_DESC_BUILD_NORMAL    = 1 << 0,
        DENOFIZ_BUILD_DESC_BUILD_TANGENT   = 1 << 1,
        DENOFIZ_BUILD_DESC_BUILD_BITANGENT = 1 << 2,
        DENOFIZ_BUILD_DESC_BUILD_TEXCOORD  = 1 << 3,
        DENOFIZ_BUILD_DESC_RIGHT_HANDED    = 1 << 4,
        DENOFIZ_BUILD_DESC_INVERT_NORMALS  = 1 << 5
    } DenOfIz_BuildDesc;

    typedef struct DenOfIz_QuadDesc
    {
        uint32_t BuildDesc;
        float    Width;
        float    Height;
    } DenOfIz_QuadDesc;

    typedef struct DenOfIz_BoxDesc
    {
        uint32_t BuildDesc;
        float    Width;
        float    Height;
        float    Depth;
    } DenOfIz_BoxDesc;

    typedef struct DenOfIz_SphereDesc
    {
        uint32_t BuildDesc;
        float    Diameter;
        uint32_t Tessellation;
    } DenOfIz_SphereDesc;

    typedef struct DenOfIz_GeoSphereDesc
    {
        uint32_t BuildDesc;
        float    Diameter;
        uint32_t Tessellation;
    } DenOfIz_GeoSphereDesc;

    typedef struct DenOfIz_CylinderDesc
    {
        uint32_t BuildDesc;
        float    Diameter;
        float    Height;
        uint32_t Tessellation;
    } DenOfIz_CylinderDesc;

    typedef struct DenOfIz_ConeDesc
    {
        uint32_t BuildDesc;
        float    Diameter;
        float    Height;
        uint32_t Tessellation;
    } DenOfIz_ConeDesc;

    typedef struct DenOfIz_TorusDesc
    {
        uint32_t BuildDesc;
        float    Diameter;
        float    Thickness;
        uint32_t Tessellation;
    } DenOfIz_TorusDesc;

    typedef struct DenOfIz_TetrahedronDesc
    {
        uint32_t BuildDesc;
        float    Size;
    } DenOfIz_TetrahedronDesc;

    typedef struct DenOfIz_OctahedronDesc
    {
        uint32_t BuildDesc;
        float    Size;
    } DenOfIz_OctahedronDesc;

    typedef struct DenOfIz_DodecahedronDesc
    {
        uint32_t BuildDesc;
        float    Size;
    } DenOfIz_DodecahedronDesc;

    typedef struct DenOfIz_IcosahedronDesc
    {
        uint32_t BuildDesc;
        float    Size;
    } DenOfIz_IcosahedronDesc;

    typedef struct DenOfIz_GeometryPositionVertexData
    {
        float X;
        float Y;
        float Z;
    } DenOfIz_GeometryPositionVertexData;

    typedef struct DenOfIz_GeometryNormalVertexData
    {
        float X;
        float Y;
        float Z;
    } DenOfIz_GeometryNormalVertexData;

    typedef struct DenOfIz_GeometryTextureCoordinateVertexData
    {
        float U;
        float V;
    } DenOfIz_GeometryTextureCoordinateVertexData;

    typedef struct DenOfIz_GeometryVertexData
    {
        DenOfIz_GeometryPositionVertexData          Position;
        DenOfIz_GeometryNormalVertexData            Normal;
        DenOfIz_GeometryTextureCoordinateVertexData TextureCoordinate;
    } DenOfIz_GeometryVertexData;

    typedef struct DenOfIz_GeometryVertexDataArray
    {
        DenOfIz_GeometryVertexData *Elements;
        size_t                      NumElements;
    } DenOfIz_GeometryVertexDataArray;

    DENOFIZ_DEFINE_HANDLE( DenOfIz_GeometryData )

    DZ_API void DenOfIz_GeometryData_GetVertexCount( DenOfIz_GeometryData geometryData, uint32_t *outCount );
    DZ_API void DenOfIz_GeometryData_GetIndexCount( DenOfIz_GeometryData geometryData, uint32_t *outCount );
    DZ_API void DenOfIz_GeometryData_GetVertexData( DenOfIz_GeometryData geometryData, void *outData );
    DZ_API void DenOfIz_GeometryData_GetIndexData( DenOfIz_GeometryData geometryData, void *outData );
    DZ_API void DenOfIz_GeometryData_Destroy( DenOfIz_GeometryData geometryData );
    DZ_API void DenOfIz_Geometry_BuildQuadXY( const DenOfIz_QuadDesc *quadDesc, DenOfIz_GeometryData *outGeometryData );
    DZ_API void DenOfIz_Geometry_BuildQuadXZ( const DenOfIz_QuadDesc *quadDesc, DenOfIz_GeometryData *outGeometryData );
    DZ_API void DenOfIz_Geometry_BuildBox( const DenOfIz_BoxDesc *desc, DenOfIz_GeometryData *outGeometryData );
    DZ_API void DenOfIz_Geometry_BuildSphere( const DenOfIz_SphereDesc *desc, DenOfIz_GeometryData *outGeometryData );
    DZ_API void DenOfIz_Geometry_BuildGeoSphere( const DenOfIz_GeoSphereDesc *desc, DenOfIz_GeometryData *outGeometryData );
    DZ_API void DenOfIz_Geometry_BuildCylinder( const DenOfIz_CylinderDesc *desc, DenOfIz_GeometryData *outGeometryData );
    DZ_API void DenOfIz_Geometry_BuildCone( const DenOfIz_ConeDesc *desc, DenOfIz_GeometryData *outGeometryData );
    DZ_API void DenOfIz_Geometry_BuildTorus( const DenOfIz_TorusDesc *desc, DenOfIz_GeometryData *outGeometryData );
    DZ_API void DenOfIz_Geometry_BuildTetrahedron( const DenOfIz_TetrahedronDesc *tetrahedronDesc, DenOfIz_GeometryData *outGeometryData );
    DZ_API void DenOfIz_Geometry_BuildOctahedron( const DenOfIz_OctahedronDesc *octahedronDesc, DenOfIz_GeometryData *outGeometryData );
    DZ_API void DenOfIz_Geometry_BuildDodecahedron( const DenOfIz_DodecahedronDesc *dodecahedronDesc, DenOfIz_GeometryData *outGeometryData );
    DZ_API void DenOfIz_Geometry_BuildIcosahedron( const DenOfIz_IcosahedronDesc *desc, DenOfIz_GeometryData *outGeometryData );

#ifdef __cplusplus
}
#endif
