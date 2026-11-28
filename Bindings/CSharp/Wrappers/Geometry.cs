using System;
using DenOfIz.Native;

namespace DenOfIz
{
    public static partial class Geometry
    {
        public static GeometryData BuildQuadXY(in QuadDesc quadDesc)
        {
            ulong outGeometryData = default;
            Methods.DenOfIz_Geometry_BuildQuadXY(in quadDesc, out outGeometryData);
            return new GeometryData(outGeometryData, ownsHandle: true);
        }

        public static GeometryData BuildQuadXZ(in QuadDesc quadDesc)
        {
            ulong outGeometryData = default;
            Methods.DenOfIz_Geometry_BuildQuadXZ(in quadDesc, out outGeometryData);
            return new GeometryData(outGeometryData, ownsHandle: true);
        }

        public static GeometryData BuildBox(in BoxDesc desc)
        {
            ulong outGeometryData = default;
            Methods.DenOfIz_Geometry_BuildBox(in desc, out outGeometryData);
            return new GeometryData(outGeometryData, ownsHandle: true);
        }

        public static GeometryData BuildSphere(in SphereDesc desc)
        {
            ulong outGeometryData = default;
            Methods.DenOfIz_Geometry_BuildSphere(in desc, out outGeometryData);
            if (outGeometryData == 0)
            {
                throw new ArgumentException("Geometry.BuildSphere failed: Tessellation must be at least 3 and low enough to keep the mesh under 65535 vertices.", nameof(desc));
            }
            return new GeometryData(outGeometryData, ownsHandle: true);
        }

        public static GeometryData BuildGeoSphere(in GeoSphereDesc desc)
        {
            ulong outGeometryData = default;
            Methods.DenOfIz_Geometry_BuildGeoSphere(in desc, out outGeometryData);
            if (outGeometryData == 0)
            {
                throw new ArgumentException("Geometry.BuildGeoSphere failed: Tessellation must be low enough to keep the mesh under 65535 vertices.", nameof(desc));
            }
            return new GeometryData(outGeometryData, ownsHandle: true);
        }

        public static GeometryData BuildCylinder(in CylinderDesc desc)
        {
            ulong outGeometryData = default;
            Methods.DenOfIz_Geometry_BuildCylinder(in desc, out outGeometryData);
            if (outGeometryData == 0)
            {
                throw new ArgumentException("Geometry.BuildCylinder failed: Tessellation must be at least 3 and low enough to keep the mesh under 65535 vertices.", nameof(desc));
            }
            return new GeometryData(outGeometryData, ownsHandle: true);
        }

        public static GeometryData BuildCone(in ConeDesc desc)
        {
            ulong outGeometryData = default;
            Methods.DenOfIz_Geometry_BuildCone(in desc, out outGeometryData);
            if (outGeometryData == 0)
            {
                throw new ArgumentException("Geometry.BuildCone failed: Tessellation must be at least 3 and low enough to keep the mesh under 65535 vertices.", nameof(desc));
            }
            return new GeometryData(outGeometryData, ownsHandle: true);
        }

        public static GeometryData BuildTorus(in TorusDesc desc)
        {
            ulong outGeometryData = default;
            Methods.DenOfIz_Geometry_BuildTorus(in desc, out outGeometryData);
            if (outGeometryData == 0)
            {
                throw new ArgumentException("Geometry.BuildTorus failed: Tessellation must be at least 3 and low enough to keep the mesh under 65535 vertices.", nameof(desc));
            }
            return new GeometryData(outGeometryData, ownsHandle: true);
        }

        public static GeometryData BuildTetrahedron(in TetrahedronDesc tetrahedronDesc)
        {
            ulong outGeometryData = default;
            Methods.DenOfIz_Geometry_BuildTetrahedron(in tetrahedronDesc, out outGeometryData);
            return new GeometryData(outGeometryData, ownsHandle: true);
        }

        public static GeometryData BuildOctahedron(in OctahedronDesc octahedronDesc)
        {
            ulong outGeometryData = default;
            Methods.DenOfIz_Geometry_BuildOctahedron(in octahedronDesc, out outGeometryData);
            return new GeometryData(outGeometryData, ownsHandle: true);
        }

        public static GeometryData BuildDodecahedron(in DodecahedronDesc dodecahedronDesc)
        {
            ulong outGeometryData = default;
            Methods.DenOfIz_Geometry_BuildDodecahedron(in dodecahedronDesc, out outGeometryData);
            return new GeometryData(outGeometryData, ownsHandle: true);
        }

        public static GeometryData BuildIcosahedron(in IcosahedronDesc desc)
        {
            ulong outGeometryData = default;
            Methods.DenOfIz_Geometry_BuildIcosahedron(in desc, out outGeometryData);
            return new GeometryData(outGeometryData, ownsHandle: true);
        }

    }
}
